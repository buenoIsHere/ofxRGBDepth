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
	currentRenderMode = RenderBW;
    
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
    currentTabObject = btnCalibrateTab;
	
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
    currentRenderModeObject = btnRenderBW;
    
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
	btnRecordBtn->setLabel("Capture Chessboard");
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
    timeline.getColors().loadColors("defaultColors.xml");
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
	
    cam.setup();
	cam.loadCameraPosition();
	
	cam.speed = 25;
	cam.setFarClip(50000);
    

    ofRegisterMouseEvents(this);
    ofRegisterKeyEvents(this);    
    ofAddListener(ofEvents().windowResized, this, &ofxRGBDCaptureGui::windowResized);
   // ofAddListener(ofEvents.exit, this, &ofxRGBDCaptureGui::exit);
    ofAddListener(ofEvents().update, this, &ofxRGBDCaptureGui::update);
    ofAddListener(ofEvents().draw, this, &ofxRGBDCaptureGui::draw);
    
    createRainbowPallet();
    depthImage.allocate(640, 480, OF_IMAGE_COLOR);
    
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
        if(currentTab == TabRecord){
	        updateDepthImage(depthImageProvider->getRawDepth());
        }
		else if(currentTab == TabCalibrate){
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
		drawPointcloud(depthSequence.currentDepthRaw, true);
		return;
	}
    
    bool drawCamera = providerSet && depthImageProvider->deviceFound();    
	if(currentTab == TabCalibrate){
        if(!drawCamera){
            ofPushStyle();
            ofSetColor(255, 0, 0);
            ofDrawBitmapString("Camera not found. Plug and unplug the device and restart the application.", previewRect.x + 30, previewRect.y + 30);
            ofPopStyle();
        }
        else{
            depthImageProvider->getRawIRImage().draw(previewRect);
            calibrationPreview.draw(0, btnheight*2);
            alignment.drawDepthImages();            
        }
	}
	else if(currentTab == TabRecord){

        if(!drawCamera){
            ofPushStyle();
            ofSetColor(255, 0, 0);
            ofDrawBitmapString("Camera not found. Plug and unplug the device and restart the application.", previewRect.x + 30, previewRect.y + 30);
            ofPopStyle();
        }
        else{
            if( currentRenderMode == RenderPointCloud){
                drawPointcloud(depthImageProvider->getRawDepth(), false);
            }
            else{
                ofPushStyle();
                ofSetColor(255, 255, 255, 60);
                ofLine(320, btnheight*2, 320, btnheight*2+480);
                ofLine(0, btnheight*2+240, 640, btnheight*2+240);
                ofPopStyle();
                //depthImageProvider->getDepthImage().draw(previewRect);        
                depthImage.draw(previewRect);
            }
        }
	}
	else if(currentTab == TabPlayback) {
        if(currentRenderMode == RenderPointCloud){
            drawPointcloud(depthSequence.currentDepthRaw, false);            
        }
        else {
            updateDepthImage(depthSequence.currentDepthRaw);
            depthImage.draw(previewRect);
        }
    
		//draw timeline
		timeline.draw();
	}
    
    if(currentTabObject != NULL){
        ofPushStyle();
        ofRectangle highlightRect = ofRectangle(currentTabObject->x,currentTabObject->y+currentTabObject->height*.75,
                                                currentTabObject->width,currentTabObject->height*.25);
        ofSetColor(timeline.getColors().highlightColor);
        ofRect(highlightRect);        
        ofPopStyle();    	
    }

    if(currentRenderModeObject != NULL){
        ofPushStyle();
        ofRectangle highlightRect = ofRectangle(currentRenderModeObject->x,currentRenderModeObject->y+currentRenderModeObject->height*.75,
                                                currentRenderModeObject->width,currentRenderModeObject->height*.25);
        ofSetColor(timeline.getColors().highlightColor);
        ofRect(highlightRect);        
        ofPopStyle();    	    
    }
    
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
        currentTabObject = btnCalibrateTab;
        btnRecordBtn->setLabel("Capture Chessboard");
	}
	else if(object == btnRecordTab){
		currentTab = TabRecord;
        currentTabObject = btnRecordTab;
    	btnRecordBtn->setLabel("Toggle Record");
	}
	else if(object == btnPlaybackTab){
		currentTab = TabPlayback;
        currentTabObject = btnPlaybackTab;
    	btnRecordBtn->setLabel("Reset Camera");
	}
	else if(object == btnRecordBtn){
		if(currentTab == TabRecord){
			toggleRecord();
		}
		else if(currentTab == TabCalibrate){
			captureCalibrationImage();
		}
		else if(currentTab == TabPlayback){
			cam.targetNode.setPosition(0, 0, 0);
            cam.targetNode.setOrientation(ofQuaternion());
            cam.targetXRot = cam.targetYRot = cam.targetZRot = 0;
		}
	}
	else if(object == btnRenderBW){
		currentRenderMode = RenderBW;
        currentRenderModeObject = btnRenderBW;
	}
	else if(object == btnRenderRainbow){
		currentRenderMode = RenderRainbow;
        currentRenderModeObject = btnRenderRainbow;
	}
	else if(object == btnRenderPointCloud){
		currentRenderMode = RenderPointCloud;
        currentRenderModeObject = btnRenderPointCloud;
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
    if(path == ""){
        ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/");
        loadDefaultDirectory();
        return;
    }
    
    ofDirectory workingDirPath(path);
    if(!workingDirPath.exists()){
        ofSystemAlertDialog("The Working directory empty. Resetting to bin/data/depthframes/");
        loadDefaultDirectory();     
        return;
    }
    
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

void ofxRGBDCaptureGui::loadDefaultDirectory(){

    //create it if it doesn't exist
    string defaultDir = "depthframes";
    if(!ofDirectory(defaultDir).exists()){
        ofDirectory(defaultDir).create(true);
    }
    loadDirectory(defaultDir);
	
}

void ofxRGBDCaptureGui::loadSequenceForPlayback( int index ){
    depthSequence.loadSequence( recorder.getTakes()[index]->path );
	timeline.setDurationInFrames(depthSequence.videoThumbs.size());
}

void ofxRGBDCaptureGui::toggleRecord(){
	recorder.toggleRecord();
	updateTakeButtons();
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::captureCalibrationImage(){
//    if(calibrationPreview.hasFoundBoard()){

    char filename[1024];
    sprintf(filename, "%s/calibration/calibration_image_%02d_%02d_%02d_%02d_%02d.png", workingDirectory.c_str(), ofGetMonth(), ofGetDay(), ofGetHours(), ofGetMinutes(), ofGetSeconds());

    ofSaveImage( depthImageProvider->getRawIRImage(), filename);
    alignment.addDepthCalibrationImage(filename);
    alignment.generateAlignment();
    alignment.saveState();
//    }
}

//--------------------------------------------------------------
void ofxRGBDCaptureGui::updateTakeButtons(){
	vector<Take*>& takes = recorder.getTakes();
	
	for(int i = 0; i < btnTakes.size(); i++){
        btnTakes[i].button->disableAllEvents();
        btnTakes[i].button->enabled = false;
		delete btnTakes[i].button;
	}
    
	btnTakes.clear();
	
	for(int i = 0; i < takes.size(); i++){
        TakeButton tb;
        tb.isSelected = false;
        tb.takeRef = takes[i];
        
		ofxMSAInteractiveObjectWithDelegate* btnTake = new ofxMSAInteractiveObjectWithDelegate();		
		float x = framewidth;
		float y = btnheight*.66*i;
		while(y >= btnheight*3+frameheight){
			y -= btnheight*3+frameheight;
			x += thirdWidth;
		}
		
		btnTake->setPosAndSize(x, y, thirdWidth, btnheight*.66);
		btnTake->setLabel( ofFilePath::getFileName(takes[i]->path) );
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

void ofxRGBDCaptureGui::drawPointcloud(ofShortPixels& pix, bool fullscreen){

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
			double wz = pix.getPixels()[y*640+x];
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

//COLORING
//taken from OpenNI
void ofxRGBDCaptureGui::createRainbowPallet() {
	unsigned char r, g, b;
	memset(LUTR, 0, 256);
	memset(LUTG, 0, 256);
	memset(LUTB, 0, 256);
	
	for (int i=1; i<255; i++) {
		if (i<=29) {
			r = (unsigned char)(129.36-i*4.36);
			g = 0;
			b = (unsigned char)255;
		}
		else if (i<=86) {
			r = 0;
			g = (unsigned char)(-133.54+i*4.52);
			b = (unsigned char)255;
		}
		else if (i<=141) {
			r = 0;
			g = (unsigned char)255;
			b = (unsigned char)(665.83-i*4.72);
		}
		else if (i<=199) {
			r = (unsigned char)(-635.26+i*4.47);
			g = (unsigned char)255;
			b = 0;
		}
		else {
			r = (unsigned char)255;
			g = (unsigned char)(1166.81-i*4.57);
			b = 0;
		}
		LUTR[i] = r;
		LUTG[i] = g;
		LUTB[i] = b;
	}
}

void ofxRGBDCaptureGui::updateDepthImage(ofShortPixels& pixels){
    
    if(!pixels.isAllocated()){
        return;
    }
    
    int max_depth = depthImageProvider->maxDepth();    
    if(max_depth == 0){
    	max_depth = 5000;
    }
//    cout << "updating depth image with max depth of " << max_depth << " render: " << (currentRenderMode == RenderRainbow ? "rainbow" : "b&w") <<  endl;
    if(currentRenderMode == RenderRainbow){
        for(int i = 0; i < 640*480; i++){
            int lookup = pixels.getPixels()[i] / (max_depth / 256);
            //int lookup = ofMap( depthPixels.getPixels()[i], 0, max_depth, 0, 255, true);
            depthImage.getPixels()[(i*3)+0] = LUTR[lookup];
            depthImage.getPixels()[(i*3)+1] = LUTG[lookup];
            depthImage.getPixels()[(i*3)+2] = LUTB[lookup];
        }
    }
    else{
        recorder.compressorRef().convertTo8BitImage(pixels, depthImage);
    }
    
    depthImage.update();
	
}