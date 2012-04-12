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
#include "ofxDepthImageCompressor.h"

typedef struct {
	unsigned short* pixels;
	string directory;
	string filename;
	int timestamp;
} QueuedFrame;

//thread classes for callbacks
class ofxDepthImageRecorder;
class ofxRGBDRecorderThread : public ofThread {
public:
    bool shutdown;
	ofxDepthImageRecorder* delegate;
	ofxRGBDRecorderThread(ofxDepthImageRecorder* d) : delegate(d){}	
	void threadedFunction();
};

class ofxRGBDEncoderThread : public ofThread {
public:
    bool shutdown;
	ofxDepthImageRecorder* delegate;
	ofxRGBDEncoderThread(ofxDepthImageRecorder* d) : delegate(d){}
	void threadedFunction();	
};

class ofxDepthImageRecorder {
  public:
	ofxDepthImageRecorder();
	~ofxDepthImageRecorder();

	vector<string> getTakePaths();
	
	void setup();
	void toggleRecord();
	bool isRecording();
	
	void setRecordLocation(string directory, string filePrefix);
	bool addImage(ofShortPixels& image);
	bool addImage(unsigned short* image);

	int numFramesWaitingSave();
	int numFramesWaitingCompession();
	int numDirectoriesWaitingCompression();
	
	void shutdown();
	
	int recordingStartTime; //in millis -- potentially should make this more accurate
	
	void encoderThreadCallback();
	void recorderThreadCallback();
	
	
  protected:
	bool recording;
	bool encoderShutdown;
    bool recorderShutdown;
	void incrementTake();
    //start converting the current directory
	void compressCurrentTake();

	
	ofxDepthImageCompressor compressor;
	ofxRGBDRecorderThread recorderThread;
	ofxRGBDEncoderThread encoderThread;
	
	ofImage compressedDepthImage;

	int framesToCompress;
	
	unsigned short* encodingBuffer;
	unsigned short* lastFramePixs;

	
	
	int folderCount;
    string currentFolderPrefix;
	string targetDirectory;
	string targetFilePrefix;
	int currentFrame;
	
	queue<QueuedFrame> saveQueue;
	queue<string> encodeDirectories;
};
