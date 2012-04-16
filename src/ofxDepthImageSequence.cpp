//
//  ofxDepthImageSequence.cpp
//  RGBDVisualize
//
//  Created by James George on 4/16/12.
//  Copyright (c) 2012 FlightPhase. All rights reserved.
//

#include "ofxDepthImageSequence.h"

ofxDepthImageSequence::ofxDepthImageSequence(){
    sequenceLoaded = false;
    framesHaveTimestamps = false;

}

ofxDepthImageSequence::~ofxDepthImageSequence(){
    
}

bool ofxDepthImageSequence::loadSequence(string sequenceDirectory){
    
	ofDirectory sequenceList(sequenceDirectory);
	if(!sequenceList.exists()){
		ofLogError("ofxDepthImageSequence -- sequence directory " + sequenceDirectory + " does not exist!");
		return false;
	}
    
	if(sequenceLoaded){
		images.clear();
		sequenceLoaded = false;
	}

	sequenceList.allowExt("png");
	int numFiles = sequenceList.listDir();
	if(numFiles == 0){
		ofLogError("ofxTLDepthImageSequence -- sequence directory " + sequenceDirectory + " is empty!");
		return false;
	}
	
	bool checkedForTimestamp = false;
	for(int i = 0; i < numFiles; i++){
        //backwards compat...
		if(sequenceList.getName(i).find("poster") != string::npos){
			ofLogWarning("discarding poster frame " + sequenceList.getPath(i) );
			continue;
		}
		
		if(!checkedForTimestamp){
			framesHaveTimestamps = sequenceList.getName(i).find("millis") != string::npos;
			checkedForTimestamp = true;
			ofLogVerbose("Frames have timestamps? " + string((framesHaveTimestamps ? "yes!" : "no :(")) );
		}
		
		DepthImage img;
		//t.setup(i, thumbDirectory);
		img.path = sequenceList.getPath(i);
		
		if(framesHaveTimestamps){
			vector<string> split = ofSplitString(sequenceList.getName(i), "_", true, true);
			for(int l = 0; l < split.size(); l++){
				if(split[l] == "millis"){
					img.timestamp = ofToInt(split[l+1]);
				}
			}
		}
//        cout << "loaded frame " << i << " with path " << img.path << endl;
		images.push_back( img );
	}
    
    currentFrame = 0;
    if(framesHaveTimestamps){
	    durationInMillis = images[images.size()-1].timestamp;
    }

	ofLogVerbose("sequence is loaded " + ofToString( images.size() ));
    sequenceLoaded = true;
    
	return true;
}

long ofxDepthImageSequence::getDurationInMillis(){
    return durationInMillis;
}

float ofxDepthImageSequence::getDurationInSeconds(){
    return durationInMillis / 1000.0;
}

int ofxDepthImageSequence::getCurrentFrame(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getCurrentFrame() -- sequence not loaded");
        return 0;
    }
    
}

long ofxDepthImageSequence::getCurrentMilliseconds(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getCurrentMilliseconds() -- sequence not loaded");
        return 0;
    }
}

float ofxDepthImageSequence::getCurrentSeconds(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getCurrentSeconds() -- sequence not loaded");
        return 0;
    }
}

int ofxDepthImageSequence::frameForTime(long timeInMillis){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::frameForTime() -- sequence not loaded");
        return 0;
    }
    
	if(!framesHaveTimestamps){
		ofLogError("ofxTLDepthImageSequence -- can't select frame for time if there are no timestamps");
		return 0;
	}
	
	for(int i = 1; i < images.size(); i++){
		if(images[i].timestamp > timeInMillis){
			return i-1;
		}
	}
    
	return images.size()-1;
    
}

void ofxDepthImageSequence::selectFrame(int frame){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::selectFrame() -- sequence not loaded");
        return;
    }
    
    if(frame < 0 || frame >= images.size()){
        ofLogError("ofxDepthImageSequence::selectFrame() -- frame out of range");
        return;
    }
    
    currentFrame = frame;
}

void ofxDepthImageSequence::selectTime(float timeInSeconds){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::selectTime(int) -- sequence not loaded");
        return;
    }
    if(!framesHaveTimestamps){
        ofLogError("ofxDepthImageSequence::selectTime() -- no timestamps!");
        return;        
    }
    if(timeInSeconds < 0 || timeInSeconds > getDurationInSeconds()){
        ofLogError("ofxDepthImageSequence::selectTime() -- time out of range!");
        return;
    }
    
    selectFrame(frameForTime(long(timeInSeconds*1000)));
}

void ofxDepthImageSequence::selectTime(long timeInMillis){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::selectTime(long) -- sequence not loaded");
        return;
    }
    if(!framesHaveTimestamps){
        ofLogError("ofxDepthImageSequence::selectTime() -- no timestamps!");
        return;        
    }    
    if(timeInMillis < 0 || timeInMillis > durationInMillis){
        ofLogError("ofxDepthImageSequence::selectTime() -- time out of range!");
        return;
    }    
    
    selectFrame(frameForTime(timeInMillis));
}

ofShortPixels& ofxDepthImageSequence::getPixels(){
    if(!sequenceLoaded){
        ofLogError("ofxDepthImageSequence::getPixels() -- sequence not loaded");
        return dummy;
    }
    
	if(!images[currentFrame].pix.isAllocated()){
    	compressor.readCompressedPng(images[currentFrame].path, 
                                     images[currentFrame].pix);
    }
    return images[currentFrame].pix;
}

bool ofxDepthImageSequence::isLoaded(){
    return sequenceLoaded;
}

bool ofxDepthImageSequence::doFramesHaveTimestamps(){
	return framesHaveTimestamps;
}

