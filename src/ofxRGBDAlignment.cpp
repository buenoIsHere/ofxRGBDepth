/*
 *  ofxRGBDAlignment.cpp
 *  DepthExternalRGB
 *
 *  Created by Jim on 10/18/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxRGBDAlignment.h"


ofxRGBDAlignment::ofxRGBDAlignment() {
	xshift = 9;
	yshift = 20;
	applyShader = true;
}

//-----------------------------------------------

ofxRGBDAlignment::~ofxRGBDAlignment() {
	
}

//-----------------------------------------------

void ofxRGBDAlignment::setup(int squaresWide, int squaresTall, int squareSize) {
	hasDepthImage = false;
	hasColorImage = false;
	
	depthCalibration.setPatternSize(squaresWide, squaresTall);
	depthCalibration.setSquareSize(squareSize);
	
	colorCalibration.setPatternSize(squaresWide, squaresTall);
	colorCalibration.setSquareSize(squareSize);

	//bin -> appname -> category -> apps -> of
	rgbdShader.setGeometryInputType(GL_TRIANGLES); 
	rgbdShader.setGeometryOutputType(GL_TRIANGLE_STRIP);
	rgbdShader.setGeometryOutputCount(6);
	rgbdShader.load("../../../../../addons/ofxRGBDepth/assets/rgbd.vert",
					"../../../../../addons/ofxRGBDepth/assets/rgbd.frag",
					"../../../../../addons/ofxRGBDepth/assets/rgbd.geom");
//	rgbdShader.load("../../../../../addons/ofxRGBDepth/assets/rgbd.vert",
//					"../../../../../addons/ofxRGBDepth/assets/rgbd.frag");
	
	rgbdShader.begin();
	rgbdShader.setUniform1i("externalTexture", 0);
	
	mesh.setUsage(GL_STREAM_DRAW);
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	int w = 640;
	int h = 240;
	for(int i = 0; i < w*h; i++) {
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
bool ofxRGBDAlignment::addCalibrationImagePair(ofPixels &ir, ofPixels &camera) {

	if(depthCalibration.add(toCv(ir))){
		if(!colorCalibration.add(toCv(camera))){
			depthCalibration.imagePoints.erase(depthCalibration.imagePoints.end()-1);
		}		
	}
	
//	if(depthCalibration.imagePoints.size() != colorCalibration.imagePoints.size()){
//		ofLogError("ofxRGBDAlignment -- image point sizes differ!");
//		return false;
//	}
	
	if(depthCalibration.imagePoints.size() > 3){
		depthCalibration.calibrate();
		colorCalibration.calibrate();
	}
	
	if(depthCalibration.isReady() && colorCalibration.isReady()){
		depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
		colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
		
		cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
		cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;

		cout << "Depth ERROR::: " << depthCalibration.getReprojectionError() << endl;
		cout << "Color ERROR::: " << colorCalibration.getReprojectionError() << endl;
		return true;
	}
	
	//cout << " NOT READY! " << depthCalibration.isReady() << " " <<  colorCalibration.isReady()  << " ? ?" << depthCalibration.imagePoints.size() << " " << colorCalibration.imagePoints.size() << endl;
	return false;
}

//-----------------------------------------------

bool ofxRGBDAlignment::calibrateFromDirectoryPair(string depthImageDirectory, string colorImageDirectory){
	
	depthCalibration.calibrateFromDirectory(depthImageDirectory);
	colorCalibration.calibrateFromDirectory(colorImageDirectory);
	
	depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
	colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
	
	cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
	cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;
	
	return depthCalibration.isReady() && colorCalibration.isReady();
}

bool ofxRGBDAlignment::ready(){
	return depthCalibration.isReady() && colorCalibration.isReady();
}

void ofxRGBDAlignment::saveCalibration() {
	depthCalibration.save("depthCalib.yml");	
	colorCalibration.save("colorCalib.yml");
}

void ofxRGBDAlignment::loadCalibration(string calibrationDirectory) {
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	colorCalibration.load(calibrationDirectory+"/colorCalib.yml");
	depthCalibration.calibrate();
	colorCalibration.calibrate();
	
	depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
	colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
	
	cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
	cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;

}

void ofxRGBDAlignment::resetCalibration() {
	//TODO:
}

Calibration & ofxRGBDAlignment::getKinectCalibration(){
	return depthCalibration;
}

Calibration & ofxRGBDAlignment::getExternalCalibration(){
	return colorCalibration;
}

//-----------------------------------------------
void ofxRGBDAlignment::setColorTexture(ofBaseHasTexture& colorImage) {
	currentColorImage = &colorImage;
	hasColorImage = true;
}

//-----------------------------------------------
void ofxRGBDAlignment::update(unsigned short* depthPixelsRaw){
	currentDepthImage = depthPixelsRaw;
	hasDepthImage = true;
	update();
}

void ofxRGBDAlignment::update(){
	int w = 640;
	int h = 240;
	
	int start = ofGetElapsedTimeMillis();
	
	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
	int validPointCount = 0;
	ofVec3f center(0,0,0);
	int index = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {

            unsigned short z = currentDepthImage[y*w+x];
            float xReal = (((float) x - principalPoint.x) / imageSize.width) * z * fx + xshift;
            float yReal = (((float) y - principalPoint.y) / imageSize.height) * z * fy + yshift;
			mesh.setVertex(index, ofVec3f(xReal, yReal, z));
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
				  rotationDepthToColor, translationDepthToColor,
				  colorCalibration.getDistortedIntrinsics().getCameraMatrix(),
				  colorCalibration.getDistCoeffs(),
				  imagePoints);
	
	//cout << "project points " << (ofGetElapsedTimeMillis() - start ) << endl;
	
	for(int i = 0; i < imagePoints.size(); i++) {
		ofVec2f textureCoord = ofVec2f(imagePoints[i].x, imagePoints[i].y);
		mesh.setTexCoord(i, textureCoord);
	}
}

ofVboMesh& ofxRGBDAlignment::getMesh(){
	return mesh;
}

void ofxRGBDAlignment::drawCalibration(bool left){
    if(left){
        depthCalibration.draw3d();
    }
    else{
        colorCalibration.draw3d();
    }
}

ofVec3f ofxRGBDAlignment::getMeshCenter(){
	return meshCenter;
}

float ofxRGBDAlignment::getMeshDistance(){
	return meshDistance;
}

void ofxRGBDAlignment::drawMesh() {
	glPushMatrix();
	glScaled(1, -1, 1);

	glEnable(GL_DEPTH_TEST);
	currentColorImage->getTextureReference().bind();
	mesh.drawFaces();
	currentColorImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();


}

void ofxRGBDAlignment::drawPointCloud() {
	
	ofPushStyle();
	
	ofSetColor(255);
	glPointSize(2);
    
	glPushMatrix();
	glScaled(1, -1, 1);
	
	currentColorImage->getTextureReference().bind();
	
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
	glTexCoordPointer(2, GL_FLOAT, sizeof(ofVec2f), &(mesh.getTexCoords()[0].x));
	glVertexPointer(3, GL_FLOAT, sizeof(ofVec3f), &(mesh.getVertices()[0].x));
                   
	glDrawArrays(GL_POINTS, 0, mesh.getVertices().size());
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glDisable(GL_DEPTH_TEST);
	
	currentColorImage->getTextureReference().unbind();
	
	glPopMatrix();
	
	ofPopStyle();
}

