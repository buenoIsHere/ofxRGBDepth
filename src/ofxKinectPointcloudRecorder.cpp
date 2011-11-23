/*
 *  ofxKinectPointcloudRecorder.cpp
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxKinectPointcloudRecorder.h"

ofxKinectPointcloudRecorder::ofxKinectPointcloudRecorder(){
	pngPixs = NULL;
	lastFramePixs = NULL;
	encodingType = DEPTH_ENCODE_PNG;
}

ofxKinectPointcloudRecorder::~ofxKinectPointcloudRecorder(){
	stopThread(true);
	if(pngPixs != NULL){
		delete pngPixs;
	}
	
	if(lastFramePixs != NULL){
		delete lastFramePixs;
	}
}

void ofxKinectPointcloudRecorder::setup(){
    folderCount = 0;
	currentFrame = 0;
	
	lastFramePixs = new unsigned short[640*480];
	memset(lastFramePixs, 0, sizeof(unsigned short)*640*480);

    startThread(true, false);
}

void ofxKinectPointcloudRecorder::setRecordLocation(string directory, string filePrefix){
	targetDirectory = directory;
	ofDirectory dir(directory);
	if(!dir.exists()){
		dir.create(true);
	}
	
	targetFilePrefix = filePrefix;
}

void ofxKinectPointcloudRecorder::setDepthEncodingType(DepthEncodingType type){
	encodingType = type;
}


void ofxKinectPointcloudRecorder::addImage(unsigned short* image){
	//confirm that it isn't a duplicate of the most recent frame;
	int framebytes = 640*480*sizeof(unsigned short);
	if(0 != memcmp(image, lastFramePixs, framebytes)){
		unsigned short* addToQueue = new unsigned short[640*480];
		memcpy(addToQueue, image, framebytes);
		memcpy(lastFramePixs, image, framebytes);
		
		lock();
		saveQueue.push( addToQueue );
		unlock();
	}
}

void ofxKinectPointcloudRecorder::incrementFolder(ofImage posterFrame){
    currentFolderPrefix = "TAKE_" + ofToString(ofGetDay()) + "_" + ofToString(ofGetHours()) + "_" + ofToString(ofGetMinutes()) + "_" + ofToString(ofGetSeconds());
    ofDirectory dir(targetDirectory + "/" + currentFolderPrefix);
    
	if(!dir.exists()){
		dir.create(true);
	}
    try{
        posterFrame.saveImage(targetDirectory+"/"+currentFolderPrefix+"/_poster.png");
    }catch(...){
        
    }
    currentFrame = 0;
}

void ofxKinectPointcloudRecorder::threadedFunction(){

	while(isThreadRunning()){
		unsigned short* tosave = NULL;
		lock();
		if(saveQueue.size() != 0){
			tosave = saveQueue.front();
			saveQueue.pop();
			cout << " currently " << saveQueue.size() << " waiting " << endl;
		}
		unlock();
		
		if(tosave != NULL){
			char filenumber[512];
            sprintf(filenumber, "%05d", currentFrame); 
            if(encodingType == DEPTH_ENCODE_RAW){
				string filename = targetDirectory +  "/" + currentFolderPrefix + "/" + targetFilePrefix + "_" + filenumber +  ".xkcd";
				ofFile file(filename, ofFile::WriteOnly, true);
				file.write( (char*)&tosave[0], sizeof(unsigned short)*640*480 );					   
				file.close();
			}
			else if(encodingType == DEPTH_ENCODE_PNG){
				string filename = targetDirectory +  "/" + currentFolderPrefix + "/" + targetFilePrefix + "_" + filenumber +  ".png";
				saveToCompressedPng(filename, tosave);
			}
						
			currentFrame++;
			delete tosave;			
		}
	}
}

void ofxKinectPointcloudRecorder::saveToCompressedPng(string filename, unsigned short* buf){
	if(pngPixs == NULL){
		pngPixs = new unsigned char[640*480*3];	
	}
	
	for(int i = 0; i < 640*480; i++){
		pngPixs[i*3+0] = buf[i] >> 8;
		pngPixs[i*3+1] = buf[i];
		pngPixs[i*3+2] = 0;
	}
	ofImage compressedDepthImage;
	compressedDepthImage.setFromPixels(pngPixs, 640,480, OF_IMAGE_COLOR);
	if(ofFilePath::getFileExt(filename) != "png"){
		ofLogError("ofxKinectPointcloudRecorder -- file is not being saved as png: " + filename);
	}
	compressedDepthImage.saveImage(filename);
	
}


unsigned short* ofxKinectPointcloudRecorder::readDepthFrame(string filename, unsigned short* outbuf) {
	int amnt;
	ofFile infile(filename, ofFile::ReadOnly, true);
	return readDepthFrame(infile, outbuf);
}

unsigned short* ofxKinectPointcloudRecorder::readDepthFrame(ofFile infile,  unsigned short* outbuf){
    if(outbuf == NULL){
        outbuf = new unsigned short[640*480];
    }
	
	infile.read((char*)(&outbuf[0]), sizeof(unsigned short)*640*480);
	
	infile.close();
	return outbuf;
}

ofImage ofxKinectPointcloudRecorder::readDepthFrametoImage(string filename){	

	unsigned short* depthFrame = readDepthFrame(filename);
	ofImage outputImage = convertTo8BitImage(depthFrame);
	
	delete depthFrame;
	return outputImage;
}

ofImage ofxKinectPointcloudRecorder::convertTo8BitImage(unsigned short* buf){
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


unsigned short* ofxKinectPointcloudRecorder::readCompressedPng(string filename, unsigned short* outbuf){
	if(outbuf == NULL){
		outbuf = new unsigned short[640*480];
	}
	
//	float startTime = ofGetElapsedTimeMillis();
	
	int totalDif = 0;
	ofImage compressedImage;
	if(!compressedImage.loadImage(filename)){
		ofLogError("ofxKinectPointcloudRecorder -- Couldn't read compressed frame " + filename);
		return outbuf;
	}
	
	unsigned char* compressedPix = compressedImage.getPixels();
	
	for(int i = 0; i < 640*480; i++){
		outbuf[i] = (compressedPix[i*3] << 8) | compressedPix[i*3+1];
	}
	
//	cout << "decompressed in " << (ofGetElapsedTimeMillis() - startTime) << endl;
	
	return outbuf;
}

