/*
 *  ofxTLDepthImageSequence.h
 *  timelineExampleVideoPlayer
 *
 *  Created by James George on 11/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxTLElement.h"
#include "ofxTLVideoThumb.h"
#include "ofxDepthImageCompressor.h"

class ofxTLDepthImageSequence : public ofxTLElement {
  public:	
	ofxTLDepthImageSequence();
	~ofxTLDepthImageSequence();

	void setup();
	void draw();

	void enable();
	void disable();

	vector<ofxTLVideoThumb> videoThumbs;

	virtual void mousePressed(ofMouseEventArgs& args);
	virtual void mouseMoved(ofMouseEventArgs& args);
	virtual void mouseDragged(ofMouseEventArgs& args, bool snapped);
	virtual void mouseReleased(ofMouseEventArgs& args);

	virtual void keyPressed(ofKeyEventArgs& args);
	
	virtual void zoomStarted(ofxTLZoomEventArgs& args);
	virtual void zoomDragged(ofxTLZoomEventArgs& args);
	virtual void zoomEnded(ofxTLZoomEventArgs& args);

	virtual void drawRectChanged();

	bool loadSequence();
	bool loadSequence(string sequenceDirectory);
	bool isLoaded();
	bool isFrameNew();
    
	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	
	ofImage currentDepthImage;
	//unsigned short* currentDepthRaw;
    ofShortPixels currentDepthRaw;
	//unsigned short* thumbnailDepthRaw;
    ofShortPixels thumbnailDepthRaw;
	
	int getSelectedFrame();
	
	int frameForTime(long timeInMillis);
	void selectFrame(int frame);
	void selectTime(float timeInSeconds);
	void selectTime(long timeStampInMillis);
	
	//only works if doFramesHaveMillis is true
	long getSelectedTimeInMillis();
	
	void toggleThumbs();
	
	bool doFramesHaveTimestamps();

  protected:
	bool framesHaveTimestamps;
	
	//only called during playback
	void update(ofEventArgs& args);
	
    ofRange thumbnailUpdatedZoomLevel;
    float thumbnailUpdatedWidth;
    float thumbnailUpdatedHeight;
    bool currentlyZooming;

	int selectedFrame;
	bool sequenceLoaded;
	
	bool thumbsEnabled;
	
	void calculateFramePositions();
	void generateVideoThumbnails();
	void generateThumbnailForFrame(int index);
	bool frameIsNew;
	
	string sequenceDirectory;
	string thumbDirectory;
	
	ofxDepthImageCompressor decoder;
};
