//
//  ofxRGBDCaptureGui.h
//  RGBDCaptureOpenNI
//
//  Created by James George on 4/12/12.
//

#pragma once
#include "ofMain.h"
#include "ofxMSAInteractiveObjectDelegate.h"
#include "ofxGameCamera.h"
#include "ofxTimeline.h"
#include "ofxTLDepthImageSequence.h"
#include "ofxDepthImageCompressor.h"
#include "ofxDepthImageProvider.h"
#include "ofxDepthImageRecorder.h"
#include "ofxCvCheckerboardPreview.h"
#include "ofxRGBDAlignment.h"
#include "ofxDepthImageProvider.h"

typedef enum {
	TabCalibrate,
	TabRecord,
	TabPlayback
} RecorderTab;

typedef enum {
	RenderBW,
	RenderRainbow,
	RenderPointCloud
} DepthRenderMode;

typedef struct {
	Take* takeRef;
    ofxMSAInteractiveObjectWithDelegate* button;
    bool isSelected;
} TakeButton;

class ofxRGBDCaptureGui : public ofxMSAInteractiveObjectDelegate {
  public:
	ofxRGBDCaptureGui();
    
    void setup();
    void setImageProvider(ofxDepthImageProvider* imageProvider);
    void update(ofEventArgs& args);
  	void draw(ofEventArgs& args);
    
    void mousePressed(ofMouseEventArgs& args);
    void mouseMoved(ofMouseEventArgs& args);
    void mouseDragged(ofMouseEventArgs& args);
    void mouseReleased(ofMouseEventArgs& args);
    
    void keyPressed(ofKeyEventArgs& args);
    void keyReleased(ofKeyEventArgs& args);
    
    void objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y);
	void objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y);
	void objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button);
	void objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button);
	void objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y);

    void windowResized(ofResizeEventArgs& args);
    
    //void exit(ofEventArgs& args);
    void exit();
    
  protected:
    ofxTimeline timeline;
	ofxTLDepthImageSequence depthSequence;
	ofxRGBDAlignment alignment;
	ofxDepthImageRecorder recorder;
	ofxCvCheckerboardPreview calibrationPreview;

	void loadDirectory();
	void loadDirectory(string path);
	void loadDefaultDirectory();
    
	void loadSequenceForPlayback( int index );
	void updateTakeButtons();
	
	void toggleRecord();
	void captureCalibrationImage();
    
	void drawPointcloud(ofShortPixels& pix, bool fullscreen);
	
	string workingDirectory;
	
	bool cameraFound;
	bool fullscreenPoints;
	
	ofColor downColor;
	ofColor idleColor;
	ofColor hoverColor;
	
	float framewidth;
	float frameheight;
	float thirdWidth;
	float btnheight;
	float takeWidth;
	
    vector<ofxMSAInteractiveObjectWithDelegate*> buttonSet; //all non take buttons
    
	ofxMSAInteractiveObjectWithDelegate* btnSetDirectory;
	
	ofxMSAInteractiveObjectWithDelegate* btnCalibrateTab;
	ofxMSAInteractiveObjectWithDelegate* btnRecordTab;
	ofxMSAInteractiveObjectWithDelegate* btnPlaybackTab;
	ofxMSAInteractiveObjectWithDelegate* currentTabObject;
    ofxMSAInteractiveObjectWithDelegate* currentRenderModeObject;
    
	ofxMSAInteractiveObjectWithDelegate* btnRecordBtn;
	
	ofxMSAInteractiveObjectWithDelegate* btnRenderBW;
	ofxMSAInteractiveObjectWithDelegate* btnRenderRainbow;
	ofxMSAInteractiveObjectWithDelegate* btnRenderPointCloud;
    
	vector<TakeButton> btnTakes;
	
	ofxGameCamera cam;
	
    ofRectangle previewRect;
    
	RecorderTab currentTab;	
	DepthRenderMode currentRenderMode;
	
	ofPtr<ofxDepthImageProvider> depthImageProvider;
	
    bool providerSet;
	ofImage calibrationImage;
	//unsigned short* frame;

  protected:
    ofImage currentDepthImage;
    void createRainbowPallet();
	unsigned char LUTR[256];
	unsigned char LUTG[256];
	unsigned char LUTB[256];

	void updateDepthImage(ofShortPixels& pixels);
    ofImage depthImage;
};
