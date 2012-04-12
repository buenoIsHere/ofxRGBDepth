/*
 *  ofxRGBDepthFrameProvider.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProvider.h"


//COLORING
//taken from OpenNI
void ofxDepthImageProvider::createRainbowPallet() {
	unsigned char r, g, b;
	memset(LUTR, 0, 256);
	memset(LUTG, 0, 256);
	memset(LUTB, 0, 256);
	
	for (int i=1; i<255; i++) {
		if (i<=29) {
			r = (unsigned char)(129.36-i*4.36);
			g = 0;
			b = (unsigned char)255;
		}
		else if (i<=86) {
			r = 0;
			g = (unsigned char)(-133.54+i*4.52);
			b = (unsigned char)255;
		}
		else if (i<=141) {
			r = 0;
			g = (unsigned char)255;
			b = (unsigned char)(665.83-i*4.72);
		}
		else if (i<=199) {
			r = (unsigned char)(-635.26+i*4.47);
			g = (unsigned char)255;
			b = 0;
		}
		else {
			r = (unsigned char)255;
			g = (unsigned char)(1166.81-i*4.57);
			b = 0;
		}
		LUTR[i] = r;
		LUTG[i] = g;
		LUTB[i] = b;
	}
}

ofxDepthImageProvider::ofxDepthImageProvider(){
	bUseRainbow = true;
    
	bDepthImageDirty = false;
	bDeviceFound = false;
	bNewFrame = false;
	
	depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	rawIRImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	depthImage.allocate(640, 480, OF_IMAGE_COLOR);
	createRainbowPallet();

}

void ofxDepthImageProvider::setDepthModeRainbow(bool useRainbow){
	if(useRainbow != bUseRainbow){
		bUseRainbow = useRainbow;
		bDepthImageDirty = true;
	}
}

bool ofxDepthImageProvider::isFrameNew(){
	bool ret = bNewFrame;
	bNewFrame = false;
	return ret;	
}

bool ofxDepthImageProvider::deviceFound(){
	return bDeviceFound;
}

ofShortPixels& ofxDepthImageProvider::getRawDepth(){
	return depthPixels;
}

ofImage& ofxDepthImageProvider::getRawIRImage(){
	return rawIRImage;	
}

ofImage& ofxDepthImageProvider::getDepthImage(){
    
	if(bDepthImageDirty){
		
		if(bUseRainbow){		
			int max_depth = maxDepth();
			for(int i = 0; i < 640*480; i++){
				int lookup = depthPixels.getPixels()[i] / (max_depth / 256);
				//int lookup = ofMap( depthPixels.getPixels()[i], 0, max_depth, 0, 255, true);
				depthImage.getPixels()[(i*3)+0] = LUTR[lookup];
				depthImage.getPixels()[(i*3)+1] = LUTG[lookup];
				depthImage.getPixels()[(i*3)+2] = LUTB[lookup];
			}
		}
		else{
			//TODO!
		}
		depthImage.update();
		bDepthImageDirty  = false;
	}
	return depthImage;
}


