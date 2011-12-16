/*
 *  ofxCvCheckerboardPreview.h
 *  DepthExternalRGB
 *
 *  Created by Jim on 10/18/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

using namespace ofxCv;
using namespace cv;

class ofxCvCheckerboardPreview : public ofThread {
  public:
	void setup(int squaresWide, int squaresTall, int squareSize); //starts the thread
	
	void setTestImage(ofPixels& pixels);
	void setSmallestSquarePixelsize(int squarePixelSize);
	bool currentImageHasCheckerboard();	
	void draw();
	void draw(float x, float y);
	void draw(ofVec2f point);
	void draw(const ofRectangle& rect);


	int getSmallestSquarePixelsize();
	Calibration& getCalibration();
	
	void quit(); //stops thread
	
  protected:
	
	void threadedFunction();
	float boardLastFoundTime;
	int squarePixelSize;
	ofPixels testImage;
	ofPixels internalPixels;
	vector<Point2f> imagePoints;
	
	bool boardFound;
	bool newBoardToFind;
	Calibration	calib;
	ofMutex drawLock;	
};