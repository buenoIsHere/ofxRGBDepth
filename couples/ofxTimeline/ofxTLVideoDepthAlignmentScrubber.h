/*
 *  ofxTLVideoDepthAlignmentScrubber.h
 *  RGBDPostAlign
 *
 *  Created by James George on 11/16/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxTLElement.h"
#include "ofxTLDepthImageSequence.h"
#include "ofxTLVideoPlayer.h"
#include "ofxRGBDVideoDepthSequence.h"

class ofxTLVideoDepthAlignmentScrubber : public ofxTLElement {
  public:
	
	ofxTLVideoDepthAlignmentScrubber();
	~ofxTLVideoDepthAlignmentScrubber();
	
	void setup();
	void draw();
	
	virtual void mousePressed(ofMouseEventArgs& args);
	virtual void mouseMoved(ofMouseEventArgs& args);
	virtual void mouseDragged(ofMouseEventArgs& args, bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args);
	
	virtual void keyPressed(ofKeyEventArgs& args);
	
	void load();
	void save();
	
	bool ready();
	
	//void addAlignedPair(int videoFrame, int depthFrame);
	void registerCurrentAlignment();
	void removeAlignmentPair(int index);
	
	vector<VideoDepthPair> & getPairs();
	ofxRGBDVideoDepthSequence& getPairSequence();
	
	ofxTLVideoPlayer* videoSequence;
	ofxTLDepthImageSequence* depthSequence;
	
	void selectPercent(float percent);
	
  protected:
	ofxRGBDVideoDepthSequence pairSequence;
	void updateSelection();

	int selectedPairIndex;
	
	int selectedVideoFrame;
	int selectedDepthFrame;
	int selectedPercent;
	
//	int getDepthFrameForVideoFrame(int videoFrame);
	
//	vector<VideoDepthPair> alignedFrames;
};