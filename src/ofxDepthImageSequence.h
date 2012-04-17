//
//  ofxDepthImageSequence.h
//  RGBDVisualize
//
//  Created by James George on 4/16/12.
//

#pragma once

#include "ofMain.h"
#include "ofxDepthImageCompressor.h"
typedef struct {
    string path;
    ofShortPixels pix;
    long timestamp;
    int frameNumber;
} DepthImage;

class ofxDepthImageSequence {
  public:
    ofxDepthImageSequence();
    ~ofxDepthImageSequence();

    bool loadSequence(string sequenceDirectory);
    bool isLoaded();
    bool doFramesHaveTimestamps();
    
    int getCurrentFrame();
    long getCurrentMilliseconds();
    float getCurrentSeconds();
    
    int frameForTime(long timeInMillis);
    
	void selectFrame(int frame);
	void selectTime(float timeInSeconds);
	void selectTime(long timeInMillis);
	
    long getDurationInMillis();
    float getDurationInSeconds();

    ofShortPixels& getPixels();
    vector<DepthImage>& getImageArray();
    
  protected:
    ofxDepthImageCompressor compressor;
    bool sequenceLoaded;
    bool framesHaveTimestamps;
    
    vector<DepthImage> images;
    int selectedFrame;
    ofShortPixels pixels;
    int currentFrame;
    
    void loadFrame(int frame);
    long durationInMillis;
};