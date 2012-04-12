/*
 *  ofxRGBDepthFrameProviderOpenNI.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProviderOpenNI.h"

void ofxDepthImageProviderOpenNI::setup(){
	bDeviceFound  = recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
	bDeviceFound &= recordDepth.setup(&recordContext);
	bDeviceFound &= recordImage.setup(&recordContext);
    
    if(!bDeviceFound){
    	ofLogError("ofxDepthImageProviderOpenNI -- OpenNI Device Failed");
    }
}

void ofxDepthImageProviderOpenNI::update(){
	recordContext.update();
	recordImage.update();
	rawIRImage.setFromPixels(recordImage.getIRPixels(), 640,480, OF_IMAGE_GRAYSCALE);
	recordDepth.update();

	if(recordDepth.isFrameNew()){
		bNewFrame = true;
		bDepthImageDirty = true;
		depthPixels.setFromPixels( (unsigned short*) recordDepth.getRawDepthPixels(), 640, 480, OF_IMAGE_GRAYSCALE);
	}
}


int ofxDepthImageProviderOpenNI::maxDepth(){
	return recordDepth.getMaxDepth();
}

void ofxDepthImageProviderOpenNI::close(){
	recordContext.shutdown();
	
}