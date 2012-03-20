/*
 *  ofxRGBDepthFrameProviderOpenNI.h
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
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

  protected:
	ofxOpenNIContext	recordContext;
	ofxDepthGenerator	recordDepth;
	ofxIRGenerator		recordImage;

};
