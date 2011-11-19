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
		settings.addValue("video", alignedFrames[i].videoFrame);
		settings.addValue("depth", alignedFrames[i].depthFrame);
		settings.popTag();
	}
	
	settings.saveFile(pairFileXml);	
}

void ofxRGBDVideoDepthSequence::loadPairingFile(string pairFileXml){
	
	ofxXmlSettings settings;
	if(settings.loadFile(pairFileXml)){
		int numPairs = settings.getNumTags("pair");
		for(int i = 0; i < numPairs; i++){
			settings.pushTag("pair", i);
			VideoDepthPair p;
			p.videoFrame = settings.getValue("video", 0);
			p.depthFrame = settings.getValue("depth", 0);
			alignedFrames.push_back(p);			
			settings.popTag();
			cout << "ofxRGBDVideoDepthSequence -- Loaded Pairs vid. " << p.videoFrame << " dep. " << p.depthFrame << endl;
		}
	}
	else{
		ofLogError("ofxRGBDVideoDepthSequence -- error loading file " + pairFileXml);
	}	
}

void ofxRGBDVideoDepthSequence::addAlignedPair(int videoFrame, int depthFrame){
	VideoDepthPair pair;
	pair.videoFrame = videoFrame;
	pair.depthFrame = depthFrame;
	addAlignedPair(pair);
}

void ofxRGBDVideoDepthSequence::addAlignedPair(VideoDepthPair pair){
	alignedFrames.push_back(pair);
	sort(alignedFrames.begin(), alignedFrames.end(), pairsort);
	
	cout << "addded " << pair.videoFrame << " " << pair.depthFrame << endl;
	
}

void ofxRGBDVideoDepthSequence::removeAlignedPair(int index){
	alignedFrames.erase(alignedFrames.begin()+index);
}

bool ofxRGBDVideoDepthSequence::ready(){
	alignedFrames.size() >= 2;
}

int ofxRGBDVideoDepthSequence::getDepthFrameForVideoFrame(int videoFrame){

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
		while(videoFrame > alignedFrames[endIndex].videoFrame ){
			startIndex++;
			endIndex++;
		}
	}
	
	if(endIndex == alignedFrames.size()){
		startIndex--;
		endIndex--;
	}
	
	//cout << "looking for index " << videoFrame << " found to be between " << startIndex << " and " << endIndex << endl;
	
	return ofMap(videoFrame, alignedFrames[startIndex].videoFrame, alignedFrames[endIndex].videoFrame,
				 alignedFrames[startIndex].depthFrame, alignedFrames[endIndex].depthFrame, false);	
}

vector<VideoDepthPair> & ofxRGBDVideoDepthSequence::getPairs(){
	return alignedFrames;
}
