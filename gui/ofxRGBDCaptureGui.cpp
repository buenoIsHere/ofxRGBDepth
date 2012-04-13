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
	buttonSet.push_back(btnSetDirectory);
    
	btnCalibrateTab = new ofxMSAInteractiveObjectWithDelegate();
	btnCalibrateTab->setPosAndSize(0, btnheight, thirdWidth, btnheight);
	btnCalibrateTab->setLabel("Calibrate");
	buttonSet.push_back(btnCalibrateTab);
	
	btnRecordTab = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordTab->setPosAndSize(thirdWidth, btnheight, thirdWidth, btnheight);
	btnRecordTab->setLabel("Record");
	buttonSet.push_back(btnRecordTab);
    
	btnPlaybackTab = new ofxMSAInteractiveObjectWithDelegate();
	btnPlaybackTab->setPosAndSize(thirdWidth*2, btnheight, thirdWidth, btnheight);
	btnPlaybackTab->setLabel("Playback");
	buttonSet.push_back(btnPlaybackTab);
    
	btnRenderBW = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderBW->setPosAndSize(0, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderBW->setLabel("Blaick&White");
	buttonSet.push_back(btnRenderBW);
    
	btnRenderRainbow = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderRainbow->setPosAndSize(thirdWidth, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderRainbow->setLabel("Rainbow");
	buttonSet.push_back(btnRenderRainbow);
    
	btnRenderPointCloud = new ofxMSAInteractiveObjectWithDelegate();
	btnRenderPointCloud->setPosAndSize(thirdWidth*2, btnheight*2+frameheight, thirdWidth, btnheight);
	btnRenderPointCloud->setLabel("Pointcloud");
	buttonSet.push_back(btnRenderPointCloud);
    
	btnRecordBtn = new ofxMSAInteractiveObjectWithDelegate();
	btnRecordBtn->setPosAndSize(0, btnheight*3+frameheight, framewidth, btnheight);
	btnRecordBtn->setLabel("Record");
    buttonSet.push_back(btnRecordBtn);
    
    for(int i = 0; i < buttonSet.size(); i++){
        buttonSet[i]->setIdleColor(idleColor);
        buttonSet[i]->setDownColor(downColor);
        buttonSet[i]->setHoverColor(hoverColor);
        buttonSet[i]->disableKeyEvents();
        buttonSet[i]->setDelegate(this);
    }
    
    previewRect = ofRectangle(0, btnheight*2, 640, 480);
    
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
    
    
    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);    
    ofAddListener(ofEvents.windowResized, this, &ofxRGBDCaptureGui::windowResized);
   // ofAddListener(ofEvents.exit, this, &ofxRGBDCaptureGui::exit);
    ofAddListener(ofEvents.update, this, &ofxRGBDCaptureGui::update);
    ofAddListener(ofEvents.draw, this, &ofxRGBDCaptureGui::draw);
    
    recorder.setup();

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
		depthImageProvider->getRawIRImage().draw(previewRect);
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
		depthImageProvider->getDepthImage().draw(previewRect);
	}
	else {
		if(currentRenderMode == RenderBW || currentRenderMode == RenderRainbow){
			depthSequence.currentDepthImage.draw(previewRect);
		}
		else {
			if(depthSequence.currentDepthRaw != NULL){
				drawPointcloud(false);
			}
		}
        
		//draw timeline
		timeline.draw();
	}
    
    //-- -- -- -- -- -- - 
    for(int i = 0; i < btnTakes.size(); i++){
    	if(btnTakes[i].isSelected){
        	ofPushStyle();
            ofSetColor(timeline.getColors().highlightColor);
            ofRectangle highlighRect(btnTakes[i].button->x,btnTakes[i].button->y,
                                     btnTakes[i].button->width, btnTakes[i].button->height*.25);
                                     
            ofRect(highlighRect);
            ofPopStyle();
        }
    	
        ofPushStyle();
        ofSetColor(timeline.getColors().disabledColor);
        float percentComplete = float(btnTakes[i].takeRef->framesConverted) / float(btnTakes[i].takeRef->numFrames);
        float processedWidth = btnTakes[i].button->width*percentComplete;
        ofRectangle highlighRect(btnTakes[i].button->x + processedWidth,
                                 btnTakes[i].button->y,
                                 btnTakes[i].button->width-processedWidth, btnTakes[i].button->height);
        ofRect(highlighRect);
        ofPopStyle();
    }
    
	//draw save meter if buffer is getting full
	if(recorder.isRecording()){
		ofPushStyle();
		ofSetColor(255, 0, 0);
		ofNoFill();
		ofSetLineWidth(5);
		
		ofRect(previewRect);
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
			if(object == btnTakes[i].button){
				loadSequenceForPlayback( i );                
                for(int b = 0; b < btnTakes.size(); b++){
                	btnTakes[b].isSelected = b == i;
                }
                break;
			}
		}
	}
}

void ofxRGBDCaptureGui::exit(){
    //causes crashing...
//  recorder.shutdown();
    if(providerSet){
//		depthImageProvider->close();
        providerSet = false; 
    } 
}

void ofxRGBDCaptureGui::mousePressed(ofMouseEventArgs& args){
    
}

void ofxRGBDCaptureGui::mouseMoved(ofMouseEventArgs& args){
    cam.usemouse = fullscreenPoints || previewRect.inside(args.x, args.y);
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
            for(int i = 0; i < buttonSet.size(); i++){
                buttonSet[i]->disableAppEvents();
                buttonSet[i]->disableMouseEvents();
            }
            for(int i = 0; i < btnTakes.size(); i++){
             	btnTakes[i].button->disableAppEvents();   
                btnTakes[i].button->disableMouseEvents();   
            }
			ofHideCursor();
		}
		else{
            for(int i = 0; i < buttonSet.size(); i++){
                buttonSet[i]->enableMouseEvents();
                buttonSet[i]->enableAppEvents();
            }
			for(int i = 0; i < btnTakes.size(); i++){
             	btnTakes[i].button->enableMouseEvents();
                btnTakes[i].button->enableAppEvents();
            }
            
			ofShowCursor();			
		}
	}   
}

void ofxRGBDCaptureGui::keyReleased(ofKeyEventArgs& args){

}

void ofxRGBDCaptureGui::loadDirectory(){
    if(recorder.numFramesWaitingCompession() != 0){
    	ofSystemAlertDialog("Cannot change directory while files are converting");
		return;
    }
    
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
    depthSequence.loadSequence( recorder.getTakes()[index].path );
	timeline.setDurationInFrames(depthSequence.videoThumbs.size());
}

void ofxRGBDCaptureGui::toggleRecord(){
	recorder.toggleRecord();
	updateTakeButtons();
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::captureCalibrationImage(){
    if(calibrationPreview.hasFoundBoard()){

        char filename[1024];
        sprintf(filename, "%s/calibration/calibration_image_%02d_%02d_%02d_%02d_%02d.png", workingDirectory.c_str(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());
        //jg conv ofSaveImage( calibrationImage, filename);
        ofSaveImage( depthImageProvider->getRawIRImage(), filename);
        alignment.addDepthCalibrationImage(filename);
        alignment.generateAlignment();
        alignment.saveState();
    }
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::updateTakeButtons(){
	vector<Take>& takes = recorder.getTakes();
	
	for(int i = 0; i < btnTakes.size(); i++){
		delete btnTakes[i].button;
	}
    
	btnTakes.clear();
	
	for(int i = 0; i < takes.size(); i++){
        TakeButton tb;
        tb.isSelected = false;
        tb.takeRef = &takes[i];
        
		ofxMSAInteractiveObjectWithDelegate* btnTake = new ofxMSAInteractiveObjectWithDelegate();		
		float x = framewidth;
		float y = btnheight*.66*i;
		while(y >= btnheight*3+frameheight){
			y -= btnheight*3+frameheight;
			x += thirdWidth;
		}
		
		btnTake->setPosAndSize(x, y, thirdWidth, btnheight*.66);
		btnTake->setLabel( ofFilePath::getFileName(takes[i].path) );
		btnTake->setIdleColor(idleColor);
		btnTake->setDownColor(downColor);
		btnTake->setHoverColor(hoverColor);
		btnTake->setDelegate(this);
        btnTake->enableMouseEvents();
        btnTake->disableKeyEvents();
        
		tb.button = btnTake;
		btnTakes.push_back( tb );
	}
}

void ofxRGBDCaptureGui::drawPointcloud(bool fullscreen){

	glEnable(GL_DEPTH_TEST);
	ofMesh mesh;
	ofRectangle rect = fullscreen ? ofRectangle(0,0, ofGetWidth(), ofGetHeight()) : previewRect;
    //glEnable(GL_POINT_SMOOTH);
    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);	// allows per-point size
    glPointSize(2);
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
