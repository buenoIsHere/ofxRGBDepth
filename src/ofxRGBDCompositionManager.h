//
//  ofxRGBDCompositionManager
//  RGBDVisualize
//
//  Created by James George on 4/16/12.
//

#pragma once

#include "ofMain.h"
#include "ofxRGBDMediaTake.h"

class ofxRGBDCompositionManager {
  public:
    ofxRGBDCompositionManager();    
    void setup();
    void populateFromMediaBin(string mediaBin);
    vector<ofxRGBDMediaTake> takes; 
};

