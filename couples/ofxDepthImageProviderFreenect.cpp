/*
 *  ofxRGBDepthFrameProviderFreenect.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProviderFreenect.h"


void ofxDepthImageProviderFreenect::setup(){
	bDeviceFound  = kinect.init(true, true); // shows infrared instead of RGB video image
	bDeviceFound &= kinect.open();
	cout << "opened kinect? " << bDeviceFound << endl;
}

void ofxDepthImageProviderFreenect::update(){
	kinect.update();
	
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
//		cout << "new kinect frame!" << endl;
		bNewFrame = true;
		bDepthImageDirty = true;
		depthPixels.setFromPixels(kinect.getRawDepthPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
		rawIRImage.setFromPixels(kinect.getPixelsRef());
		rawIRImage.update();
	}		
}

int ofxDepthImageProviderFreenect::maxDepth(){
	return 10000; //taken from looking into how ofxKinect calculates it's look up tables.
}