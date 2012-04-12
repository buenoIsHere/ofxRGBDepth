//
//  ofxRGBDCaptureGui.cpp
//  RGBDCaptureOpenNI
//
//  Created by James George on 4/12/12.
//

#include "ofxRGBDCaptureGui.h"

ofxRGBDCaptureGui::ofxRGBDCaptureGui(){
    providerSet = false;
    fullscreenPoints = false;
}

void ofxRGBDCaptureGui::setup(){
    
    
	currentTab = TabCalibrate;
    
	downColor  = ofColor(255, 120, 0);
	idleColor  = ofColor(220, 200, 200);
	hoverColor = ofColor(255*.2, 255*.2, 30*.2);
	
	//setup buttons
	framewidth = 640;
	frameheight = 480;
	thirdWidth = framewidth/3;
	btnheight = 30;
	
	btnSetDirectory = new ofxMSAInteractiveObjectWithDelegate();
	btnSetDirectory->setPosAndSize(0, 0, framewidth, btnheight);
	btnSetDirectory->setLabel("Load Directory");
	btnSetDirectory->setIdleColor(idleColor);
	btnSetDirectory->setDownColor(downColor);
	btnSetDirectory->setHoverColor(hoverColor);
	btnSetDirectory->setDelegate(this);
	
	btnCalibrateTab = new ofxMSAInteractiveObjectWithDelegate();
	btnCalibrateTab->setPosAndSize(0, btnheight, thirdWidth, btnheight);
	btnCalibrateTab->setLabel("Calibrate");
	btnCalibrateTab->setIdleColor(idleColor);
	btnCalibrateTab->setDownColor(downColor);
	btnCalibrateTab->setHoverColor(hoverColor);
	btnCalibrateTab->setDelegate(this);
	
	btnRecordTab = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordTab->setPosAndSize(thirdWidth, btnheight, thirdWidth, btnheight);
	btnRecordTab->setLabel("Record");
	btnRecordTab->setIdleColor(idleColor);
	btnRecordTab->setDownColor(downColor);
	btnRecordTab->setHoverColor(hoverColor);
	btnRecordTab->setDelegate(this);
	
	btnPlaybackTab = new ofxMSAInteractiveObjectWithDelegate();
	btnPlaybackTab->setPosAndSize(thirdWidth*2, btnheight, thirdWidth, btnheight);
	btnPlaybackTab->setLabel("Playback");
	btnPlaybackTab->setIdleColor(idleColor);
	btnPlaybackTab->setDownColor(downColor);
	btnPlaybackTab->setHoverColor(hoverColor);
	btnPlaybackTab->setDelegate(this);
	
	btnRenderBW = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderBW->setPosAndSize(0, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderBW->setLabel("Blaick&White");
	btnRenderBW->setIdleColor(idleColor);
	btnRenderBW->setDownColor(downColor);
	btnRenderBW->setHoverColor(hoverColor);
	btnRenderBW->setDelegate(this);
	
	btnRenderRainbow = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderRainbow->setPosAndSize(thirdWidth, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderRainbow->setLabel("Rainbow");
	btnRenderRainbow->setIdleColor(idleColor);
	btnRenderRainbow->setDownColor(downColor);
	btnRenderRainbow->setHoverColor(hoverColor);
	btnRenderRainbow->setDelegate(this);
	
	btnRenderPointCloud = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderPointCloud->setPosAndSize(thirdWidth*2, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderPointCloud->setLabel("Pointcloud");
	btnRenderPointCloud->setIdleColor(idleColor);
	btnRenderPointCloud->setDownColor(downColor);
	btnRenderPointCloud->setHoverColor(hoverColor);
	btnRenderPointCloud->setDelegate(this);
	
	btnRecordBtn = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordBtn->setPosAndSize(0, btnheight*3+frameheight, framewidth, btnheight);
	btnRecordBtn->setLabel("Record");
	btnRecordBtn->setIdleColor(idleColor);
	btnRecordBtn->setDownColor(downColor);
	btnRecordBtn->setHoverColor(hoverColor);
	btnRecordBtn->setDelegate(this);
    
	calibrationPreview.setup(10, 7, 2.5);
	alignment.setup(10, 7, 2.5);
	alignment.setupGui(0, btnheight*4+frameheight, ofGetWidth());
	
	timeline.setup();
	timeline.setOffset(ofVec2f(0,btnRecordBtn->y+btnRecordBtn->height));
	timeline.addElement("depth sequence", &depthSequence);
	timeline.setWidth(ofGetWidth());
	timeline.setLoopType(OF_LOOP_NORMAL);
	
	depthSequence.setup();
	
	ofxXmlSettings defaults;
	if(defaults.loadFile("defaults.xml")){
		loadDirectory(defaults.getValue("currentDir", ""));
	}
	else{
		loadDirectory("depthframes");
	}
	
	updateTakeButtons();
	
	cam.loadCameraPosition();
	
	cam.speed = 25;
	cam.setFarClip(50000);
    
	recorder.setup();
    
    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);
    
    ofAddListener(ofEvents.windowResized, this, &ofxRGBDCaptureGui::windowResized);
   // ofAddListener(ofEvents.exit, this, &ofxRGBDCaptureGui::exit);
    ofAddListener(ofEvents.update, this, &ofxRGBDCaptureGui::update);
    ofAddListener(ofEvents.draw, this, &ofxRGBDCaptureGui::draw);
}

void ofxRGBDCaptureGui::setImageProvider(ofxDepthImageProvider* imageProvider){
	depthImageProvider = ofPtr<ofxDepthImageProvider>( imageProvider );
    depthImageProvider->setup();
    providerSet = true;
}

void ofxRGBDCaptureGui::update(ofEventArgs& args){
    
	if(!providerSet || !depthImageProvider->deviceFound()){
		return;
	}
	
	//JG conv -- recordContext.update();
	depthImageProvider->update();
	if(depthImageProvider->isFrameNew()){
		
		if(currentTab == TabCalibrate){
			calibrationPreview.setTestImage( depthImageProvider->getRawIRImage() );
		}
		
		if(recorder.isRecording()){
			//JG conv -- recorder.addImage( (unsigned short*)recordDepth.getRawDepthPixels());
			recorder.addImage(depthImageProvider->getRawDepth());
		}
	}
}

void ofxRGBDCaptureGui::draw(ofEventArgs& args){
    
	if(fullscreenPoints && currentTab == TabPlayback){
		drawPointcloud(true);
		return;
	}
    
	if(currentTab == TabCalibrate){
		//recordImage.draw(0, btnheight*2, 640, 480);
		depthImageProvider->getRawIRImage().draw(0, btnheight*2, 640, 480);
		calibrationPreview.draw(0, btnheight*2);
		alignment.drawDepthImages();
	}
	else if(currentTab == TabRecord){
		//TODO render modes			
		ofPushStyle();
		ofSetColor(255, 255, 255, 60);
		ofLine(320, btnheight*2, 320, btnheight*2+480);
		ofLine(0, btnheight*2+240, 640, btnheight*2+240);
		ofPopStyle();
		depthImageProvider->getDepthImage().draw(0, btnheight*2, 640, 480);
	}
	else {
		if(currentRenderMode == RenderBW || currentRenderMode == RenderRainbow){
			depthSequence.currentDepthImage.draw(0, btnheight*2, 640, 480);
		}
		else {
			if(depthSequence.currentDepthRaw != NULL){
				drawPointcloud(false);
			}
		}
        
		//draw timeline
		timeline.draw();
	}
    
	//draw save meter if buffer is getting full
	if(recorder.isRecording()){
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofNoFill();
		ofSetLineWidth(5);
		
		ofRect(0, btnheight*2, 640, 480);
		ofPopStyle();
	}
	
	if(recorder.numFramesWaitingSave() > 0){
		ofPushStyle();
		float width = recorder.numFramesWaitingSave()/2000.0 * btnRecordBtn->width;
		ofFill();
		ofSetColor(255,0, 0);
		ofRect(btnRecordBtn->x,btnRecordBtn->y,width,btnRecordBtn->height);
		
		if(ofGetFrameNum() % 30 < 15){
			ofSetColor(255, 0, 0, 40);
			ofRect(*btnRecordBtn);
		}
		ofPopStyle();
	}
}

void ofxRGBDCaptureGui::objectDidRollOver(ofxMSAInteractiveObject* object, int x, int y){}
void ofxRGBDCaptureGui::objectDidRollOut(ofxMSAInteractiveObject* object, int x, int y){}
void ofxRGBDCaptureGui::objectDidPress(ofxMSAInteractiveObject* object, int x, int y, int button){}
void ofxRGBDCaptureGui::objectDidMouseMove(ofxMSAInteractiveObject* object, int x, int y){}
void ofxRGBDCaptureGui::objectDidRelease(ofxMSAInteractiveObject* object, int x, int y, int button){
 	if(object == btnSetDirectory){
		loadDirectory();
	}
	else if(object == btnCalibrateTab){
		currentTab = TabCalibrate; 
	}
	else if(object == btnRecordTab){
		currentTab = TabRecord;
	}
	else if(object == btnPlaybackTab){
		currentTab = TabPlayback;
	}
	else if(object == btnRecordBtn){
		if(currentTab == TabRecord){
			toggleRecord();
		}
		else if(currentTab == TabCalibrate){
			captureCalibrationImage();
		}
		else if(currentTab == TabPlayback){
			//no need to do anything atm
		}
	}
	else if(object == btnRenderBW){
		currentRenderMode = RenderBW;
	}
	else if(object == btnRenderRainbow){
		currentRenderMode = RenderRainbow;
	}
	else if(object == btnRenderPointCloud){
		currentRenderMode = RenderPointCloud;
	}
	else {
		for(int i = 0; i < btnTakes.size(); i++){
			if(object == btnTakes[i]){
				loadSequenceForPlayback( i );
			}
		}
	}
}

void ofxRGBDCaptureGui::exit(){
   	recorder.shutdown();
    if(providerSet){
		depthImageProvider->close();
        providerSet = false; 
    } 
}

void ofxRGBDCaptureGui::mousePressed(ofMouseEventArgs& args){
    
}

void ofxRGBDCaptureGui::mouseMoved(ofMouseEventArgs& args){
    
}

void ofxRGBDCaptureGui::mouseDragged(ofMouseEventArgs& args){

}
void ofxRGBDCaptureGui::mouseReleased(ofMouseEventArgs& args){

}

void ofxRGBDCaptureGui::keyPressed(ofKeyEventArgs& args){
    int key = args.key;
    
 	if(key == ' '){
		if(currentTab == TabCalibrate){
			captureCalibrationImage();
		}
		else if(currentTab == TabRecord){
			toggleRecord();
		}
		else if(currentTab == TabPlayback){
			timeline.togglePlay();
		}
	}
    
	if(key == OF_KEY_DEL && currentTab == TabCalibrate){
		alignment.discardCurrentPair();
		alignment.saveState();
	}
    
	if(key == 'f'){
		ofToggleFullscreen();
	}
	
	if(key == 'p'){
		fullscreenPoints = !fullscreenPoints;
		if(fullscreenPoints){
			btnSetDirectory->disableAllEvents();
            
			btnCalibrateTab->disableAllEvents();
			btnRecordTab->disableAllEvents();
			btnPlaybackTab->disableAllEvents();
			
			btnRecordBtn->disableAllEvents();
			
			btnRenderBW->disableAllEvents();
			btnRenderRainbow->disableAllEvents();
			btnRenderPointCloud->disableAllEvents();
			
			for(int i = 0; i < btnTakes.size(); i++) btnTakes[i]->disableAllEvents();
			ofHideCursor();
		}
		else{
			btnSetDirectory->enableAllEvents();
			
			btnCalibrateTab->enableAllEvents();
			btnRecordTab->enableAllEvents();
			btnPlaybackTab->enableAllEvents();
			
			btnRecordBtn->enableAllEvents();
			
			btnRenderBW->enableAllEvents();
			btnRenderRainbow->enableAllEvents();
			btnRenderPointCloud->enableAllEvents();
			
			for(int i = 0; i < btnTakes.size(); i++) btnTakes[i]->enableAllEvents();
            
			ofShowCursor();
		}
	}   
}

void ofxRGBDCaptureGui::keyReleased(ofKeyEventArgs& args){

}

void ofxRGBDCaptureGui::loadDirectory(){
	ofFileDialogResult r = ofSystemLoadDialog("Select Record Directory", true);
	if(r.bSuccess){
		loadDirectory(r.getPath());
	}
}

void ofxRGBDCaptureGui::loadDirectory(string path){
	workingDirectory = path;
	recorder.setRecordLocation(path+"/takes", "frame");
	ofDirectory dir(workingDirectory+"/calibration");
	if(!dir.exists()){
		dir.create(true);
	}
	alignment.loadState(workingDirectory+"/calibration/alignmentsave.xml");
	
	btnSetDirectory->setLabel(path);
	updateTakeButtons();
	ofxXmlSettings defaults;
	defaults.loadFile("defaults.xml");
	defaults.setValue("currentDir", path);
	defaults.saveFile("defaults.xml");
}

void ofxRGBDCaptureGui::loadSequenceForPlayback( int index ){
	vector<string> paths = recorder.getTakePaths();
	//TODO add timeline stuff in here;
	depthSequence.loadSequence(paths[index]);
	timeline.setDurationInFrames(depthSequence.videoThumbs.size());
}

void ofxRGBDCaptureGui::toggleRecord(){
	recorder.toggleRecord();
	updateTakeButtons();
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::captureCalibrationImage(){
	char filename[1024];
	sprintf(filename, "%s/calibration/calibration_image_%02d_%02d_%02d_%02d_%02d.png", workingDirectory.c_str(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
	//jg conv ofSaveImage( calibrationImage, filename);
	ofSaveImage( depthImageProvider->getRawIRImage(), filename);
	alignment.addDepthCalibrationImage(filename);
	alignment.generateAlignment();
	alignment.saveState();
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::updateTakeButtons(){
	vector<string> files = recorder.getTakePaths();
	
	for(int i = 0; i < btnTakes.size(); i++){
		delete btnTakes[i];
	}
	btnTakes.clear();
	
	for(int i = 0; i < files.size(); i++){
		ofxMSAInteractiveObjectWithDelegate* btnTake = new ofxMSAInteractiveObjectWithDelegate();		
		float x = framewidth;
		float y = btnheight/2*i;
		while(y >= btnheight*3+frameheight){
			y -= btnheight*3+frameheight;
			x += thirdWidth;
		}
		
		btnTake->setPosAndSize(x, y, thirdWidth, btnheight/2);
		btnTake->setLabel( ofFilePath::getFileName(files[i]) );
		btnTake->setIdleColor(idleColor);
		btnTake->setDownColor(downColor);
		btnTake->setHoverColor(hoverColor);
		btnTake->setDelegate(this);
		btnTake->enableAllEvents();
		
		btnTakes.push_back( btnTake );
	}
}

void ofxRGBDCaptureGui::drawPointcloud(bool fullscreen){

	glEnable(GL_DEPTH_TEST);
	ofMesh mesh;
	ofRectangle rect = fullscreen ? ofRectangle(0,0, ofGetWidth(), ofGetHeight()) : ofRectangle(0, btnheight*2, 640, 480);

	glPointSize(4);
	for(int y = 0; y < 480; y++){
		for(int x = 0; x < 640; x++){
			//0.104200 ref dist 120.000000
			double ref_pix_size = 0.104200;
			double ref_distance = 120.000000;
			double wz = depthSequence.currentDepthRaw[y*640+x];
			double factor = 2 * ref_pix_size * wz / ref_distance;
			double wx = (double)(x - 640/2) * factor;
			double wy = (double)(y - 480/2) * factor;
            mesh.addVertex(ofVec3f(wx,-wy,-wz));
		}
	}
    
    cam.begin(rect);
	mesh.drawVertices();
	cam.end();
    
	glDisable(GL_DEPTH_TEST);	
}

void ofxRGBDCaptureGui::windowResized(ofResizeEventArgs& args){
	timeline.setWidth(args.width);
	alignment.setMaxDrawWidth(args.width);
}
