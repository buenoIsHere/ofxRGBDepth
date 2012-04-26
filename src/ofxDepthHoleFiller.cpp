/*
 *  ofxDepthHoleFiller.cpp
 *
 */

#include "ofxDepthHoleFiller.h"

ofxDepthHoleFiller::ofxDepthHoleFiller(){
	enable = true;
	kernelSize = 1;
	iterations = 1;
}

void ofxDepthHoleFiller::setIterations(int newIterations){
	iterations = ofClamp(newIterations, 1, 20);
}

void ofxDepthHoleFiller::setKernelSize(int newKernelSize){
	kernelSize = ofClamp(newKernelSize, 1, 20);	
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
        Mat original = toCv(depthPixels);
		Mat filledMask;
        Mat dilated;
        
		Mat m_element_m = getStructuringElement(MORPH_RECT, cv::Size(kernelSize, kernelSize));
        morphologyEx(original, dilated, MORPH_CLOSE, m_element_m, cv::Point(-1,-1), iterations);
        
        cv::compare(original, 0, filledMask, CMP_EQ);
        cv::add(original, dilated, original, filledMask);        
	}	
}

