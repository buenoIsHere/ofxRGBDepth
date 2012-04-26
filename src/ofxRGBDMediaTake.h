//
//  ofxRGBDMediaTake.h
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#pragma once
#include "ofMain.h"
#include "ofxRGBDRenderSettings.h"

class ofxRGBDMediaTake {
  public:
    ofxRGBDMediaTake();
    
    bool loadFromFolder(string sourceMediaFolder);
    bool valid();
    
    
    //void populateCompositions();
	void populateRenderSettings();
    vector<ofxRGBDRenderSettings>& getRenderSettings();
    
    string mediaFolder;
    string calibrationDirectory;
    string lowResVideoPath;
    string hiResVideoPath;
    string depthFolder;
    string pairingsFile;
    
  protected:

    bool hasCalibrationDirectory;
    bool hasDepthFolder;
    bool hasLargeVideoFile;
    bool hasSmallVideoFile;
    
    //TODO convert to full compositions!
    vector<ofxRGBDRenderSettings> renderSettings;
    
    
};


