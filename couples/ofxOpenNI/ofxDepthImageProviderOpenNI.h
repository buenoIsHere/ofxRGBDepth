/*
 *  ofxRGBDepthFrameProviderOpenNI.h
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by James George on 3/13/12.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxOpenNI.h"
#include "ofxDepthImageProvider.h"

class ofxDepthImageProviderOpenNI : public ofxDepthImageProvider {
  public:

	void setup();
	void update();
	int maxDepth();	
	void close();
    
  protected:
	ofxOpenNIContext	recordContext;
	ofxDepthGenerator	recordDepth;
	ofxIRGenerator		recordImage;

};
