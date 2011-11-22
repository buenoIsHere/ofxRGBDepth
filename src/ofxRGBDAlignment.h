/*
 *  ofxRGBDAlignment.h
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

class ofxRGBDAlignment {
  public:
	ofxRGBDAlignment();
	~ofxRGBDAlignment();
	
	void setup(int squaresWide, int squaresTall, int squareSize);
	
	bool addCalibrationImagePair(ofPixels& ir, ofPixels &camera);
	bool calibrateFromDirectoryPair(string irImageDirectory, string colorImageDirectory);
	bool ready();

	//void setColorImage(ofImage& colorImage);
	void setColorTexture(ofBaseHasTexture& colorImage); 
	
    void update(unsigned short* depthPixelsRaw);
	void update();
	
	void drawMesh();
	void drawPointCloud();
	
	void saveCalibration();
	void loadCalibration(string calibrationDirectory);
	void resetCalibration();

	Calibration & getKinectCalibration();
	Calibration & getExternalCalibration();
	//populated with vertices, texture coords, and indeces
	ofMesh & getMesh();
	
	void drawCalibration(bool left);
    
	ofVec3f getMeshCenter();
	float getMeshDistance();
	
    float yshift;
	float xshift;
	
  protected:
	bool hasDepthImage;
	bool hasColorImage;
	bool hasPointCloud;
		
	ofBaseHasTexture* currentColorImage;
	unsigned short* currentDepthImage;
	
	ofShader renderShader;
	
	vector<Point2f> imagePoints;    
	ofVboMesh mesh;
    vector<ofIndexType> indeces;
    vector<ofVec2f> texcoords;
    vector<ofVec3f> vertices;

	Calibration depthCalibration, colorCalibration;    
	Mat rotationDepthToColor, translationDepthToColor;
	Mat rotationColorToDepth, translationColorToDepth;
	Mat rotation, translation;
	
	ofVec3f meshCenter;
	float meshDistance;
	
};