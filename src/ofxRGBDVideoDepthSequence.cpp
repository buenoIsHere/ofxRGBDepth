/*
 *  ofxRGBDVideoDepthSequence.cpp
 *  RGBDVisualize
 *
 *  Created by James George on 11/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxRGBDVideoDepthSequence.h"
#include "ofxXmlSettings.h"

bool pairsort(VideoDepthPair frameA, VideoDepthPair frameB){
	return frameA.videoFrame < frameB.videoFrame;
}

ofxRGBDVideoDepthSequence::ofxRGBDVideoDepthSequence(){
}

ofxRGBDVideoDepthSequence::~ofxRGBDVideoDepthSequence(){
}

void ofxRGBDVideoDepthSequence::savePairingFile(string pairFileXml){
	ofxXmlSettings settings;
	for(int i = 0; i < alignedFrames.size(); i++){
		settings.addTag("pair");
		settings.pushTag("pair", i);
		
		if(alignedFrames[i].isTimeBased){
			settings.addValue("videoMillis", alignedFrames[i].videoFrame);
			settings.addValue("depthMillis", alignedFrames[i].depthFrame);
		}
		else{
			settings.addValue("video", alignedFrames[i].videoFrame);
			settings.addValue("depth", alignedFrames[i].depthFrame);
		}
		settings.popTag();
	}
	settings.saveFile(pairFileXml);	
}

bool ofxRGBDVideoDepthSequence::loadPairingFile(string pairFileXml){
	
    alignedFrames.clear();//AP: Fixed bug loading new comp! 
	ofxXmlSettings settings;
	if(settings.loadFile(pairFileXml)){
		int numPairs = settings.getNumTags("pair");
		for(int i = 0; i < numPairs; i++){
			settings.pushTag("pair", i);
			VideoDepthPair p;
			p.isTimeBased = settings.getNumTags("videoMillis") != 0;
			if(p.isTimeBased){
				p.videoFrame = settings.getValue("videoMillis", 0);
				p.depthFrame = settings.getValue("depthMillis", 0);
			}
			else{
				p.videoFrame = settings.getValue("video", 0);
				p.depthFrame = settings.getValue("depth", 0);
			}
			alignedFrames.push_back(p);			
			settings.popTag();
		}
		return true;
	}
	else{
		ofLogError("ofxRGBDVideoDepthSequence -- error loading file " + pairFileXml);
		return false;
	}	
}

void ofxRGBDVideoDepthSequence::reset(){
    alignedFrames.clear();
}

void ofxRGBDVideoDepthSequence::addAlignedFrames(int videoFrame, int depthFrame){
	VideoDepthPair pair;
	pair.isTimeBased = false;
	pair.videoFrame = videoFrame;
	pair.depthFrame = depthFrame;
	addAlignedPair(pair);
}

void ofxRGBDVideoDepthSequence::addAlignedTime(int videoMillis, int depthMillis){
	cout << "added aligned time of video " << videoMillis << " to depth " << depthMillis << endl;
	VideoDepthPair pair;
	pair.isTimeBased = true;
	pair.videoFrame = videoMillis;
	pair.depthFrame = depthMillis;
	addAlignedPair(pair);
}

bool ofxRGBDVideoDepthSequence::isSequenceTimebased(){
	if(alignedFrames.size() == 0){
		return false;
	}
	return alignedFrames[0].isTimeBased;
}

void ofxRGBDVideoDepthSequence::addAlignedPair(VideoDepthPair pair){
	alignedFrames.push_back(pair);
	sort(alignedFrames.begin(), alignedFrames.end(), pairsort);
}

void ofxRGBDVideoDepthSequence::removeAlignedPair(int index){
	alignedFrames.erase(alignedFrames.begin()+index);
}

bool ofxRGBDVideoDepthSequence::ready(){
	return (alignedFrames.size() > 0 &&  alignedFrames[0].isTimeBased) ||
		   (alignedFrames.size() > 1 && !alignedFrames[0].isTimeBased);
}

long ofxRGBDVideoDepthSequence::getDepthFrameForVideoFrame(long videoFrame){

	if(!ready()){
		return 0;
	}

    if(alignedFrames[0].isTimeBased && alignedFrames.size() == 1){
    	return (alignedFrames[0].depthFrame - alignedFrames[0].videoFrame) + videoFrame;  
    }
    
    int startIndex, endIndex;
    if(videoFrame < alignedFrames[0].videoFrame){
        startIndex = 0;
        endIndex = 1;
    }
    if(videoFrame > alignedFrames[alignedFrames.size()-1].videoFrame){
        startIndex = alignedFrames.size()-2;
        endIndex = alignedFrames.size()-1;
    }
    else {
        startIndex = 0;
        endIndex = 1;
        while(videoFrame > alignedFrames[endIndex].videoFrame){
            startIndex++;
            endIndex++;
        }
    }
    
    if(endIndex == alignedFrames.size()){
        startIndex--;
        endIndex--;
    }
    
    long mapping = ofMap(videoFrame, alignedFrames[startIndex].videoFrame, alignedFrames[endIndex].videoFrame,
                        alignedFrames[startIndex].depthFrame, alignedFrames[endIndex].depthFrame, false);	
    //		cout << "looking for video frame " << videoFrame << " mapped to depth " << mapping << " found to be between " << startIndex << " and " << endIndex <<endl;
    return mapping;
}

vector<VideoDepthPair> & ofxRGBDVideoDepthSequence::getPairs(){
	return alignedFrames;
}
