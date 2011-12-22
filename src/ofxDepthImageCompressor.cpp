/*
 *  ofxDepthImageCompressor.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 12/20/11.
 *  Copyright 2011 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageCompressor.h"

ofxDepthImageCompressor::ofxDepthImageCompressor(){
	compressedDepthImage.setUseTexture(false);
}

ofxDepthImageCompressor::~ofxDepthImageCompressor(){

}

void ofxDepthImageCompressor::saveToRaw(string filename, unsigned short* buf){
	ofFile file(filename, ofFile::WriteOnly, true);
	file.write( (char*)&(buf)[0], sizeof(unsigned short)*640*480 );					   
	file.close();
}

void ofxDepthImageCompressor::saveToCompressedPng(string filename, unsigned short* buf){
	if(!compressedDepthImage.isAllocated()){
		compressedDepthImage.allocate(640, 480, OF_IMAGE_COLOR);
	}

	unsigned char* pngPixs = compressedDepthImage.getPixels();
	for(int i = 0; i < 640*480; i++){
		pngPixs[i*3+0] = buf[i] >> 8;
		pngPixs[i*3+1] = buf[i];
		pngPixs[i*3+2] = 0;
	}
	//compressedDepthImage.setFromPixels(pngPixs, 640,480, OF_IMAGE_COLOR);
	compressedDepthImage.update();
	if(ofFilePath::getFileExt(filename) != "png"){
		ofLogError("ofxDepthImageRecorder -- file is not being saved as png: " + filename);
	}
	compressedDepthImage.saveImage(filename);
	
}

unsigned short* ofxDepthImageCompressor::readDepthFrame(string filename, unsigned short* outbuf) {
	ofFile infile(filename, ofFile::ReadOnly, true);
	return readDepthFrame(infile, outbuf);
}

unsigned short* ofxDepthImageCompressor::readDepthFrame(ofFile infile,  unsigned short* outbuf){
    if(outbuf == NULL){
        outbuf = new unsigned short[640*480];
    }
	
	infile.read((char*)(&outbuf[0]), sizeof(unsigned short)*640*480);
	
	infile.close();
	return outbuf;
}


ofImage ofxDepthImageCompressor::readDepthFrametoImage(string filename){	
	
	unsigned short* depthFrame = readDepthFrame(filename);
	ofImage outputImage = convertTo8BitImage(depthFrame);
	
	delete depthFrame;
	return outputImage;
}

ofImage ofxDepthImageCompressor::convertTo8BitImage(unsigned short* buf){
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
	outputImage.update();
	return outputImage;
}

unsigned short* ofxDepthImageCompressor::readCompressedPng(string filename, unsigned short* outbuf){
	if(outbuf == NULL){
		outbuf = new unsigned short[640*480];
	}
	
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
	
	return outbuf;
}

