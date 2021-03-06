/*
 *  ofxTLDepthImageSequence.cpp
 *  timelineExampleVideoPlayer
 *
 *  Created by James George on 11/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxTLDepthImageSequence.h"
#include "ofxTimeline.h"

ofxTLDepthImageSequence::ofxTLDepthImageSequence(){
	sequenceLoaded = false;
//	currentDepthRaw = NULL;
//	thumbnailDepthRaw = NULL;
	selectedFrame = 0;
	thumbsEnabled = true;
	framesHaveTimestamps = false;
    thumbnailUpdatedZoomLevel = -1;
    thumbnailUpdatedWidth = -1;
    thumbnailUpdatedHeight = -1;
    currentlyZooming = false;

}

ofxTLDepthImageSequence::~ofxTLDepthImageSequence(){
//	if(currentDepthRaw != NULL){
//		delete currentDepthRaw;
//	}
//	if(thumbnailDepthRaw != NULL){
//		delete thumbnailDepthRaw;
//	}
}

void ofxTLDepthImageSequence::setup(){
	
	enable();
	currentDepthRaw.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	thumbnailDepthRaw.allocate(640, 480, OF_IMAGE_GRAYSCALE);
    
	currentDepthImage = decoder.convertTo8BitImage(currentDepthRaw);
}

void ofxTLDepthImageSequence::enable(){
	ofxTLElement::enable();
	ofxTLRegisterPlaybackEvents(this);

}

void ofxTLDepthImageSequence::disable(){
	ofxTLElement::disable();
	ofxTLRemovePlaybackEvents(this);
}

void ofxTLDepthImageSequence::update(ofEventArgs& args){
    if(!sequenceLoaded || timeline == NULL){
    	return;
    }
    
	//if we are on a timebased timeline and our frames have timestamps
	//prefer selecting based on the time as we'll get more accurate playback speeds
	if(!timeline->getIsFrameBased() && framesHaveTimestamps){ 
		selectTime( timeline->getCurrentTime()*1000 );
	}
	else{
		selectFrame( timeline->getCurrentFrame() );
	}
}

void ofxTLDepthImageSequence::draw(){

	if(getDrawRect().height < 10){
		return;
	}
    
    if(!sequenceLoaded){
		ofPushStyle();
        ofSetColor(timeline->getColors().disabledColor);
        ofRect(getDrawRect());
        ofPopStyle();
        return;
    }
	
    calculateFramePositions();
    if(thumbsEnabled && !ofGetMousePressed() && 
       (thumbnailUpdatedZoomLevel != zoomBounds ||
        thumbnailUpdatedWidth != getDrawRect().width ||
        thumbnailUpdatedHeight != getDrawRect().height) ) {
           generateVideoThumbnails();	
    }

	ofPushStyle();
	if(thumbsEnabled){
		ofSetColor(255);
		for(int i = 0; i < videoThumbs.size(); i++){
			if(videoThumbs[i].visible){
				videoThumbs[i].thumb.draw(videoThumbs[i].displayRect);
			}
		}
	}
	
	for(int i = 0; i < videoThumbs.size(); i++){
		if(videoThumbs[i].visible){
			if(!thumbsEnabled){
				ofFill();
				ofSetColor(0);
				ofRect(videoThumbs[i].displayRect);
			}
			ofNoFill();
			ofSetColor(255, 150, 0);
			ofDrawBitmapString(ofToString(videoThumbs[i].framenum), videoThumbs[i].displayRect.x+5, videoThumbs[i].displayRect.y+15);
			ofRect(videoThumbs[i].displayRect);
		}
	}
	
	int selectedFrameX = screenXForIndex(selectedFrame, videoThumbs.size());
	ofSetColor(0, 125, 255);
	ofLine(selectedFrameX, bounds.y, selectedFrameX, bounds.y+bounds.height);
	ofDrawBitmapString("frame " + ofToString(selectedFrame), selectedFrameX, bounds.y+30);
    if(framesHaveTimestamps){
        ofDrawBitmapString("second " + ofToString(videoThumbs[selectedFrame].timestamp/1000.0), selectedFrameX, bounds.y+48);
    }
	
	ofPopStyle();
}

void ofxTLDepthImageSequence::zoomStarted(ofxTLZoomEventArgs& args){
    currentlyZooming = true;
	ofxTLElement::zoomStarted(args);
	calculateFramePositions();
}

void ofxTLDepthImageSequence::zoomDragged(ofxTLZoomEventArgs& args){
	ofxTLElement::zoomDragged(args);
	calculateFramePositions();
}

void ofxTLDepthImageSequence::zoomEnded(ofxTLZoomEventArgs& args){
    currentlyZooming = false;
	ofxTLElement::zoomEnded(args);
	calculateFramePositions();
	if(thumbsEnabled){
		generateVideoThumbnails();	
	}
}

void ofxTLDepthImageSequence::mousePressed(ofMouseEventArgs& args){
	ofxTLElement::mousePressed(args);
}

void ofxTLDepthImageSequence::mouseMoved(ofMouseEventArgs& args){
	ofxTLElement::mouseMoved(args);
}

void ofxTLDepthImageSequence::mouseDragged(ofMouseEventArgs& args, bool snapped){
    
    if(!isLoaded()) return;
    
	if( bounds.inside(args.x, args.y) ){
		int index = indexForScreenX(args.x, videoThumbs.size());
		selectFrame(index);
		timeline->setCurrentFrame(index);
        timeline->flagUserChangedValue();
	}
}

void ofxTLDepthImageSequence::keyPressed(ofKeyEventArgs& args){
	if(hover){
		if(args.key == OF_KEY_LEFT){
			selectFrame(MAX(selectedFrame-1, 0));
		}
		else if(args.key == OF_KEY_RIGHT){
			selectFrame(MIN(selectedFrame+1, videoThumbs.size()-1));		
		}
	}
}

void ofxTLDepthImageSequence::playbackStarted(ofxTLPlaybackEventArgs& args){
	ofAddListener(ofEvents().update, this, &ofxTLDepthImageSequence::update);
}

void ofxTLDepthImageSequence::playbackEnded(ofxTLPlaybackEventArgs& args){
	ofRemoveListener(ofEvents().update, this, &ofxTLDepthImageSequence::update);
}

void ofxTLDepthImageSequence::playbackLooped(ofxTLPlaybackEventArgs& args){
}

void ofxTLDepthImageSequence::selectFrame(int frame){
	selectedFrame = ofClamp(frame, 0, videoThumbs.size()-1);
	decoder.readCompressedPng(videoThumbs[selectedFrame].sourcepath, currentDepthRaw.getPixels());
	currentDepthImage = decoder.convertTo8BitImage(currentDepthRaw);
    frameIsNew = true;
}

void ofxTLDepthImageSequence::selectTime(long timeStampInMillis){
	selectFrame( frameForTime(timeStampInMillis) );
}

void ofxTLDepthImageSequence::selectTime(float timeInSeconds){
	selectFrame( frameForTime(timeInSeconds*1000) );
}

long ofxTLDepthImageSequence::getSelectedTimeInMillis(){
	return videoThumbs[selectedFrame].timestamp;
}

int ofxTLDepthImageSequence::frameForTime(long timeInMillis){
	
    if(!sequenceLoaded){
    	return 0;
    }
    
	if(!framesHaveTimestamps){
		ofLogError("ofxTLDepthImageSequence -- can't select frame for time if there are no timestamps");
		return 0;
	}
	
	for(int i = 1; i < videoThumbs.size(); i++){
		if(videoThumbs[i].timestamp > timeInMillis){
			return i-1;
		}
	}
	return videoThumbs.size()-1;
    
}

void ofxTLDepthImageSequence::mouseReleased(ofMouseEventArgs& args){
}

void ofxTLDepthImageSequence::drawRectChanged(){
	calculateFramePositions();
}

bool ofxTLDepthImageSequence::loadSequence(){
	ofFileDialogResult r = ofSystemLoadDialog("Load Depth Sequence Directory", true);
	if(r.bSuccess){
		return loadSequence(r.getPath());
	}
	return false;
}

bool ofxTLDepthImageSequence::isLoaded(){
	return sequenceLoaded;
}

bool ofxTLDepthImageSequence::isFrameNew(){
    bool result = frameIsNew;
    frameIsNew = false;
    return result;
}

bool ofxTLDepthImageSequence::loadSequence(string seqdir){
	ofDirectory sequenceList(seqdir);
	if(!sequenceList.exists()){
		ofLogError("ofxTLDepthImageSequence -- sequence directory " + seqdir + " does not exist!");
		return false;
	}
	
	thumbDirectory = seqdir + "/thumbs";
	ofDirectory thumdir(thumbDirectory);
	if(!thumdir.exists()){
		if(!thumdir.create(true)){
			ofLogError("ofxTLDepthImageSequence -- sequence directory " + seqdir + " cannot be created!");
			return false;
		}
	}
	
	if(sequenceLoaded){
		videoThumbs.clear();
		sequenceLoaded = false;
	}
	
	sequenceList.allowExt("png");
	int numFiles = sequenceList.listDir();
	bool checkedForTimestamp = false;
	if(numFiles == 0){
		ofLogError("ofxTLDepthImageSequence -- sequence directory " + seqdir + " is empty!");
		return false;
	}
//	cout << "TEST num file sis " << numFiles << " for " << seqdir << endl;  
	for(int i = 0; i < numFiles; i++){
		if(sequenceList.getName(i).find("poster") != string::npos){
			cout << "discarding poster frame " << sequenceList.getPath(i) << endl;
			continue;
		}
		
		if(!checkedForTimestamp){
			framesHaveTimestamps = sequenceList.getName(i).find("millis") != string::npos;
			checkedForTimestamp = true;
			cout << "Frames have timestamps?? " << (framesHaveTimestamps ? "yes!" : "no :(") << endl;
		}
		
		ofxTLVideoThumb	t;
		t.setup(i, thumbDirectory);
		t.sourcepath = sequenceList.getPath(i);
		
		if(framesHaveTimestamps){
			vector<string> split = ofSplitString(sequenceList.getName(i), "_", true, true);
			for(int l = 0; l < split.size(); l++){
				if(split[l] == "millis"){
					t.timestamp = ofToInt(split[l+1]);
				}
			}
		}
		 
		videoThumbs.push_back(t);
	}
	
//	cout << "sequence is loaded " << videoThumbs.size() << endl;
    //only generate if we already had a sequence loaded.
    if(sequenceLoaded){
        cout << "generating thumbnails" << endl;
        generateVideoThumbnails();
    }
	sequenceLoaded = true;
	videoThumbs[0].visible = true;
	generateThumbnailForFrame(0);
//	cout << "calculating frame positions" << endl;
	calculateFramePositions();
//	cout << "generating thumbnails" << endl;
	generateVideoThumbnails();
	return true;
}

void ofxTLDepthImageSequence::calculateFramePositions(){
    
//    cout << "calculating frame positions" << endl;
    
	if(timeline == NULL){
        ofLogError("ofxTLDepthImageSequence -- timeline is null!");
		return;
	}
	
	if(!sequenceLoaded){
		return;
	}
    
    if( bounds.height < 10){
        return;
    }
	
	int frameWidth = int( bounds.height * videoThumbs[0].targetWidth / videoThumbs[0].targetHeight );
	int totalPixels = int( bounds.width / zoomBounds.span() );
	int framesToShow = MAX(totalPixels / frameWidth, 1);
	int frameStep = MAX(videoThumbs.size() / framesToShow, 1); 
	int minPixelIndex = -(zoomBounds.min * totalPixels);
	
	for(int i = 0; i < videoThumbs.size(); i++){
		if(i % frameStep == 0){
			int screenX = screenXForIndex(i, videoThumbs.size());
			videoThumbs[i].displayRect = ofRectangle(screenX, bounds.y, frameWidth, bounds.height);
			videoThumbs[i].visible = videoThumbs[i].displayRect.x+videoThumbs[i].displayRect.width > 0 && videoThumbs[i].displayRect.x < bounds.width;
		}
		else {
			videoThumbs[i].visible = false;
		}
	}
}

void ofxTLDepthImageSequence::generateVideoThumbnails() {
    thumbnailUpdatedZoomLevel = zoomBounds;
    thumbnailUpdatedWidth = getDrawRect().width;
	thumbnailUpdatedHeight = getDrawRect().height; 
	for(int i = 0; i < videoThumbs.size(); i++){
		generateThumbnailForFrame(i);
	}
}

void ofxTLDepthImageSequence::generateThumbnailForFrame(int i){
	if(videoThumbs[i].visible && !videoThumbs[i].loaded){
		if(videoThumbs[i].exists){
			videoThumbs[i].load();
		}
		else {
			decoder.readCompressedPng(videoThumbs[i].sourcepath, thumbnailDepthRaw.getPixels());
			ofImage grayConverted = decoder.convertTo8BitImage(thumbnailDepthRaw);
			videoThumbs[i].create(grayConverted);
		}
	}
}

void ofxTLDepthImageSequence::toggleThumbs(){
	thumbsEnabled = !thumbsEnabled;
}

int ofxTLDepthImageSequence::getSelectedFrame(){
	return selectedFrame;
}

bool ofxTLDepthImageSequence::doFramesHaveTimestamps(){
	return framesHaveTimestamps;
}
