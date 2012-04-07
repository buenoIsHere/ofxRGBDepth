/*
 *  DepthHoleFiller.h
 *  ofxKinect
 *
 *  Created by James George 4/6/12
 *  Inspired by Golan's depth hole filler example
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

using namespace cv;
using namespace ofxCv;

class ofxDepthHoleFiller {
	
  public:
	
	ofxDepthHoleFiller();
	void close(ofShortPixels& depthPixels);

	void setIterations(int iterations);
	void setKernelSize(int kernelSize);
	int getIterations();
	int getKernelSize();
	
	bool enable;
  protected:
	int kernelSize;
	int iterations;
	
};
