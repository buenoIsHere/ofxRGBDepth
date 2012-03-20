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

typedef struct{
	int vertexIndex;
	bool valid;
} IndexMap;

ofxRGBDRenderer::ofxRGBDRenderer(){
	//come up with better names
	xshift = 0;
	yshift = 0;
	xmult = 0;
	ymult = 0;
	xscale = 1;
	yscale = 1;
	
	edgeCull = 4000;
	simplify = 1;
	rotationCompensation = 0;
	
	xTextureScale = 1;
	yTextureScale = 1;

	hasDepthImage = false;
	hasRGBImage = false;

	farClip = 5000;
	fadeToWhite = 0.0;
	useCustomShader = false;
	
}

ofxRGBDRenderer::~ofxRGBDRenderer(){

}

bool ofxRGBDRenderer::setup(string calibrationDirectory){
	
	cout << "setting up renderer " << endl;
	
	if(!ofDirectory(calibrationDirectory).exists()){
		ofLogError("ofxRGBDRenderer --- Calibration directory doesn't exist: " + calibrationDirectory);
		return false;
	}
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	rgbCalibration.load(calibrationDirectory+"/rgbCalib.yml");
	
	loadMat(rotationDepthToRGB, calibrationDirectory+"/rotationDepthToRGB.yml");
	loadMat(translationDepthToRGB, calibrationDirectory+"/translationDepthToRGB.yml");
	
	colorShader.load("shaders/colorcontrol");
	colorShader.setUniform1i("tex0", 0);
	
	setSimplification(1);
	return true;
}

void ofxRGBDRenderer::setSimplification(int level){
	simplify = level;
	if (simplify <= 0) {
		simplify = 1;
	}
	else if(simplify > 8){
		simplify == 8;
	}
	
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
	
	if(!hasDepthImage) return;
	
	
	bool debug = false;
	
	int w = 640;
	int h = 480;
	
	int start = ofGetElapsedTimeMillis();

	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
	depthCalibration.undistort( toCv(currentDepthImage), toCv(undistortedDepthImage) );
	
	
	vector<IndexMap> indexMap;
	simpleMesh.clearVertices();
	simpleMesh.clearIndices();
	simpleMesh.clearTexCoords();
	simpleMesh.clearNormals();
		
	int imageIndex = 0;
	int vertexIndex = 0;
	for(int y = 0; y < h; y+= simplify) {
		for(int x = 0; x < w; x+= simplify) {
			unsigned short z = undistortedDepthImage.getPixels()[y*w+x];
			IndexMap indx;
			if(z != 0 && z < farClip){
				float xReal,yReal;
				if(mirror){
					xReal = (((float) principalPoint.x - x + xmult ) / imageSize.width) * z * fx/* + xshift*/;
				}
				else{
					xReal = (((float) x - principalPoint.x + xmult ) / imageSize.width) * z * fx/* + xshift*/;
				}
				yReal = (((float) y - principalPoint.y + ymult ) / imageSize.height) * z * fy/* + yshift*/;
				indx.vertexIndex = simpleMesh.getVertices().size();
				indx.valid = true;
				ofVec3f pt = ofVec3f(xReal, yReal, z);
				simpleMesh.addVertex(pt);
			}
			else {
				indx.valid = false;
			}
			indexMap.push_back( indx );
		}
	}
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
				
				if(calculatedNormals.find(indexMap[baseIndeces[i]].vertexIndex) == calculatedNormals.end()){
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
		projectPoints(pcMat,
					  rotationDepthToRGB, translationDepthToRGB,
					  rgbCalibration.getDistortedIntrinsics().getCameraMatrix(),
					  rgbCalibration.getDistCoeffs(),
					  imagePoints);
	
		if(debug) cout << "project points " << (ofGetElapsedTimeMillis() - start) << endl;
		
		
		start = ofGetElapsedTimeMillis();

		for(int i = 0; i < imagePoints.size(); i++) {
			if(mirror){
				simpleMesh.addTexCoord(ofVec2f( currentRGBImage->getTextureReference().getWidth() - imagePoints[i].x * xTextureScale, imagePoints[i].y * yTextureScale));
			}
			else{
				simpleMesh.addTexCoord(ofVec2f( imagePoints[i].x * xTextureScale, imagePoints[i].y * yTextureScale));			
			}
		}
		if(debug) cout << "gen tex coords took " << (ofGetElapsedTimeMillis() - start) << endl;
	}
}

ofMesh& ofxRGBDRenderer::getMesh(){
	return simpleMesh;
}

void ofxRGBDRenderer::drawMesh() {
	
	if(!hasDepthImage) return;
	
	glPushMatrix();
	glScaled(1, -1, 1);
	glRotatef(rotateMeshX, 1, 0, 0);
	
	glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		if(useCustomShader){
			colorShader.begin();
			colorShader.setUniform1f("white", fadeToWhite);		
		}
		currentRGBImage->getTextureReference().bind();
	}
	simpleMesh.drawFaces();
	if(hasRGBImage){
		currentRGBImage->getTextureReference().unbind();
		if(useCustomShader){
			colorShader.end();
		}
	}
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
}

void ofxRGBDRenderer::drawPointCloud() {
	
	if(!hasDepthImage) return;
	
	glPushMatrix();
	glScaled(1, -1, 1);
	glRotatef(rotateMeshX, 1, 0, 0);
	glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		colorShader.begin();
		colorShader.setUniform1f("white", fadeToWhite);				
		currentRGBImage->getTextureReference().bind();
	}
	simpleMesh.drawVertices();
	if(hasRGBImage){
		currentRGBImage->getTextureReference().unbind();
		colorShader.end();
	}
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
}

void ofxRGBDRenderer::drawWireFrame() {
	
	if(!hasDepthImage) return;
	
	glPushMatrix();
	glScaled(1, -1, 1);
	glRotatef(rotateMeshX, 1, 0, 0);
	
	glEnable(GL_DEPTH_TEST);
	if(hasRGBImage){
		colorShader.begin();
		colorShader.setUniform1f("white", fadeToWhite);				
		currentRGBImage->getTextureReference().bind();
	}
	simpleMesh.drawWireframe();
	if(hasRGBImage){
		currentRGBImage->getTextureReference().unbind();
		colorShader.end();
	}
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();	
}

ofTexture& ofxRGBDRenderer::getTextureReference(){
	return currentRGBImage->getTextureReference();
}
