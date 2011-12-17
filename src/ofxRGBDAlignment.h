/*
 *  ofxRGBDAlignment.h
 *  DepthExternalRGB
 *
 * The alignment tool helps organize a series of checkerboard alignment images
 * to produce a set of calibration files. 
 *
 * A big part of the process of creating an external <-> depth alignment is having good alignment images
 * One bad checkerboard can throw the entire aligment off, so this tool helps weed out bad alignment pairs
 * by checking against reprojection errors
 *
 * This class encapsulates a gui system that assists previewing the alignment images, adding
 * and removing them from the set.
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

using namespace ofxCv;
using namespace cv;

typedef struct {
	string filepath;
	ofImage image;
	bool hasCheckerboard;
	float subpixelRefinement;
	float reprojectionError;
	ofRectangle drawRect;
} CalibrationImage;

typedef struct {
	CalibrationImage* rgbImage;
	CalibrationImage* depthImage;
} CalibrationImagePair;

class ofxRGBDAlignment {
  public:
	ofxRGBDAlignment();
	~ofxRGBDAlignment();
	
	void setup(int squaresWide, int squaresTall, int squareSize);
	
	void addRGBCalibrationImage(string rgbCalibrationImagePath);
	void addDepthCalibrationImage(string depthCalibrationImagePath);
	void addCalibrationImagePair(string depthCalibrationImagePath, string rgbCalibrationPath);
	
	void addRGBCalibrationDirectory(string rgbImageDirectory);
	void addDepthCalibrationDirectory(string depthImageDirectory);
	void addCalibrationDirectoryPair(string depthImageDirectory, string rgbImageDirectory);
	
	void clearRGBImages();
	void clearDepthImages();
	
	bool ready();
	bool generateAlignment();

	//save and load the current image sets 
	void saveState();
	void saveState(string filePath);
	void loadState(string filePath);

	void draw3DCalibrationDebug(bool left);
	
	Calibration& getDepthCalibration();
	Calibration& getRGBCalibration();
	
	//create calibration files from the current images for use in ofxRGBDRenderer
	void saveAlignment(string saveDirectory);
	
	//GUI STUFF
	//perhaps this is bad deisgn to mix these two together, but it works well in this case
	void setupGui(float x, float y, float maxDrawWidth); //enables events
	void setMaxDrawWidth(float maxDrawWidth);
	
	void drawGui();
	
	void drawImagePairs();
	void drawDepthImages();
	void drawRGBImages();

	//discards the current selected set of images
	void discardCurrentPair();
	
	ofImage& getCurrentDepthImage();
	ofImage& getCurrentRGBImage();
	
	void keyPressed(ofKeyEventArgs& args);
	void keyReleased(ofKeyEventArgs& args);
	void mouseMoved(ofMouseEventArgs& args);
	void mouseDragged(ofMouseEventArgs& args);
	void mousePressed(ofMouseEventArgs& args);
	void mouseReleased(ofMouseEventArgs& args);

  protected:
	
	string stateFilePath;
	vector<CalibrationImage> rgbImages;
	vector<CalibrationImage> depthImages;
	vector<CalibrationImagePair> images;
	
	Calibration depthCalibration, rgbCalibration;    
	Mat rotationDepthToRGB, translationDepthToRGB;
	Mat rotationRGBToDepth, translationRGBToDepth;
	Mat rotation, translation;
	
	ofRectangle depthImageBoundingRect;
	ofRectangle rgbImageBoundingRect;
	
	bool guiIsSetup;
	int selectedDepthImage;
	int selectedRgbImage;
	ofImage currentRGBImage;
	ofImage currentDepthImage;
	float infoBoxHeight;
	void recalculateImageDrawRects();
	
	ofVec2f guiPosition;
	float maxGuiDrawWidth;

	
	
	
};