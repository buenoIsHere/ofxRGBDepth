/*
 *  ofxKinectPointcloudDecoder.h
 *  RGBDPost
 *
 *  Created by James George on 11/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "ofMain.h"

class ofxKinectPointcloudDecoder {
  public:
	unsigned short* readDepthFrame(string filename, unsigned short* outbuf = NULL);
	unsigned short* readDepthFrame(ofFile file, unsigned short* outbuf = NULL);
	
	ofImage readDepthFrametoImage(string filename);
	ofImage convertTo8BitImage(unsigned short* buf);
	
};