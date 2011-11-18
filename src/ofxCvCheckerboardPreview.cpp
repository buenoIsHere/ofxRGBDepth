	/*
 *  ofxCvCheckerboardPreview.cpp
 *  DepthExternalRGB
 *
 *  Created by Jim on 10/18/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxCvCheckerboardPreview.h"


void ofxCvCheckerboardPreview::setup(int squaresWide, int squaresTall, int squareSize){
	boardFound = false;
	newBoardToFind = false;
	
	squarePixelSize = 11;
	calib.setPatternSize(squaresWide, squaresTall);
	calib.setSquareSize(squareSize);
	calib.setSubpixelSize(squareSize);
	boardLastFoundTime = 0;
	
	
	startThread(true, false);

}

void ofxCvCheckerboardPreview::setTestImage(ofPixels& pixels) {
	
	if(pixels.getImageType() != OF_IMAGE_GRAYSCALE){
		ofLogError("ofxCvCheckerboardPreview -- must use grayscale image");
		return;
	}
	
	lock();
	testImage = pixels;
	if(!internalPixels.isAllocated()){
		internalPixels.allocate(pixels.getWidth(), pixels.getHeight(), OF_PIXELS_MONO);
	}
	newBoardToFind = true;
	boardFound = false;
	unlock();
}

bool ofxCvCheckerboardPreview::currentImageHasCheckerboard() {
	return boardFound;
}

void ofxCvCheckerboardPreview::draw(){
	draw(ofRectangle(0,0,testImage.getWidth(), testImage.getHeight()) );
}

void ofxCvCheckerboardPreview::draw(ofVec2f point){
	draw(ofRectangle(point.x,point.y,testImage.getWidth(), testImage.getHeight() ));
}
		 
void ofxCvCheckerboardPreview::draw(const ofRectangle& rect){
	vector<Point2f> points;
	drawLock.lock();
	points = imagePoints;
	drawLock.unlock();
	
	//float alpha = ofMap(ofGetElapsedTimef(), boardLastFoundTime+.5, boardLastFoundTime+2, 255, 0, true);
	float alpha = 255;
	
	ofPushStyle();
	ofSetColor(255, 0,0, alpha);
	ofFill();
	
	float x, y;
	for (int i = 0; i < imagePoints.size(); ++i) {
		x = ofMap(imagePoints[i].x, 0, testImage.getWidth(), rect.x, rect.x + rect.width, false);
		y = ofMap(imagePoints[i].y, 0, testImage.getHeight(), rect.y, rect.y + rect.height, false);
		
		ofCircle(x, y, 3);
	}
	
	ofPopStyle();
}


void ofxCvCheckerboardPreview::setSmallestSquarePixelsize(int squareSize){
	if(squareSize != squarePixelSize){
		squarePixelSize = squareSize;
		calib.setSubpixelSize(squareSize);
		if(testImage.isAllocated()){
			newBoardToFind = true;
		}
	}
}

int ofxCvCheckerboardPreview::getSmallestSquarePixelsize(){
	return squarePixelSize;
}

Calibration& ofxCvCheckerboardPreview::getCalibration(){
	return calib;
}

void ofxCvCheckerboardPreview::threadedFunction() {
	
	while (isThreadRunning()){
		if(newBoardToFind){
			
			lock();
			memcpy(internalPixels.getPixels(), testImage.getPixels(), testImage.getWidth()*testImage.getHeight()); 
			unlock();
			
			vector<Point2f> points;
			boardFound = calib.findBoard(toCv(internalPixels), points);
			if (boardFound){
				boardLastFoundTime = ofGetElapsedTimef();
				drawLock.lock();
				imagePoints = points;
				drawLock.unlock();
			}
			else{
				drawLock.lock();
				imagePoints.clear();
				drawLock.unlock();
				
			}
			newBoardToFind = false;
		}
	}
}

void ofxCvCheckerboardPreview::quit() {
	stopThread(true);
}