/*
 *  ofxKinectPointcloudRecorder.h
 *  PointcloudWriter
 *
 *  Created by Jim on 10/20/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#pragma once
#include "ofMain.h"

enum DepthEncodingType{
	DEPTH_ENCODE_RAW,
	DEPTH_ENCODE_PNG
};

typedef struct QueuedFrame {
	unsigned short* pixels;
	string directory;
	string filename;
	int timestamp;
	DepthEncodingType encodingType;
};

class ofxDepthImageRecorder : ofThread {
  public:
	ofxDepthImageRecorder();
	~ofxDepthImageRecorder();
	
	void setDepthEncodingType(DepthEncodingType type);
	
	//ENCODE
	void setup();
	void setRecordLocation(string directory, string filePrefix);
	void addImage(unsigned short* image);
	void incrementFolder();
	void incrementFolder(ofImage posterFrame); //providing a poster frame saves it in the same directory
    
	void saveToCompressedPng(string filename, unsigned short* buf);
	
	int numFramesWaitingSave();
	
	//DECODE
	unsigned short* readDepthFrame(string filename, unsigned short* outbuf = NULL);
	unsigned short* readDepthFrame(ofFile file, unsigned short*  outbuf = NULL);
	unsigned short* readCompressedPng(string filename, unsigned short* outbuf = NULL);
	
	ofImage readDepthFrametoImage(string filename);
	ofImage convertTo8BitImage(unsigned short* buf);
	
	int recordingStartTime; //in millis should make this more accurate
	
  protected:
	DepthEncodingType encodingType;
	ofImage compressedDepthImage;

	unsigned short* lastFramePixs;
	void threadedFunction();
	int folderCount;
    string currentFolderPrefix;
	string targetDirectory;
	string targetFilePrefix;
	int currentFrame;
	//queue<unsigned short*> saveQueue;
	queue<QueuedFrame> saveQueue;
	unsigned char* pngPixs;
	bool isRecording;
};
