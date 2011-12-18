/*
 *  ofxRGBDAlignment.cpp
 *  DepthExternalRGB
 *
 *  Created by Jim on 10/18/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxRGBDAlignment.h"
#include "ofxXmlSettings.h"

ofxRGBDAlignment::ofxRGBDAlignment() {
	guiIsSetup = false;
	selectedDepthImage = -1;
	selectedRgbImage = -1;
	
}

//-----------------------------------------------

ofxRGBDAlignment::~ofxRGBDAlignment() {
	
}

//-----------------------------------------------

void ofxRGBDAlignment::setup(int squaresWide, int squaresTall, int squareSize) {
	depthCalibration.setPatternSize(squaresWide, squaresTall);
	depthCalibration.setSquareSize(squareSize);
	
	rgbCalibration.setPatternSize(squaresWide, squaresTall);
	rgbCalibration.setSquareSize(squareSize);

}

void ofxRGBDAlignment::addRGBCalibrationImage(string rgbCalibrationImagePath){
	CalibrationImage ci;
	ci.filepath = rgbCalibrationImagePath;
	if(!ci.image.loadImage(ci.filepath)){
		ofLogError("ofxRGBDAlignment -- Couldn't load RGB Calibration Image at path " + rgbCalibrationImagePath);
		return;
	}
	ci.image.setImageType(OF_IMAGE_GRAYSCALE);
	ci.subpixelRefinement = 11;
	ci.reprojectionError = 0;
	rgbImages.push_back( ci );
	recalculateImageDrawRects();
}

void ofxRGBDAlignment::addDepthCalibrationImage(string depthCalibrationImagePath){
	CalibrationImage ci;
	ci.filepath = depthCalibrationImagePath;
	if(!ci.image.loadImage(ci.filepath)){
		ofLogError("ofxRGBDAlignment -- Couldn't load RGB Calibration Image at path " + depthCalibrationImagePath);
		return;
	}
	ci.image.setImageType(OF_IMAGE_GRAYSCALE);
	ci.subpixelRefinement = 11;
	ci.reprojectionError = 0;
	depthImages.push_back( ci );	
	recalculateImageDrawRects();

}

void ofxRGBDAlignment::addCalibrationImagePair(string depthCalibrationImagePath, string rgbCalibrationPath){
	addRGBCalibrationImage(rgbCalibrationPath);
	addDepthCalibrationImage(depthCalibrationImagePath);
}

void ofxRGBDAlignment::addRGBCalibrationDirectory(string rgbImageDirectory){
	ofDirectory dir(rgbImageDirectory);
	if(!dir.exists()){
		ofLogError("ofxRGBDAlignment -- RGB Image Directory " + rgbImageDirectory + " does not exist");
		return;
	}
	
	dir.allowExt("png");
	dir.allowExt("jpg");	
	dir.listDir();

	for(int i = 0; i < dir.numFiles(); i++){
		addRGBCalibrationImage( dir.getPath(i) );
		cout << "adding RGB image " << dir.getPath(i) << endl;
	}
	
	generateAlignment();
	if (guiIsSetup) {
		recalculateImageDrawRects();
	}
	
}

void ofxRGBDAlignment::addDepthCalibrationDirectory(string depthImageDirectory){
	ofDirectory dir(depthImageDirectory);
	if(!dir.exists()){
		ofLogError("ofxRGBDAlignment -- RGB Image Directory " + depthImageDirectory + " does not exist");
		return;
	}
	dir.allowExt("png");
	dir.allowExt("jpg");
	dir.listDir();
	cout << "found " << dir.numFiles() << " for depth directory " << endl;
	for(int i = 0; i < dir.numFiles(); i++){
		addDepthCalibrationImage( dir.getPath(i) );
		cout << "adding DEPTH image " << dir.getPath(i) << endl;
	}
	
	generateAlignment();
	if (guiIsSetup) {
		recalculateImageDrawRects();
	}
	
}

void ofxRGBDAlignment::addCalibrationDirectoryPair(string depthImageDirectory, string rgbImageDirectory){
	addDepthCalibrationDirectory(depthImageDirectory);
	addRGBCalibrationDirectory(rgbImageDirectory);
}

void ofxRGBDAlignment::clearRGBImages(){
	rgbImages.clear();
}

void ofxRGBDAlignment::clearDepthImages(){
	depthImages.clear();
}

void ofxRGBDAlignment::discardCurrentPair(){
	if(selectedRgbImage != -1){
		rgbImages.erase(rgbImages.begin() + selectedRgbImage);
		selectedRgbImage = -1;
	}
	if(selectedDepthImage != -1){
		depthImages.erase(depthImages.begin() + selectedDepthImage);
		selectedDepthImage = -1;
	}
	generateAlignment();
	recalculateImageDrawRects();	
}

//save and load the current image sets 
void ofxRGBDAlignment::saveState(string filePath){
	stateFilePath = filePath;
	saveState();
}

void ofxRGBDAlignment::saveState(){
	ofxXmlSettings imageLocations;
	imageLocations.addTag("images");
	imageLocations.pushTag("images");
	
	imageLocations.addTag("rgbimages");
	imageLocations.pushTag("rgbimages");
	for(int i = 0; i < rgbImages.size(); i++){
		imageLocations.addValue("image", rgbImages[i].filepath);
	}
	imageLocations.popTag();
	
	
	imageLocations.addTag("depthimages");
	imageLocations.pushTag("depthimages");
	for(int i = 0; i < depthImages.size(); i++){
		imageLocations.addValue("image", depthImages[i].filepath);	
	}
	imageLocations.popTag();
	
	imageLocations.popTag();
	imageLocations.saveFile(stateFilePath);
}

void ofxRGBDAlignment::loadState(string filePath){
	stateFilePath = filePath;
	ofxXmlSettings imageLocations;
	if(imageLocations.loadFile(stateFilePath)){
		imageLocations.pushTag("images");
		imageLocations.pushTag("rgbimages");		
		int numRGBImages = imageLocations.getNumTags("image");
		for(int i = 0; i < numRGBImages; i++){
			addRGBCalibrationImage(imageLocations.getValue("image", "", i));
		}
		imageLocations.popTag();//rgb images
		
		imageLocations.pushTag("depthimages");		
		int numDepthImages = imageLocations.getNumTags("image");
		for(int i = 0; i < numDepthImages; i++){
			addDepthCalibrationImage(imageLocations.getValue("image", "", i));
		}
		imageLocations.popTag();//depthimages
		
		imageLocations.popTag();
		generateAlignment();
		recalculateImageDrawRects();
	}
	else {
		ofLogError("ofxRGBDAlignment -- failed to load image locations at " + filePath );
	}
}


bool ofxRGBDAlignment::generateAlignment(){
	rgbCalibration.imagePoints.clear();
	depthCalibration.imagePoints.clear();
	for(int i = 0; i < rgbImages.size(); i++){
		rgbImages[i].hasCheckerboard = rgbCalibration.add(toCv(rgbImages[i].image));
	}
	
	for(int i = 0; i < depthImages.size(); i++){
		depthImages[i].hasCheckerboard = depthCalibration.add(toCv(depthImages[i].image));
	}
	
	if(depthCalibration.size() > 0){
		depthCalibration.calibrate();
	}
	if(rgbCalibration.size() > 0){
		rgbCalibration.calibrate();
	}
	
	//set reprojection errors
	int skip = 0;
	for(int i = 0; i < depthImages.size(); i++){
		if(!depthImages[i].hasCheckerboard) skip++;
		depthImages[i].reprojectionError = depthCalibration.getReprojectionError(i-skip);
	}
	skip = 0;
	for(int i = 0; i < rgbImages.size(); i++){
		if(!rgbImages[i].hasCheckerboard) skip++;
		rgbImages[i].reprojectionError = rgbCalibration.getReprojectionError(i-skip);
	}
	
	if(rgbImages.size() == depthImages.size() && depthImages.size() > 3){
		depthCalibration.getTransformation(rgbCalibration, rotationDepthToRGB, translationDepthToRGB);
		rgbCalibration.getTransformation(depthCalibration, rotationRGBToDepth, translationRGBToDepth);
	}
	
	return ready();
}

bool ofxRGBDAlignment::ready(){
	return depthCalibration.isReady() && rgbCalibration.isReady();
}

void ofxRGBDAlignment::saveAlignment(string saveDirectory) {
	if(ready()){
		ofDirectory dir(saveDirectory);
		if(!dir.exists()){
			dir.create(true);
		}
		cout << "saving alignment, rgb cam dist is " << rgbCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
		cout << "saving alignment, depth cam dist is " << depthCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
		
		depthCalibration.save(saveDirectory+"/depthCalib.yml");	
		rgbCalibration.save(saveDirectory+"/rgbCalib.yml");
		saveMat(rotationDepthToRGB, saveDirectory+"/rotationDepthToRGB.yml");
		saveMat(translationDepthToRGB, saveDirectory+"/translationDepthToRGB.yml");
		saveMat(rotationRGBToDepth, saveDirectory+"/rotationRGBToDepth.yml");
		saveMat(translationRGBToDepth, saveDirectory+"/translationRGBToDepth.yml");

	}
	else {
		ofLogWarning("ofxRGBDAlignment -- Could not save alignment, it's not ready");
	}
}


Calibration & ofxRGBDAlignment::getDepthCalibration(){
	return depthCalibration;
}

Calibration & ofxRGBDAlignment::getRGBCalibration(){
	return rgbCalibration;
}


////////// GUI STUFF
void ofxRGBDAlignment::setupGui(float x, float y, float maxDrawWidth){
	guiIsSetup = true;
	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);
	guiPosition = ofVec2f(x,y);
	maxGuiDrawWidth = maxDrawWidth;
	infoBoxHeight = 20;//so we can write info about the image below
	recalculateImageDrawRects();
	
}

void ofxRGBDAlignment::setMaxDrawWidth(float maxDrawWidth){
	maxGuiDrawWidth = maxDrawWidth;
	recalculateImageDrawRects();
}

void ofxRGBDAlignment::drawGui(){
	drawImagePairs();
	//TOOD add in previews for selected objects
	
}

void ofxRGBDAlignment::drawImagePairs(){
	drawDepthImages();
	drawRGBImages();
}

void ofxRGBDAlignment::drawDepthImages(){
	ofPushStyle();
	for(int i = 0; i < depthImages.size(); i++){
		ofRectangle drawRect = depthImages[i].drawRect;
		if(depthImages[i].hasCheckerboard){
			ofSetColor(255);
		}
		else{
			ofSetColor(255, 0, 0);
		}
		depthImages[i].image.draw(drawRect);
		ofDrawBitmapString("Er " + ofToString(depthImages[i].reprojectionError, 3), drawRect.x, drawRect.y+drawRect.height+10);
	}
	
	if(selectedDepthImage != -1){
		ofPushStyle();
		ofNoFill();
		ofSetColor(255, 0, 0);
		ofSetLineWidth(2);
		ofRect(depthImages[selectedDepthImage].drawRect);
		ofPopStyle();
	}
	ofPopStyle();
}

void ofxRGBDAlignment::drawRGBImages(){
	ofPushStyle();
	for(int i = 0; i < rgbImages.size(); i++){
		ofRectangle drawRect = rgbImages[i].drawRect;
		rgbImages[i].image.draw(drawRect);
		if(rgbImages[i].hasCheckerboard){
			ofSetColor(255);
		}
		else{
			ofSetColor(255, 0, 0);
		}
		
		ofDrawBitmapString("Er " + ofToString(rgbImages[i].reprojectionError,3), drawRect.x, drawRect.y+drawRect.height+10);
	}
	
	if(selectedRgbImage != -1){
		ofPushStyle();
		ofNoFill();
		ofSetColor(255, 0, 0);
		ofSetLineWidth(2);
		ofRect(rgbImages[selectedRgbImage].drawRect);
		ofPopStyle();
	}
	ofPopStyle();
}

ofImage& ofxRGBDAlignment::getCurrentDepthImage(){
	return currentDepthImage;
}

ofImage& ofxRGBDAlignment::getCurrentRGBImage(){
	return currentRGBImage;
}

void ofxRGBDAlignment::recalculateImageDrawRects(){

	if(depthImages.size() == 0 && rgbImages.size() == 0){
		return;
	}
	
	//we calculate a shared width for each image, and find the right height to maintain aspect ratio
	float calculatedDrawWidth;
	float calculatedDepthDrawHeight = 0;
	float calculatedRGBDrawHeight = 0;
	if(depthImages.size() > rgbImages.size()){
		calculatedDrawWidth = MIN(depthImages[0].image.getWidth(), maxGuiDrawWidth/depthImages.size());
	}
	else{
		calculatedDrawWidth = MIN(rgbImages[0].image.getWidth(), maxGuiDrawWidth/rgbImages.size());
	}
	
	if(depthImages.size() > 0){
		calculatedDepthDrawHeight = calculatedDrawWidth/depthImages[0].image.getWidth() * depthImages[0].image.getHeight();
	}
	
	if(rgbImages.size() > 0){
		calculatedRGBDrawHeight = calculatedDrawWidth/rgbImages[0].image.getWidth() * rgbImages[0].image.getHeight();
	}
	
	for(int i = 0; i < depthImages.size(); i++){
		depthImages[i].drawRect = ofRectangle(guiPosition.x + i*calculatedDrawWidth, guiPosition.y, calculatedDrawWidth, calculatedDepthDrawHeight);
	}
	
	for(int i = 0; i < rgbImages.size(); i++){
		rgbImages[i].drawRect = ofRectangle(guiPosition.x + i*calculatedDrawWidth, guiPosition.y+calculatedDepthDrawHeight+infoBoxHeight, calculatedDrawWidth, calculatedRGBDrawHeight);
	}
}

void ofxRGBDAlignment::keyPressed(ofKeyEventArgs& args){
	if (args.key == OF_KEY_LEFT) {
		if(selectedRgbImage != -1){
			
		}
	}
	else if(args.key == OF_KEY_RIGHT){
		if(selectedRgbImage != -1 && selectedRgbImage != rgbImages.size()-1){
			CalibrationImage temp = rgbImages[selectedRgbImage+1];
			rgbImages[selectedRgbImage+1] = rgbImages[selectedRgbImage];
			rgbImages[selectedRgbImage] = temp;
		}
	}
}

void ofxRGBDAlignment::keyReleased(ofKeyEventArgs& args){
	
}

void ofxRGBDAlignment::mouseMoved(ofMouseEventArgs& args){

}

void ofxRGBDAlignment::mouseDragged(ofMouseEventArgs& args){
	
}

void ofxRGBDAlignment::mousePressed(ofMouseEventArgs& args){
	for(int i = 0; i < depthImages.size(); i++){
		if (depthImages[i].drawRect.inside(args.x,args.y)) {
			selectedDepthImage = i;
			if(i < rgbImages.size()){
				selectedRgbImage = i;
			}
			break;
		}
	}
	for(int i = 0; i < rgbImages.size(); i++){
		if (rgbImages[i].drawRect.inside(args.x,args.y)) {
			selectedRgbImage = i;
			if(i < depthImages.size()){
				selectedDepthImage = i;
			}
			break;
		}
	}
}

void ofxRGBDAlignment::mouseReleased(ofMouseEventArgs& args){

}
