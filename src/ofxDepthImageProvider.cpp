/*
 *  ofxRGBDepthFrameProvider.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProvider.h"

ofxDepthImageProvider::ofxDepthImageProvider(){
	bUseRainbow = true;
    
	bDepthImageDirty = false;
	bDeviceFound = false;
	bNewFrame = false;
	
	depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	rawIRImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	depthImage.allocate(640, 480, OF_IMAGE_COLOR);
}

//void ofxDepthImageProvider::setDepthModeRainbow(bool useRainbow){
//	if(useRainbow != bUseRainbow){
//		bUseRainbow = useRainbow;
//		bDepthImageDirty = true;
//	}
//}

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

/*
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
*/
