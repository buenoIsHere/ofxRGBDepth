/*
 *  ofxRGBDRenderer.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/17/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxRGBDRenderer.h"

typedef struct{
	int vertexIndex;
	bool valid;
} IndexMap;

ofxRGBDRenderer::ofxRGBDRenderer(){
	xshift = 0;
	yshift = 0;
	xscale = 1;
	yscale = 1;
	
	xTextureScale = 1;
	yTextureScale = 1;

	hasDepthImage = false;
	hasRGBImage = false;
}

ofxRGBDRenderer::~ofxRGBDRenderer(){

}

bool ofxRGBDRenderer::setup(string calibrationDirectory){
	if(!ofDirectory(calibrationDirectory).exists()){
		return false;
	}
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	rgbCalibration.load(calibrationDirectory+"/rgbCalib.yml");
	
	loadMat(rotationDepthToRGB, calibrationDirectory+"/rotationDepthToRGB.yml");
	loadMat(translationDepthToRGB, calibrationDirectory+"/translationDepthToRGB.yml");
	
	applyShader = false;
	
	/*
	cout << "rotation is " << rotationDepthToRGB << endl;
	cout << "translation is " << translationDepthToRGB << endl;
	
	cout << "rgb cam matrix " << rgbCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
	cout << "rgb cam dist coef " << rgbCalibration.getDistCoeffs() << endl;

	cout << "depth cam matrix " << depthCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
	cout << "depth cam dist coef " << depthCalibration.getDistCoeffs() << endl;
	 */
	
	//bin -> appname -> category -> apps -> of
	rgbdShader.setGeometryInputType(GL_TRIANGLES); 
	rgbdShader.setGeometryOutputType(GL_TRIANGLE_STRIP);
	rgbdShader.setGeometryOutputCount(6);
	rgbdShader.load("../../../../../addons/ofxRGBDepth/assets/rgbd.vert",
					"../../../../../addons/ofxRGBDepth/assets/rgbd.frag",
					"../../../../../addons/ofxRGBDepth/assets/rgbd.geom");
	rgbdShader.begin();
	rgbdShader.setUniform1i("externalTexture", 0);
	rgbdShader.end();
	
//	mesh.setUsage(GL_STREAM_DRAW);
//	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	int w = 640;
	int h = 480;
	
//	for(int i = 0; i < w*h; i++) {
//		mesh.addColor(ofFloatColor(0,0,0));
// 		mesh.addVertex(ofVec3f(0,0,0));
//		mesh.addTexCoord(ofVec2f(0,0));
//	}
	
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = (x+1)+y*w;
			c = x+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);
//			mesh.addTriangle(a, b, c);
			
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			baseIndeces.push_back(a);
			baseIndeces.push_back(b);
			baseIndeces.push_back(c);			
//			mesh.addTriangle(a, b, c);
		}
	}		
}

//-----------------------------------------------
void ofxRGBDRenderer::setRGBTexture(ofBaseHasTexture& rgbImage) {
	currentRGBImage = &rgbImage;
	hasRGBImage = true;
}

void ofxRGBDRenderer::setDepthImage(unsigned short* depthPixelsRaw){
	currentDepthImage = depthPixelsRaw;
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
	
	if(!hasDepthImage || !hasRGBImage) return;
	
	bool debug = false;
	
	int w = 640;
	int h = 480;
	
	int start = ofGetElapsedTimeMillis();

	Point2d fov = depthCalibration.getDistortedIntrinsics().getFov();
	
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getDistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getDistortedIntrinsics().getImageSize();
		
	vector<IndexMap> indexMap;
	simpleMesh.clearVertices();
	
	int imageIndex = 0;
	int vertexIndex = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			
            unsigned short z = currentDepthImage[y*w+x];
			IndexMap indx;
			if(z != 0){
				float xReal = (((float) x - principalPoint.x + xshift ) / imageSize.width) * z * fx;
				float yReal = (((float) y - principalPoint.y + yshift ) / imageSize.height) * z * fy;
				
				indx.vertexIndex = simpleMesh.getVertices().size();
				indx.valid = true;
				simpleMesh.addVertex(ofVec3f(xReal, yReal, z));
			}
			else {
				indx.valid = false;
			}
			indexMap.push_back( indx );
		}
	}
	if(debug) cout << "un projection took " << ofGetElapsedTimeMillis() - start << endl;

	
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
	simpleMesh.clearIndices();
	for(int i = 0; i < baseIndeces.size(); i+=3){
		if(indexMap[baseIndeces[i]].valid &&
		   indexMap[baseIndeces[i+1]].valid &&
		   indexMap[baseIndeces[i+2]].valid){
			simpleMesh.addTriangle(indexMap[baseIndeces[i]].vertexIndex, 
								   indexMap[baseIndeces[i+1]].vertexIndex,
								   indexMap[baseIndeces[i+2]].vertexIndex);
		}
	}
	
	if(debug) cout << "reindexing took " << ofGetElapsedTimeMillis() - start << endl;
	
	start = ofGetElapsedTimeMillis();
	simpleMesh.clearTexCoords();
	for(int i = 0; i < imagePoints.size(); i++) {
		ofVec2f textureCoord = ofVec2f(imagePoints[i].x * xTextureScale, imagePoints[i].y * yTextureScale);
		simpleMesh.addTexCoord(textureCoord);
	}
	
	if(debug) cout << "gen tex coords took " << (ofGetElapsedTimeMillis() - start) << endl;
}

ofMesh& ofxRGBDRenderer::getMesh(){
	//return mesh;
	return simpleMesh;
}


void ofxRGBDRenderer::drawMesh() {
	glPushMatrix();
	glScaled(1, -1, 1);
	
	glEnable(GL_DEPTH_TEST);
	currentRGBImage->getTextureReference().bind();
	simpleMesh.drawFaces();
	currentRGBImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
}

void ofxRGBDRenderer::drawPointCloud() {
	
	glPushMatrix();
	glScaled(1, -1, 1);
	
	glEnable(GL_DEPTH_TEST);
	currentRGBImage->getTextureReference().bind();
	simpleMesh.drawVertices();
	currentRGBImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
	
}

void ofxRGBDRenderer::drawWireFrame() {
	glPushMatrix();
	glScaled(1, -1, 1);
	
	glEnable(GL_DEPTH_TEST);
	currentRGBImage->getTextureReference().bind();
	simpleMesh.drawWireframe();
	currentRGBImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
	
}