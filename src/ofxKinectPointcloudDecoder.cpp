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
