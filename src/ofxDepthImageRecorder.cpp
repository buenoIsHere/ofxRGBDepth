/*
 *  ofxDepthImageRecorder.cpp
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxDepthImageRecorder.h"

ofxDepthImageRecorder::ofxDepthImageRecorder(){
	pngPixs = NULL;
	lastFramePixs = NULL;
	encodingType = DEPTH_ENCODE_PNG;
}

ofxDepthImageRecorder::~ofxDepthImageRecorder(){
	stopThread(true);
	if(pngPixs != NULL){
		delete pngPixs;
	}
	
	if(lastFramePixs != NULL){
		delete lastFramePixs;
	}
}

void ofxDepthImageRecorder::setup(){
    folderCount = 0;
	currentFrame = 0;
	
	lastFramePixs = new unsigned short[640*480];
	memset(lastFramePixs, 0, sizeof(unsigned short)*640*480);

    startThread(true, false);
}

void ofxDepthImageRecorder::setRecordLocation(string directory, string filePrefix){
	targetDirectory = directory;
	ofDirectory dir(directory);
	if(!dir.exists()){
		dir.create(true);
	}
	
	targetFilePrefix = filePrefix;
}

void ofxDepthImageRecorder::setDepthEncodingType(DepthEncodingType type){
	encodingType = type;
}


void ofxDepthImageRecorder::addImage(unsigned short* image){
	//confirm that it isn't a duplicate of the most recent frame;
	int framebytes = 640*480*sizeof(unsigned short);
	if(0 != memcmp(image, lastFramePixs, framebytes)){
		QueuedFrame frame;
		frame.timestamp = ofGetElapsedTimeMillis() - recordingStartTime;
		frame.directory = targetDirectory +  "/" + currentFolderPrefix + "/";
		unsigned short* addToQueue = new unsigned short[640*480];
		memcpy(addToQueue, image, framebytes);
		memcpy(lastFramePixs, image, framebytes);
		frame.pixels = addToQueue;
		frame.encodingType = encodingType;
		
		char filenumber[512];
		sprintf(filenumber, "%05d", currentFrame); 

		if(encodingType == DEPTH_ENCODE_RAW){
			frame.filename = targetFilePrefix + "_" + filenumber +  "_millis_" + ofToString(frame.timestamp) + ".xkcd";
		}
		else if(encodingType == DEPTH_ENCODE_PNG){
			frame.filename = targetFilePrefix + "_" + filenumber +  "_millis_" + ofToString(frame.timestamp) + ".png";
		}
				
		lock();
		saveQueue.push( frame );
		unlock();
		currentFrame++;
	}
}

int ofxDepthImageRecorder::numFramesWaitingSave(){
	return saveQueue.size();
}

void ofxDepthImageRecorder::incrementFolder(){
    currentFolderPrefix = "TAKE_" + ofToString(ofGetDay()) + "_" + ofToString(ofGetHours()) + "_" + ofToString(ofGetMinutes()) + "_" + ofToString(ofGetSeconds());
    ofDirectory dir(targetDirectory + "/" + currentFolderPrefix);
    
	if(!dir.exists()){
		dir.create(true);
	}
    currentFrame = 0;	
	recordingStartTime = ofGetElapsedTimeMillis();
}
											  
void ofxDepthImageRecorder::incrementFolder(ofImage posterFrame){
	incrementFolder();
	posterFrame.saveImage(targetDirectory+"/"+currentFolderPrefix+"/_poster.png");
}

void ofxDepthImageRecorder::threadedFunction(){

	while(isThreadRunning()){
		//unsigned short* tosave = NULL;
		QueuedFrame frame;
		bool foundFrame = false;
		lock();
		if(saveQueue.size() != 0){
			frame = saveQueue.front();
			saveQueue.pop();
			foundFrame = true;
		}
		unlock();
		
		if(foundFrame){
            if(frame.encodingType == DEPTH_ENCODE_RAW){
				//string filename = targetDirectory +  "/" + currentFolderPrefix + "/" + targetFilePrefix + "_" + filenumber +  ".xkcd";
				ofFile file(frame.directory + frame.filename, ofFile::WriteOnly, true);
				file.write( (char*)&frame.pixels[0], sizeof(unsigned short)*640*480 );					   
				file.close();
			}
			else if(encodingType == DEPTH_ENCODE_PNG){
				saveToCompressedPng(frame.directory+frame.filename, frame.pixels);
			}
						

			delete frame.pixels;			
		}
	}
}

void ofxDepthImageRecorder::saveToCompressedPng(string filename, unsigned short* buf){
	if(pngPixs == NULL){
		pngPixs = new unsigned char[640*480*3];	
	}
	
	for(int i = 0; i < 640*480; i++){
		pngPixs[i*3+0] = buf[i] >> 8;
		pngPixs[i*3+1] = buf[i];
		pngPixs[i*3+2] = 0;
	}
	compressedDepthImage.setUseTexture(false);
	compressedDepthImage.setFromPixels(pngPixs, 640,480, OF_IMAGE_COLOR);
	if(ofFilePath::getFileExt(filename) != "png"){
		ofLogError("ofxDepthImageRecorder -- file is not being saved as png: " + filename);
	}
	compressedDepthImage.saveImage(filename);
	
}


unsigned short* ofxDepthImageRecorder::readDepthFrame(string filename, unsigned short* outbuf) {
	int amnt;
	ofFile infile(filename, ofFile::ReadOnly, true);
	return readDepthFrame(infile, outbuf);
}

unsigned short* ofxDepthImageRecorder::readDepthFrame(ofFile infile,  unsigned short* outbuf){
    if(outbuf == NULL){
        outbuf = new unsigned short[640*480];
    }
	
	infile.read((char*)(&outbuf[0]), sizeof(unsigned short)*640*480);
	
	infile.close();
	return outbuf;
}

ofImage ofxDepthImageRecorder::readDepthFrametoImage(string filename){	

	unsigned short* depthFrame = readDepthFrame(filename);
	ofImage outputImage = convertTo8BitImage(depthFrame);
	
	delete depthFrame;
	return outputImage;
}

ofImage ofxDepthImageRecorder::convertTo8BitImage(unsigned short* buf){
	int nearPlane = 500;
	int farPlane = 4000;
	ofImage outputImage;
	outputImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	unsigned char* pix = outputImage.getPixels();
	for(int i = 0; i < 640*480; i++){
		if(buf[i] == 0){
			pix[i] = 0;
		}
		else {
			pix[i] = ofMap(buf[i], nearPlane, farPlane, 255, 0, true);
		}
	}

	outputImage.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
	return outputImage;
}


unsigned short* ofxDepthImageRecorder::readCompressedPng(string filename, unsigned short* outbuf){
	if(outbuf == NULL){
		outbuf = new unsigned short[640*480];
	}
	
//	float startTime = ofGetElapsedTimeMillis();
	
	int totalDif = 0;
	ofImage compressedImage;
	if(!compressedImage.loadImage(filename)){
		ofLogError("ofxDepthImageRecorder -- Couldn't read compressed frame " + filename);
		return outbuf;
	}
	
	unsigned char* compressedPix = compressedImage.getPixels();
	
	for(int i = 0; i < 640*480; i++){
		outbuf[i] = (compressedPix[i*3] << 8) | compressedPix[i*3+1];
	}
	
//	cout << "decompressed in " << (ofGetElapsedTimeMillis() - startTime) << endl;
	
	return outbuf;
}

