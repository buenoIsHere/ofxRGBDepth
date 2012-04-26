//
//  ofxRGBDRenderSettings.h
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#pragma once

#include "ofMain.h"
#include "ofxRGBDRenderer.h"

class ofxRGBDRenderSettings {
  public:  
    ofxRGBDRenderSettings();
    
    void applyToRenderer(ofxRGBDRenderer& renderer);
    void drawWithRenderer(ofxRGBDRenderer& renderer);
    
    void saveToXml(string xmlFile);
    void loadFromXml(string xmlFile);
    
    int edgeClip;
    int zThreshold;
    
    bool mirror;
    ofVec2f offset;
    int simplify;
	int startFrame;
    int endFrame;
    bool drawPointcloud;
    bool drawWireframe;
    bool drawBackdrop;
    
    bool fillHoles;
    int kernelSize;
    int iterations;
    
    float wireFrameSize;
    float pointSize;
    
    ofNode cameraPosition;
};