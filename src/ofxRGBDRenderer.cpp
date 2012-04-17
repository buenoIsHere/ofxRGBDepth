/*
 *  ofxRGBDRenderer.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/17/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxRGBDRenderer.h"
#include <set>

ofxRGBDRenderer::ofxRGBDRenderer(){
	//come up with better names
	xmult = 0;
	ymult = 0;
	
	edgeCull = 4000;
	simplify = 1;

	farClip = 6000;
	ZFuzz = 0;
    meshRotate = ofVec3f(0,0,0);
    
    xTextureScale = 1;
	yTextureScale = 1;
    
    calculateNormals = false;
    
	hasDepthImage = false;
	hasRGBImage = false;
	mirror = false;
	calibrationSetup = false;
    setSimplification(1);
	reloadShader();
}

ofxRGBDRenderer::~ofxRGBDRenderer(){

}

bool ofxRGBDRenderer::setup(string calibrationDirectory){
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDRenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	rgbCalibration.load(calibrationDirectory+"/rgbCalib.yml");
	
	loadMat(rotationDepthToRGB, calibrationDirectory+"/rotationDepthToRGB.yml");
	loadMat(translationDepthToRGB, calibrationDirectory+"/translationDepthToRGB.yml");
	
	calibrationSetup = true;

	return true;
}

void ofxRGBDRenderer::setSimplification(int level){
	simplify = level;
	if (simplify <= 0) {
		simplify = 1;
	}
	else if(simplify > 8){
		simplify = 8;
	}

	indexMap.clear();
	baseIndeces.clear();
	int w = 640 / simplify;
	int h = 480 / simplify;
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = (x+1)+y*w;
			c = x+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
			
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);

			IndexMap m;
			indexMap.push_back(m);
			simpleMesh.addVertex(ofVec3f(0,0,0));
		}
	}		
}

//-----------------------------------------------
int ofxRGBDRenderer::getSimplification(){
	return simplify;
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasTexture& rgbImage) {
	currentRGBImage = &rgbImage;
	hasRGBImage = true;
}

ofBaseHasTexture& ofxRGBDRenderer::getRGBTexture() {
    return *currentRGBImage;
}

void ofxRGBDRenderer::setDepthImage(ofShortPixels& pix){
	currentDepthImage.setFromPixels(pix);
	if(!undistortedDepthImage.isAllocated()){
		undistortedDepthImage.allocate(640,480,OF_IMAGE_GRAYSCALE);
	}
	hasDepthImage = true;
}

void ofxRGBDRenderer::setDepthImage(unsigned short* depthPixelsRaw){
	currentDepthImage.setFromPixels(depthPixelsRaw, 640,480, OF_IMAGE_GRAYSCALE);
	if(!undistortedDepthImage.isAllocated()){
		undistortedDepthImage.allocate(640,480,OF_IMAGE_GRAYSCALE);
	}
	hasDepthImage = true;
}

void ofxRGBDRenderer::setTextureScale(float xs, float ys){
	xTextureScale = xs;
	yTextureScale = ys;
}

Calibration& ofxRGBDRenderer::getDepthCalibration(){
	return depthCalibration;
}

Calibration& ofxRGBDRenderer::getRGBCalibration(){
	return rgbCalibration;
}

void ofxRGBDRenderer::update(){
	
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }

    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return;
    }

	bool debug = false;
	
	int w = 640;
	int h = 480;
	
	int start = ofGetElapsedTimeMillis();

	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
	depthCalibration.undistort( toCv(currentDepthImage), toCv(undistortedDepthImage), CV_INTER_NN);
	
	//simpleMesh.clearVertices();
	simpleMesh.clearIndices();
	simpleMesh.clearTexCoords();
	simpleMesh.clearNormals();
	
	/*
	int imageIndex = 0;
	int vertexIndex = 0;
	for(int y = 0; y < h; y+= simplify) {
		for(int x = 0; x < w; x+= simplify) {
			unsigned short z = undistortedDepthImage.getPixels()[y*w+x];
			IndexMap indx;
			if(z != 0 && z < farClip){
				float xReal,yReal;
				if(mirror){
					xReal = (((float) principalPoint.x - x - xmult ) / imageSize.width) * z * fx;
				}
				else{
					xReal = (((float) x - principalPoint.x + xmult ) / imageSize.width) * z * fx;
				}
				yReal = (((float) y - principalPoint.y + ymult ) / imageSize.height) * z * fy;
				indx.vertexIndex = simpleMesh.getVertices().size();
				indx.valid = true;
				if(ZFuzz != 0){
					z += ofRandomf()*ZFuzz;
				}
				ofVec3f pt = ofVec3f(xReal, yReal, z);
				simpleMesh.addVertex(pt);
			}
			else {
				indx.valid = false;
			}
			indexMap.push_back( indx );
		}
	}
	*/

    //start
	int imageIndex = 0;
	int vertexIndex = 0;
	float xReal,yReal;
	unsigned short* ptr = undistortedDepthImage.getPixels();
	for(int y = 0; y < h; y += simplify) {
		for(int x = 0; x < w; x += simplify) {
			//unsigned short z = undistortedDepthImage.getPixels()[y*w+x];
			unsigned short z = *ptr++;
			IndexMap& indx = indexMap[y*w+x];
			if(z != 0 && z < farClip){
				xReal = (((float)x - principalPoint.x + xmult ) / imageSize.width) * z * fx;
				yReal = (((float)y - principalPoint.y + ymult ) / imageSize.height) * z * fy;
				//indx.vertexIndex = simpleMesh.getVertices().size();
				indx.vertexIndex = y*w+x;
				indx.valid = true;
//				ofVec3f pt = ofVec3f(xReal, yReal, z);
				//simpleMesh.addVertex(pt);
				simpleMesh.getVertices()[y*w+x] = ofVec3f(xReal, yReal, z);
			}
			else {
				indx.valid = false;
			}
		}
	}
    //end

	if(debug) cout << "unprojection " << simpleMesh.getVertices().size() << " took " << ofGetElapsedTimeMillis() - start << endl;
	
    if(simpleMesh.getVertices().size() < 3){
		ofLogError("ofxRGBDRenderer -- No verts");
		return;
	}
	
	set<ofIndexType> calculatedNormals;
	start = ofGetElapsedTimeMillis();
	simpleMesh.getNormals().resize(simpleMesh.getVertices().size());
	
	for(int i = 0; i < baseIndeces.size(); i+=3){
		if(indexMap[baseIndeces[i]].valid &&
		   indexMap[baseIndeces[i+1]].valid &&
		   indexMap[baseIndeces[i+2]].valid){
			
			ofVec3f a,b,c;
			a = simpleMesh.getVertices()[indexMap[baseIndeces[i]].vertexIndex]; 
			b = simpleMesh.getVertices()[indexMap[baseIndeces[i+1]].vertexIndex]; 
			c = simpleMesh.getVertices()[indexMap[baseIndeces[i+2]].vertexIndex]; 
			if(fabs(a.z - b.z) < edgeCull && fabs(a.z - c.z) < edgeCull){
				simpleMesh.addTriangle(indexMap[baseIndeces[i]].vertexIndex, 
									   indexMap[baseIndeces[i+1]].vertexIndex,
									   indexMap[baseIndeces[i+2]].vertexIndex);
				
				if(calculateNormals && calculatedNormals.find(indexMap[baseIndeces[i]].vertexIndex) == calculatedNormals.end()){
					//calculate normal
					simpleMesh.setNormal(indexMap[baseIndeces[i]].vertexIndex, (b-a).getCrossed(b-c).getNormalized());
					calculatedNormals.insert(indexMap[baseIndeces[i]].vertexIndex);
				}
			}
		}
	}
	
	if(debug) cout << "indexing  " << simpleMesh.getIndices().size() << " took " << ofGetElapsedTimeMillis() - start << endl;

	if(hasRGBImage){
		start = ofGetElapsedTimeMillis();
		
		//Mat pcMat = Mat(toCv(mesh));
		Mat pcMat = Mat(toCv(simpleMesh));
		
		//cout << "PC " << pcMat << endl;
		//	cout << "Rot Depth->Color " << rotationDepthToColor << endl;
		//	cout << "Trans Depth->Color " << translationDepthToColor << endl;
		//	cout << "Intrs Cam " << colorCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
		//	cout << "Intrs Dist Coef " << colorCalibration.getDistCoeffs() << endl;
		
		imagePoints.clear();

		depthToRGBView = ofxCv::makeMatrix(rotationDepthToRGB, translationDepthToRGB);

		//--
		//load projection matrix
		ofPushView();
		rgbCalibration.getDistortedIntrinsics().loadProjectionMatrix();
		glGetFloatv(GL_PROJECTION_MATRIX, rgbProjection.getPtr());
		ofPopView();
		//--

		/*
		projectPoints(pcMat,
					  rotationDepthToRGB, translationDepthToRGB,
					  rgbCalibration.getDistortedIntrinsics().getCameraMatrix(),
					  rgbCalibration.getDistCoeffs(),
					  imagePoints);
		*/
		if(debug) cout << "project points " << (ofGetElapsedTimeMillis() - start) << endl;
		
		start = ofGetElapsedTimeMillis();
		/*
		for(int i = 0; i < imagePoints.size(); i++) {
			if(mirror){
				simpleMesh.addTexCoord(ofVec2f( currentRGBImage->getTextureReference().getWidth() - imagePoints[i].x * xTextureScale, imagePoints[i].y * yTextureScale));
			}
			else{
				simpleMesh.addTexCoord(ofVec2f(imagePoints[i].x * xTextureScale, 
                                               imagePoints[i].y * yTextureScale));			
			}
		}
		*/
		if(debug) cout << "gen tex coords took " << (ofGetElapsedTimeMillis() - start) << endl;
	}
}

ofMesh& ofxRGBDRenderer::getMesh(){
	return simpleMesh;
}

void ofxRGBDRenderer::drawMesh() {
	
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return;
    }
	
	//glPushMatrix();
    ofPushMatrix();
    ofScale(1, -1, 1);
    ofRotate(meshRotate.x,1,0,0);
    ofRotate(meshRotate.y,0,1,0);
    ofRotate(meshRotate.z,0,0,1);
	
	glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		currentRGBImage->getTextureReference().bind();
	}
	simpleMesh.drawFaces();
	if(hasRGBImage){
		currentRGBImage->getTextureReference().unbind();
	}
    
	glDisable(GL_DEPTH_TEST);
	
	ofPopMatrix();
}

void ofxRGBDRenderer::drawPointCloud() {
	
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return;
    }
	
    ofPushMatrix();
    ofScale(1, -1, 1);
    ofRotate(meshRotate.x,1,0,0);
    ofRotate(meshRotate.y,0,1,0);
    ofRotate(meshRotate.z,0,0,1);
    
    glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		currentRGBImage->getTextureReference().bind();
	}
	
    simpleMesh.drawVertices();
    
	if(hasRGBImage){
		currentRGBImage->getTextureReference().unbind();
	}
    
	glDisable(GL_DEPTH_TEST);
	
	ofPopMatrix();
}

void ofxRGBDRenderer::drawWireFrame() {
	
	if(!hasDepthImage){
     	ofLogError("ofxRGBDRenderer::update() -- no depth image");
        return;
    }
    
    if(!calibrationSetup){
     	ofLogError("ofxRGBDRenderer::update() -- no calibration");
        return;
    }
	
    ofPushMatrix();
    ofScale(1, -1, 1);
    ofRotate(meshRotate.x,1,0,0);
    ofRotate(meshRotate.y,0,1,0);
    ofRotate(meshRotate.z,0,0,1);
	
	ofMatrix4x4 rgbMatrix = (depthToRGBView * rgbProjection).getInverse();
	//cout << rgbMatrix << endl;
	glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		currentRGBImage->getTextureReference().bind();
		shader.begin();	
		GLint loc = glGetUniformLocation(shader.getProgram(), "tTex");
		glUniformMatrix4fv( loc, 1, GL_FALSE, rgbMatrix.getPtr() );
		//cout << "loc " << loc << " prog " << shader.getProgram() << endl;
		//shader.setUniform1f("dummy", 4.0);
		ofVec2f dims = ofVec2f(currentRGBImage->getTextureReference().getWidth(), 
   							currentRGBImage->getTextureReference().getHeight());
		shader.setUniform2f("fudge", xmult/dims.x, ymult/dims.y);
		shader.setUniform2f("dim", dims.x, dims.y);
		glMatrixMode(GL_TEXTURE);
		glLoadMatrixf(rgbMatrix.getPtr());
		glMatrixMode(GL_MODELVIEW);
	}
	simpleMesh.drawWireframe();
	if(hasRGBImage){
		shader.end();
		currentRGBImage->getTextureReference().unbind();
	}
    
	glDisable(GL_DEPTH_TEST);
	
	ofPopMatrix();
}

ofTexture& ofxRGBDRenderer::getTextureReference(){
	return currentRGBImage->getTextureReference();
}
