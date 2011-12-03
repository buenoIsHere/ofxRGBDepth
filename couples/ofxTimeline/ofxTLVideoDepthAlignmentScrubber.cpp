/*
 *  ofxTLVideoDepthAlignmentScrubber.cpp
 *  RGBDPostAlign
 *
 *  Created by James George on 11/16/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLVideoDepthAlignmentScrubber.h"
//bool pairsort(VideoDepthPair frameA, VideoDepthPair frameB){
//	return frameA.videoFrame < frameB.videoFrame;
//}

ofxTLVideoDepthAlignmentScrubber::ofxTLVideoDepthAlignmentScrubber(){
	videoSequence = NULL;
	depthSequence = NULL;
}

ofxTLVideoDepthAlignmentScrubber::~ofxTLVideoDepthAlignmentScrubber(){

}
	
void ofxTLVideoDepthAlignmentScrubber::setup(){
	enable();
}

void ofxTLVideoDepthAlignmentScrubber::draw(){
	
	if(!ready()){
		ofPushStyle();
		ofSetColor(255, 100, 0, 30);
		ofRect(bounds);
		ofPopStyle();
		return;
	}
		
	ofPushStyle();
	ofSetColor(255, 100, 0);
	vector<VideoDepthPair> & alignedFrames = pairSequence.getPairs();
	for(int i = 0; i < alignedFrames.size(); i++){
		int screenX = screenXForIndex( alignedFrames[i].videoFrame);
		ofLine(screenX, bounds.y, screenX, bounds.y+bounds.height);
		ofDrawBitmapString("video: " + ofToString(alignedFrames[i].videoFrame), ofPoint(screenX+10, bounds.y+15));
		ofDrawBitmapString("depth: " + ofToString(alignedFrames[i].depthFrame), ofPoint(screenX+10, bounds.y+35));
	}
	
	ofSetColor(0, 125, 255);
	int selectedScreenX = normalizedXtoScreenX(selectedPercent);
	ofLine(selectedScreenX, bounds.y, selectedScreenX, bounds.y+bounds.height);
	ofDrawBitmapString("sel.video: " + ofToString(selectedVideoFrame), ofPoint(selectedScreenX+10, bounds.y+55));
	ofDrawBitmapString("sel.depth: " + ofToString(selectedDepthFrame), ofPoint(selectedScreenX+10, bounds.y+75));
	
	ofPopStyle();
}

void ofxTLVideoDepthAlignmentScrubber::mousePressed(ofMouseEventArgs& args){
}

void ofxTLVideoDepthAlignmentScrubber::mouseMoved(ofMouseEventArgs& args){
}

void ofxTLVideoDepthAlignmentScrubber::mouseDragged(ofMouseEventArgs& args){
	if(ready() && bounds.inside(args.x, args.y)){
		selectedPercent = screenXtoNormalizedX(args.x);
		selectedVideoFrame = indexForScreenX(args.x);
		selectedDepthFrame = pairSequence.getDepthFrameForVideoFrame(selectedVideoFrame);
		videoSequence->selectFrame(selectedVideoFrame);
		depthSequence->selectFrame(selectedDepthFrame);
	}
}

void ofxTLVideoDepthAlignmentScrubber::mouseReleased(ofMouseEventArgs& args){
}

void ofxTLVideoDepthAlignmentScrubber::addAlignedPair(int videoFrame, int depthFrame){


	pairSequence.addAlignedPair(videoFrame, depthFrame);
	
	save();
}

void ofxTLVideoDepthAlignmentScrubber::removeAlignmentPair(int index){
	
	pairSequence.removeAlignedPair(index);
	
	save();
}

void ofxTLVideoDepthAlignmentScrubber::save(){
	if(xmlFileName == ""){
		ofLogError("ofxTLVideoDepthAlignmentScrubber -- saving with no save file");
		return;
	}

	pairSequence.savePairingFile(xmlFileName);

}

void ofxTLVideoDepthAlignmentScrubber::load(){
	if(xmlFileName == ""){
		ofLogError("ofxTLVideoDepthAlignmentScrubber -- loading no save file");
		return;
	}

	pairSequence.loadPairingFile(xmlFileName);
}

vector<VideoDepthPair> & ofxTLVideoDepthAlignmentScrubber::getPairs(){
	return pairSequence.getPairs();
}

bool ofxTLVideoDepthAlignmentScrubber::ready(){
	return videoSequence != NULL && depthSequence != NULL && pairSequence.ready();
}
