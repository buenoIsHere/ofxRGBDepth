/*
 *  ofxDepthHoleFiller.cpp
 *
 *
 */

#include "ofxDepthHoleFiller.h"

ofxDepthHoleFiller::ofxDepthHoleFiller(){
	enable = true;
	kernelSize = 1;
	iterations = 1;
}

void ofxDepthHoleFiller::setIterations(int newIterations){
	iterations = ofClamp(newIterations, 1, 10);
}

void ofxDepthHoleFiller::setKernelSize(int newKernelSize){
	kernelSize = ofClamp(newKernelSize, 1, 10);	
	if(kernelSize % 2 == 0){
		kernelSize++;
	}
}

int ofxDepthHoleFiller::getIterations(){
	return iterations;
}

int ofxDepthHoleFiller::getKernelSize(){
	return kernelSize;
}

void ofxDepthHoleFiller::close(ofShortPixels& depthPixels){
	if(enable){
		for(int i = 0; i < iterations; i++){
			Mat pix = toCv(depthPixels);
			Mat m_element_m = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize));
			morphologyEx(pix, pix, MORPH_CLOSE, m_element_m);
		}
	}	
}