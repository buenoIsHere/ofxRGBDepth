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
	while(isThreadRunning()){
		delegate->encoderThreadCallback();
	}
}

void ofxRGBDRecorderThread::threadedFunction(){
	while(isThreadRunning()){
		delegate->recorderThreadCallback();
	}
}

#pragma mark Thread Implementation
ofxDepthImageRecorder::ofxDepthImageRecorder()
  : recorderThread(this),
	encoderThread(this)
{
	lastFramePixs = NULL;
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
	targetDirectory = directory;
	ofDirectory dir(directory);
	if(!dir.exists()){
		dir.create(true);
	}
	
	targetFilePrefix = filePrefix;
}


vector<string> ofxDepthImageRecorder::getTakePaths(){
	ofDirectory dir = ofDirectory(targetDirectory);
	dir.listDir();
	dir.sort();
	vector<string> paths;
	for(int i = 0; i < dir.numFiles(); i++){
		paths.push_back(dir.getPath(i));
	}
	return paths;
}

bool ofxDepthImageRecorder::addImage(unsigned short* image){
	//confirm that it isn't a duplicate of the most recent frame;
	int framebytes = 640*480*sizeof(unsigned short);
	if(0 != memcmp(image, lastFramePixs, framebytes)){
		QueuedFrame frame;
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

//start converting the current directory
void ofxDepthImageRecorder::compressCurrentTake(){
	if(currentFolderPrefix != ""){
		encoderThread.lock();
		encodeDirectories.push( targetDirectory + "/" + currentFolderPrefix );
		encoderThread.unlock();
	}	
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
	recordingStartTime = ofGetElapsedTimeMillis();
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
		compressor.saveToRaw(frame.directory+frame.filename, frame.pixels);
		delete frame.pixels;
	}
	
//            if(frame.encodingType == DEPTH_ENCODE_RAW){
				
				//string filename = targetDirectory +  "/" + currentFolderPrefix + "/" + targetFilePrefix + "_" + filenumber +  ".xkcd";
//				ofFile file(frame.directory + frame.filename, ofFile::WriteOnly, true);
//				file.write( (char*)&((unsigned short*)frame.pixels)[0], sizeof(unsigned short)*640*480 );					   
//				file.close();
				

				
//			}
//			else if(encodingType == DEPTH_ENCODE_PNG){
////				int startTime = ofGetElapsedTimeMillis();
//				saveToCompressedPng(frame.directory+frame.filename, (unsigned short*)frame.pixels);
//				delete (unsigned short*)frame.pixels;
////				cout << "compression took " << ofGetElapsedTimeMillis() - startTime << " Millis " << endl;
//			}


}

void ofxDepthImageRecorder::encoderThreadCallback(){
	string dir;
	bool foundDir = false;

	encoderThread.lock();
	if(encodeDirectories.size() != 0){
		foundDir = true;
		dir = encodeDirectories.front();
		encodeDirectories.pop();
	}
	encoderThread.unlock();

	if(foundDir){
		//start to convert
		ofDirectory rawDir(dir);
		rawDir.allowExt("raw");
		rawDir.listDir();
		if(encodingBuffer == NULL){
			encodingBuffer = new unsigned short[640*480];
		}
		cout << "ofxDepthImageCompressor -- Starting to convert " << rawDir.numFiles() << " in " << dir << endl;
		for(int i = 0; i < rawDir.numFiles(); i++){
			string path = rawDir.getPath(i);
			compressor.readDepthFrame(path, encodingBuffer);
			compressor.saveToCompressedPng(ofFilePath::removeExt(path)+".png", encodingBuffer);
			rawDir.getFile(i, ofFile::ReadOnly, true).remove();
		}
	}
	ofSleepMillis(2);
}

//void ofxDepthImageRecorder::saveToCompressedPng(string filename, unsigned short* buf){
//	if(pngPixs == NULL){
//		pngPixs = new unsigned char[640*480*3];	
//	}
//	
//	for(int i = 0; i < 640*480; i++){
//		pngPixs[i*3+0] = buf[i] >> 8;
//		pngPixs[i*3+1] = buf[i];
//		pngPixs[i*3+2] = 0;
//	}
//	compressedDepthImage.setUseTexture(false);
//	compressedDepthImage.setFromPixels(pngPixs, 640,480, OF_IMAGE_COLOR);
//	if(ofFilePath::getFileExt(filename) != "png"){
//		ofLogError("ofxDepthImageRecorder -- file is not being saved as png: " + filename);
//	}
//	compressedDepthImage.saveImage(filename);
//	
//}


