/*
 *  ofxTLVideoDepthAlignmentScrubber.cpp
 *  RGBDPostAlign
 *
 *  Created by James George on 11/16/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLVideoDepthAlignmentScrubber.h"
#include "ofxTimeline.h"

ofxTLVideoDepthAlignmentScrubber::ofxTLVideoDepthAlignmentScrubber(){
	videoSequence = NULL;
	depthSequence = NULL;
	selectedPairIndex = -1;
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
	vector<VideoDepthPair>& alignedFrames = pairSequence.getPairs();
	for(int i = 0; i < alignedFrames.size(); i++){
		int videoFrame;
		if(depthSequence->doFramesHaveTimestamps()){
            float videoPercent = alignedFrames[i].videoFrame / (videoSequence->getPlayer().getDuration()*1000.0);
			videoFrame = videoPercent * videoSequence->getPlayer().getTotalNumFrames();
		}
		else{
			videoFrame = alignedFrames[i].videoFrame;
		}
        
		int screenX = screenXForIndex( videoFrame );
		if(i == selectedPairIndex){
			ofSetColor(timeline->getColors().textColor);
		}
		else{
			ofSetColor(timeline->getColors().keyColor);
		}
		
		ofLine(screenX, bounds.y, 
               screenX, bounds.y+bounds.height);
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

void ofxTLVideoDepthAlignmentScrubber::selectPercent(float percent){
	selectedPercent = percent;
	selectedVideoFrame = normalizedXtoScreenX(percent, zoomBounds);
}

void ofxTLVideoDepthAlignmentScrubber::keyPressed(ofKeyEventArgs& args){
	if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE ){
		if(selectedPairIndex != -1){
			removeAlignmentPair(selectedPairIndex);
			selectedPairIndex = -1;
		}
	}
}

void ofxTLVideoDepthAlignmentScrubber::mousePressed(ofMouseEventArgs& args){
	vector<VideoDepthPair> & alignedFrames = pairSequence.getPairs();
	for(int i = 0; i < alignedFrames.size(); i++){
		int videoFrame;
		if(depthSequence->doFramesHaveTimestamps()){
			videoFrame = videoSequence->getPlayer().getTotalNumFrames() * alignedFrames[i].videoFrame / (videoSequence->getPlayer().getDuration()*1000.0);
		}
		else{
			videoFrame = alignedFrames[i].videoFrame;
		}
		int screenX = screenXForIndex( videoFrame );
		if(abs(args.x - screenX) < 5){
			selectedPairIndex = i;
			return;;
		}
	}
	
	selectedPairIndex = -1;
}

void ofxTLVideoDepthAlignmentScrubber::mouseMoved(ofMouseEventArgs& args){
}

void ofxTLVideoDepthAlignmentScrubber::mouseDragged(ofMouseEventArgs& args, bool snapped){
	if(ready() && bounds.inside(args.x, args.y)){
		selectedPercent = screenXtoNormalizedX(args.x);
		selectedVideoFrame = indexForScreenX(args.x);
		updateSelection();
	}
}

void ofxTLVideoDepthAlignmentScrubber::updateSelection(){

	videoSequence->selectFrame(selectedVideoFrame);
	if(depthSequence->doFramesHaveTimestamps()){
		long selectedVideoTime = 1000*videoSequence->getCurrentTime();
		selectedDepthFrame = depthSequence->frameForTime( pairSequence.getDepthFrameForVideoFrame(selectedVideoTime) );
		depthSequence->selectFrame(selectedDepthFrame);
	}
	else{
		selectedDepthFrame = pairSequence.getDepthFrameForVideoFrame(selectedVideoFrame);
		depthSequence->selectFrame(selectedDepthFrame);
	}
}

void ofxTLVideoDepthAlignmentScrubber::mouseReleased(ofMouseEventArgs& args){
}

void ofxTLVideoDepthAlignmentScrubber::registerCurrentAlignment(){
	if(depthSequence->doFramesHaveTimestamps()){
        double videoSeconds = videoSequence->getCurrentTime();
        long depthMillis = depthSequence->getSelectedTimeInMillis();
        cout << "video seconds " << videoSeconds << "  " << depthMillis << endl;
		pairSequence.addAlignedTime(long(1000*videoSeconds), depthMillis);
	}
	else{
		pairSequence.addAlignedFrames(videoSequence->getSelectedFrame(), depthSequence->getSelectedFrame());
	}
	
	save();
}

//void ofxTLVideoDepthAlignmentScrubber::addAlignedPair(int videoFrame, int depthFrame){
//
//
//	pairSequence.addAlignedFrame(videoFrame, depthFrame);
//	
//	save();
//}

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
        pairSequence.reset();
		ofLogError("ofxTLVideoDepthAlignmentScrubber -- loading no save file");
		return;
	}

	pairSequence.loadPairingFile(xmlFileName);
}

vector<VideoDepthPair> & ofxTLVideoDepthAlignmentScrubber::getPairs(){
	return pairSequence.getPairs();
}

ofxRGBDVideoDepthSequence& ofxTLVideoDepthAlignmentScrubber::getPairSequence(){
	return pairSequence;
}

bool ofxTLVideoDepthAlignmentScrubber::ready(){
	return videoSequence != NULL && depthSequence != NULL && pairSequence.ready();
}


