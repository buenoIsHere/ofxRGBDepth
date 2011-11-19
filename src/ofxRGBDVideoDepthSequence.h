/*
 *  ofxRGBDVideoDepthSequence.h
 *  RGBDVisualize
 *
 *  Created by James George on 11/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

//TODO somehow account for timestamping

#pragma once

#include "ofMain.h"

typedef struct{
	int videoFrame;
	int depthFrame;
} VideoDepthPair;

class ofxRGBDVideoDepthSequence {
  public:
	ofxRGBDVideoDepthSequence();
	~ofxRGBDVideoDepthSequence();
	
	void savePairingFile(string pairFileXml);
	void loadPairingFile(string pairFileXml);
	
	bool ready();
	void addAlignedPair(int videoFrame, int depthFrame);
	void addAlignedPair(VideoDepthPair pair);
	
	void removeAlignedPair(int index);

	int getDepthFrameForVideoFrame(int videoFrame);

	vector<VideoDepthPair> & getPairs();

  protected:
	vector<VideoDepthPair> alignedFrames;
};