/*
 *  ofxRGBDRenderer.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/17/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxRGBDRenderer.h"

ofxRGBDRenderer::ofxRGBDRenderer(){
	xshift = 0;
	yshift = 0;
	xscale = 0;
	yscale = 0;
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
	
	mesh.setUsage(GL_STREAM_DRAW);
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	int w = 640;
	int h = 480;
	for(int i = 0; i < w*h; i++) {
		mesh.addColor(ofFloatColor(0,0,0));
 		mesh.addVertex(ofVec3f(0,0,0));
		mesh.addTexCoord(ofVec2f(0,0));
	}
	
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = (x+1)+y*w;
			c = x+(y+1)*w;
			mesh.addTriangle(a, b, c);
			
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			mesh.addTriangle(a, b, c);
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

void ofxRGBDRenderer::update(){
	int w = 640;
	int h = 480;
	
	int start = ofGetElapsedTimeMillis();
	
	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	
//	cout << "fov? " << fov.x << " " << fov.y << endl;
	
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
//	cout << "principal point " << principalPoint.x << " " << principalPoint.y << endl;
//	cout << "image size " << imageSize.width << " " << imageSize.height << endl;
	
	int validPointCount = 0;
	ofVec3f center(0,0,0);
	int index = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			
            unsigned short z = currentDepthImage[y*w+x];
            float xReal = (((float) x - principalPoint.x) / imageSize.width) * z * fx + xshift;
            float yReal = (((float) y - principalPoint.y) / imageSize.height) * z * fy + yshift;
			mesh.setVertex(index, ofVec3f(xReal, yReal, z));
			mesh.setColor(index, z == 0 ? ofFloatColor(0,0,0,0) : ofFloatColor(1,1,1,1));
			index++;
		}
	}
	
	//cout << "unproject points " << (ofGetElapsedTimeMillis() - start) << endl;
	start = ofGetElapsedTimeMillis();
	
	Mat pcMat = Mat(toCv(mesh));
	
	//cout << "create mesh " << (ofGetElapsedTimeMillis() - start) << endl;
	start = ofGetElapsedTimeMillis();
	
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
	
	//cout << "project points " << (ofGetElapsedTimeMillis() - start ) << endl;
	
	for(int i = 0; i < imagePoints.size(); i++) {
		ofVec2f textureCoord = ofVec2f(imagePoints[i].x, imagePoints[i].y);
		mesh.setTexCoord(i, textureCoord);
	}
}

ofVboMesh& ofxRGBDRenderer::getMesh(){
	return mesh;
}

ofVec3f ofxRGBDRenderer::getMeshCenter(){
	return meshCenter;
}

float ofxRGBDRenderer::getMeshDistance(){
	return meshDistance;
}

void ofxRGBDRenderer::drawMesh() {
	glPushMatrix();
	glScaled(1, -1, 1);
	
	glEnable(GL_DEPTH_TEST);
	currentRGBImage->getTextureReference().bind();
	mesh.drawFaces();
	currentRGBImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();
}

void ofxRGBDRenderer::drawPointCloud() {
	
	ofPushStyle();
	
	ofSetColor(255);
	glPointSize(2);
    
	glPushMatrix();
	glScaled(1, -1, 1);
	
	//currentRGBImage->getTextureReference().bind();
	
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
	
	glTexCoordPointer(2, GL_FLOAT, sizeof(ofVec2f), &(mesh.getTexCoords()[0].x));
	glVertexPointer(3, GL_FLOAT, sizeof(ofVec3f), &(mesh.getVertices()[0].x));
	glColorPointer(4, GL_FLOAT, sizeof(ofFloatColor), &(mesh.getColors()[0].r));
				   
	glDrawArrays(GL_POINTS, 0, mesh.getVertices().size());
	
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glDisable(GL_DEPTH_TEST);
	
	//currentRGBImage->getTextureReference().unbind();
	
	glPopMatrix();
	
	ofPopStyle();
}
