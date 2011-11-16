/*
 *  ofxKinectPointcloudDecoder.cpp
 *  RGBDPost
 *
 *  Created by James George on 11/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ofxKinectPointcloudDecoder.h"

unsigned short* ofxKinectPointcloudDecoder::readDepthFrame(string filename, unsigned short* outbuf) {
	int amnt;
	ofFile infile(filename, ofFile::ReadOnly, true);
	return readDepthFrame(infile, outbuf);
}

unsigned short* ofxKinectPointcloudDecoder::readDepthFrame(ofFile infile,  unsigned short* outbuf){
    if(outbuf == NULL){
        outbuf = new unsigned short[640*480];
    }
	infile.read((char*)(&outbuf[0]), sizeof(unsigned short)*640*480);
    int lastlinesum = 0;
    for(int i = 640*480-640*10; i < 640*480; i++){
        lastlinesum += outbuf[i];
		//        cout << i << " " << outbuf[i] << endl;
    }
	//    cout << "last lines sum " << lastlinesum << endl;
    
	infile.close();
	return outbuf;
}

ofImage ofxKinectPointcloudDecoder::readDepthFrametoImage(string filename){
	//straight copied out of ofxkinect
	
	cout << "reading depth with file " << filename << endl;
	unsigned short* depthFrame = readDepthFrame(filename);
	ofImage outputImage = convertTo8BitImage(depthFrame);
	
	delete depthFrame;
	return outputImage;
}

ofImage ofxKinectPointcloudDecoder::convertTo8BitImage(unsigned short* buf){
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
//	outputImage.update();
	outputImage.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
	return outputImage;
}