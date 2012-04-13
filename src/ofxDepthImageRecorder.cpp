/*
 *  ofxDepthImageRecorder.cpp
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxDepthImageRecorder.h"

#pragma mark Thread Implementation
void ofxRGBDEncoderThread::threadedFunction(){
    shutdown = false;
	while(isThreadRunning()){
		delegate->encoderThreadCallback();
	}
    shutdown = true;
}

void ofxRGBDRecorderThread::threadedFunction(){
    shutdown = false;
	while(isThreadRunning()){
		delegate->recorderThreadCallback();
	}
    shutdown = true;
}

#pragma mark Thread Implementation
ofxDepthImageRecorder::ofxDepthImageRecorder()
  : recorderThread(this),
	encoderThread(this)
{
	recording = false;
	lastFramePixs = NULL;
	encodingBuffer = NULL;
	framesToCompress = 0;
    compressingTakeIndex = 0;
}

ofxDepthImageRecorder::~ofxDepthImageRecorder(){
	if(lastFramePixs != NULL){
		delete lastFramePixs;
	}
}

void ofxDepthImageRecorder::setup(){
    folderCount = 0;
	currentFrame = 0;
	
	lastFramePixs = new unsigned short[640*480];
	memset(lastFramePixs, 0, sizeof(unsigned short)*640*480);

    recorderThread.startThread(true, false);
	encoderThread.startThread(true, false);
}

void ofxDepthImageRecorder::setRecordLocation(string directory, string filePrefix){
    if(numFramesWaitingSave() != 0 || numDirectoriesWaitingCompression() != 0){
        return;
    }
    
	targetDirectory = directory;
	ofDirectory dir(directory);
	if(!dir.exists()){
		dir.create(true);
	}
	targetFilePrefix = filePrefix;
	
    updateTakes();
}

vector<Take>& ofxDepthImageRecorder::getTakes(){
	return takes;
}

/*
vector<string> ofxDepthImageRecorder::getTakePaths(){
    /*
	ofDirectory dir = ofDirectory(targetDirectory);
	dir.listDir();
	dir.sort();
	vector<string> paths;
	for(int i = 0; i < dir.numFiles(); i++){
		paths.push_back(dir.getPath(i));
	}
	return paths;
}
    */

bool ofxDepthImageRecorder::addImage(ofShortPixels& image){
	return addImage(image.getPixels());
}

bool ofxDepthImageRecorder::addImage(unsigned short* image){
	//confirm that it isn't a duplicate of the most recent frame;
	int framebytes = 640*480*sizeof(unsigned short);
	if(0 != memcmp(image, lastFramePixs, framebytes)){
		QueuedFrame frame;
        //TODO use high res timer!!!
		frame.timestamp = ofGetElapsedTimeMillis() - recordingStartTime;
		frame.pixels = new unsigned short[640*480];
		memcpy(frame.pixels, image, framebytes);
		memcpy(lastFramePixs, image, framebytes);
		
		char filenumber[512];
		sprintf(filenumber, "%05d", currentFrame); 
		
		char millisstring[512];
		sprintf(millisstring, "%010d", frame.timestamp);
		frame.filename = targetFilePrefix + "_" + filenumber +  "_millis_" + millisstring + ".raw";
		frame.directory = targetDirectory +  "/" + currentFolderPrefix + "/";
				
		recorderThread.lock();
		saveQueue.push( frame );
		recorderThread.unlock();
		
		currentFrame++;
		return true;
	}
	return false;
}

int ofxDepthImageRecorder::numFramesWaitingSave(){
	return saveQueue.size();
}

int ofxDepthImageRecorder::numFramesWaitingCompession(){
	return framesToCompress;
}

int ofxDepthImageRecorder::numDirectoriesWaitingCompression(){
	return encodeDirectories.size();
}

void ofxDepthImageRecorder::toggleRecord(){
	recording = !recording;
	if(recording){
		incrementTake();		
	}
	else {
		compressCurrentTake();
	}
}


bool ofxDepthImageRecorder::isRecording(){
	return recording;
}

void ofxDepthImageRecorder::incrementTake(){
	char takeString[1024] ;
	sprintf(takeString, "TAKE_%02d_%02d_%02d_%02d_%02d", ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
    currentFolderPrefix = string(takeString);
    ofDirectory dir(targetDirectory + "/" + currentFolderPrefix);
    
	if(!dir.exists()){
		dir.create(true);
	}
	
    currentFrame = 0;	
    //TODO: use high res timer!!!
	recordingStartTime = ofGetElapsedTimeMillis();
}


//start converting the current directory
void ofxDepthImageRecorder::compressCurrentTake(){
	if(currentFolderPrefix != ""){
		encoderThread.lock();
        Take t;
        t.path = targetDirectory + "/" + currentFolderPrefix ;
        takes.push_back(t);
		encodeDirectories.push( &takes[takes.size()-1] );
		encoderThread.unlock();
	}	
}

void ofxDepthImageRecorder::updateTakes(){
    takes.clear();
    
    ofDirectory dir = ofDirectory(targetDirectory);
	dir.listDir();
	dir.sort();
    
	//vector<string> paths;
	for(int i = 0; i < dir.numFiles(); i++){
        Take t;
        t.path = dir.getPath(i);
		takes.push_back(t);
	}
    
    
	encoderThread.lock();
	for(int i = 0; i < takes.size(); i++){
		encodeDirectories.push( &takes[i] );
	}
	encoderThread.unlock();		
}

void ofxDepthImageRecorder::shutdown(){
	recorderThread.stopThread(true);
	encoderThread.stopThread(true);
    while(!encoderThread.shutdown || !recorderThread.shutdown){
   		ofSleepMillis(2);
  	}
}
											  
void ofxDepthImageRecorder::recorderThreadCallback(){

	QueuedFrame frame;
	bool foundFrame = false;
	recorderThread.lock();
	if(saveQueue.size() != 0){
		frame = saveQueue.front();
		saveQueue.pop();
		foundFrame = true;
	}
	recorderThread.unlock();
	
	if(foundFrame){
		char filenumber[512];
		sprintf(filenumber, "%05d", currentFrame); 
		if(compressor.saveToRaw(frame.directory+frame.filename, frame.pixels)){
			delete frame.pixels;
		}
		else {
			//if the save fils push it back on tehs tack
			recorderThread.lock();
			saveQueue.push(frame);
			recorderThread.unlock();
			ofLogError("ofxDepthImageRecorder -- Save Failed! readding to queue");
		}
	}
	ofSleepMillis(2);
}

void ofxDepthImageRecorder::encoderThreadCallback(){
    Take* take = NULL;
	bool foundDir = false;

	encoderThread.lock();
	if(encodeDirectories.size() != 0){
		foundDir = true;
		take = encodeDirectories.front();
		encodeDirectories.pop();
	}
	encoderThread.unlock();

	if(foundDir){
        
		//start to convert
		ofDirectory rawDir(take->path);
		rawDir.allowExt("raw");
		rawDir.allowExt("xkcd");
		rawDir.listDir();
		if(encodingBuffer == NULL){
			encodingBuffer = new unsigned short[640*480];
		}
        ofDirectory convertedDir(take->path);
        convertedDir.allowExt("png");
        convertedDir.listDir();
        
		take->numFrames = rawDir.numFiles() + convertedDir.numFiles();
        take->framesConverted = convertedDir.numFiles();
        ofLogVerbose("ofxDepthImageCompressor -- Starting to convert " + ofToString(rawDir.numFiles()) + " in " + take->path);
		framesToCompress = rawDir.numFiles();
		for(int i = 0; i < rawDir.numFiles(); i++){
            
			//don't do this while recording
			while(recording){
//                ofLogWarning("ofxDepthImageRecorder -- paused converting while recording...");
				ofSleepMillis(250);
			}
            
            if(!encoderThread.isThreadRunning()){
                ofLogWarning( "ofxDepthImageRecorder -- Breaking conversion because recorder isn't running");
            	break;
            }
            
			string path = rawDir.getPath(i);
            //READ IN THE RAW FILE
			compressor.readDepthFrame(path, encodingBuffer);
            //COMPRESS TO PNG
			compressor.saveToCompressedPng(ofFilePath::removeExt(path)+".png", encodingBuffer);
            //DELETE the file
			rawDir.getFile(i, ofFile::ReadOnly, true).remove();
            //UPDATE COUNTS
			framesToCompress--;
            take->framesConverted++;
		}
	}
    
	ofSleepMillis(2);
}

