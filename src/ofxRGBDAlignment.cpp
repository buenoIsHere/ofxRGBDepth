/*
 *  ofxRGBDAlignment.cpp
 *  DepthExternalRGB
 *
 *  Created by Jim on 10/18/11.
 *  Copyright 2011 University of Washington. All rights reserved.
 *
 */

#include "ofxRGBDAlignment.h"


ofxRGBDAlignment::ofxRGBDAlignment() {
	xshift = yshift = 0;
}

//-----------------------------------------------

ofxRGBDAlignment::~ofxRGBDAlignment() {
	
}

//-----------------------------------------------

void ofxRGBDAlignment::setup(int squaresWide, int squaresTall, int squareSize) {
	hasDepthImage = false;
	hasColorImage = false;
	
	depthCalibration.setPatternSize(squaresWide, squaresTall);
	depthCalibration.setSquareSize(squareSize);
	
	colorCalibration.setPatternSize(squaresWide, squaresTall);
	colorCalibration.setSquareSize(squareSize);
	
	mesh.setUsage(GL_STREAM_DRAW);
//	mesh.getVertices().reserve(640*480);
//	mesh.getIndices().reserve(640*480*3);
//	mesh.getTexCoords().reserve(640*480);	
}

//-----------------------------------------------
bool ofxRGBDAlignment::addCalibrationImagePair(ofPixels &ir, ofPixels &camera) {

	if(depthCalibration.add(toCv(ir))){
		if(!colorCalibration.add(toCv(camera))){
			depthCalibration.imagePoints.erase(depthCalibration.imagePoints.end()-1);
		}		
	}
	
//	if(depthCalibration.imagePoints.size() != colorCalibration.imagePoints.size()){
//		ofLogError("ofxRGBDAlignment -- image point sizes differ!");
//		return false;
//	}
	
	if(depthCalibration.imagePoints.size() > 3){
		depthCalibration.calibrate();
		colorCalibration.calibrate();
	}
	
	if(depthCalibration.isReady() && colorCalibration.isReady()){
		depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
		colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
		
		cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
		cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;

		cout << "Depth ERROR::: " << depthCalibration.getReprojectionError() << endl;
		cout << "Color ERROR::: " << colorCalibration.getReprojectionError() << endl;
		return true;
	}
	
	//cout << " NOT READY! " << depthCalibration.isReady() << " " <<  colorCalibration.isReady()  << " ? ?" << depthCalibration.imagePoints.size() << " " << colorCalibration.imagePoints.size() << endl;
	return false;
}

//-----------------------------------------------

bool ofxRGBDAlignment::calibrateFromDirectoryPair(string depthImageDirectory, string colorImageDirectory){
	
	depthCalibration.calibrateFromDirectory(depthImageDirectory);
	colorCalibration.calibrateFromDirectory(colorImageDirectory);
	
	depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
	colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
	
	cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
	cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;
	
	return depthCalibration.isReady() && colorCalibration.isReady();
}

bool ofxRGBDAlignment::ready(){
	return depthCalibration.isReady() && colorCalibration.isReady();
}

void ofxRGBDAlignment::saveCalibration() {
	depthCalibration.save("depthCalib.yml");	
	colorCalibration.save("colorCalib.yml");
}

void ofxRGBDAlignment::loadCalibration(string calibrationDirectory) {
	
	depthCalibration.load(calibrationDirectory+"/depthCalib.yml");
	colorCalibration.load(calibrationDirectory+"/colorCalib.yml");
	depthCalibration.calibrate();
	colorCalibration.calibrate();
	
	depthCalibration.getTransformation(colorCalibration, rotationDepthToColor, translationDepthToColor);
	colorCalibration.getTransformation(depthCalibration, rotationColorToDepth, translationColorToDepth);
	
	cout << "Kinect to Color:" << endl << rotationDepthToColor << endl << translationDepthToColor << endl;
	cout << "Color to Kinect:" << endl << rotationColorToDepth << endl << translationColorToDepth << endl;

}

void ofxRGBDAlignment::resetCalibration() {
	//TODO:
}

Calibration & ofxRGBDAlignment::getKinectCalibration(){
	return depthCalibration;
}

Calibration & ofxRGBDAlignment::getExternalCalibration(){
	return colorCalibration;
}

//-----------------------------------------------
void ofxRGBDAlignment::setColorTexture(ofBaseHasTexture& colorImage) {
	currentColorImage = &colorImage;
	hasColorImage = true;
}

//-----------------------------------------------


void ofxRGBDAlignment::update(unsigned short* depthPixelsRaw){
	int w = 640;
	int h = 480;
	
	currentDepthImage = depthPixelsRaw;
	hasDepthImage = true;
	
//	pointCloud.clear();

	Point2d fov = depthCalibration.getUndistortedIntrinsics().getFov();
	float fx = tanf(ofDegToRad(fov.x) / 2) * 2;
	float fy = tanf(ofDegToRad(fov.y) / 2) * 2;
	
	Point2d principalPoint = depthCalibration.getUndistortedIntrinsics().getPrincipalPoint();
	cv::Size imageSize = depthCalibration.getUndistortedIntrinsics().getImageSize();
	
	
    currentDepthImage = depthPixelsRaw;
	hasDepthImage = true;
	
	xshift = yshift = 0;
	
	int validPointCount = 0;
	ofVec3f center(0,0,0);
	int index = 0;
	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {

			//float pixel = rawToCentimeters( currentDepthImage[y*w+j] );
            unsigned short z = currentDepthImage[y*w+x];
            float xReal = (((float) x - principalPoint.x) / imageSize.width) * z * fx + xshift;
            float yReal = (((float) y - principalPoint.y) / imageSize.height) * z * fy + yshift;
            // add each point into pointCloud
            //pointCloud.push_back(Point3f(xReal, yReal, z));
			ofVec3f vert = ofVec3f(xReal, yReal, z);
			if(mesh.getVertices().size() > index){
				mesh.setVertex(index, vert);
			}
			else{
				mesh.addVertex(vert);
			}
			index++;
            if(z > 1){
                //pointCloud.push_back(Point3f(x/640.0, y/640., 0));
                center += ofVec3f(xReal, yReal, z);
                validPointCount++;
            }
		}
	}
	
//	for(; index < 640*480; index++){
//		mesh.getVertices()[index] = ofVec3f(0,0,0);
//	}
	
//    meshCenter = center / validPointCount;
//	meshDistance = 0;
//	for(int i = 0; i < pointCloud.size(); i++){
//		if(pointCloud[i].z > 1){
//			float thisDistance = center.distance(ofVec3f(pointCloud[i].x,
//														 pointCloud[i].y,
//														 pointCloud[i].z));
//			if(thisDistance > meshDistance){
//				meshDistance = thisDistance;
//			}
//		}
//	}
    
//    cout << "mesh center " <<  meshCenter << " distance " << meshDistance << endl;
		
	updateColors();
    updateMesh();
}

void ofxRGBDAlignment::updateMesh() {
	
//	cout << "mesh has " << mesh.getVertices().size() << " vertices " << endl;
	
	Mat pcMat = Mat(toCv(mesh));
	
	//cout << "PC " << pcMat << endl;
	//	cout << "Rot Depth->Color " << rotationDepthToColor << endl;
	//	cout << "Trans Depth->Color " << translationDepthToColor << endl;
	//	cout << "Intrs Cam " << colorCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
	//	cout << "Intrs Dist Coef " << colorCalibration.getDistCoeffs() << endl;
	
	imagePoints.clear();
	projectPoints(pcMat,
				  rotationDepthToColor, translationDepthToColor,
				  colorCalibration.getDistortedIntrinsics().getCameraMatrix(),
				  colorCalibration.getDistCoeffs(),
				  imagePoints);
	
//    texcoords.clear();
//    vertices.clear();
//	mesh.clearVertices();
	int index = 0;
	
	for(int i = 0; i < imagePoints.size(); i++) {
		ofVec2f textureCoord = ofVec2f(imagePoints[i].x, imagePoints[i].y);
		//int j = (int)imagePoints[i].y * currentColorImage->getTextureReference().getWidth() + (int)imagePoints[i].x;
		//ofFloatColor color;
		//color = ofFloatColor(1, 1, 1, 1);		
        //vertices.push_back( toOf(pointCloud[i]) );
        //texcoords.push_back( textureCoord );
        
		//mesh.setColor(i, color);
		if(mesh.getTexCoords().size() > index){
			mesh.setTexCoord(i, textureCoord);
		}
		else{
			mesh.addTexCoord(textureCoord);
		}
		index++;
//		mesh.addVertex(toOf(pointCloud[i]));
	}
    
	int facesAdded = 0;
	int indecesAdded = 0;

	int w = 640;
	int h = 480;
	for (int y = 0; y < h-1; y++){
		for (int x=0; x < w-1; x++){
			ofIndexType a,b,c;
			a = x+y*w;
			b = (x+1)+y*w;
			c = x+(y+1)*w;
			if(mesh.getVertices()[a].z > zthresh &&
			   mesh.getVertices()[b].z > zthresh &&
			   mesh.getVertices()[c].z > zthresh)
			{                
				if(mesh.getIndices().size() > indecesAdded){
					mesh.setIndex(indecesAdded++, a);// 0
					mesh.setIndex(indecesAdded++, b);			// 1
					mesh.setIndex(indecesAdded++, c);		// 10									
				}
				else{
					mesh.addTriangle(a, b, c);
					indecesAdded+=3;
				}
				facesAdded++;
			}
			a = (x+1)+y*w;
			b = x+(y+1)*w;
			c = (x+1)+(y+1)*w;
			if(mesh.getVertices()[a].z > zthresh &&
			   mesh.getVertices()[b].z > zthresh &&
			   mesh.getVertices()[c].z > zthresh)
			{        
				if(mesh.getIndices().size() > indecesAdded){
					mesh.setIndex(indecesAdded++, a);// 0
					mesh.setIndex(indecesAdded++, b);			// 1
					mesh.setIndex(indecesAdded++, c);		// 10									
				}
				else{
					mesh.addTriangle(a, b, c);
					indecesAdded+=3;
				}
			}
		}
	}
	
//	for(int i = indecesAdded; i < 
}

/*
void ofxRGBDAlignment::setPointCloud(vector<Point3f>& newCloud){
//	pointCloud.clear();
//	for(int i = 0; i < newCloud.size(); i++){
//		pointCloud.push_back( toCv(newCloud[i]) );
//	}
	pointCloud = newCloud;
	if(colorCalibration.isReady() && depthCalibration.isReady()){
		updateColors();
		//updateMesh();
	}
}
*/

void ofxRGBDAlignment::updateColors() {
//	pointCloudColors.clear();
//	imagePoints.clear();
	
	// rotate, translate the points to fit the colorCalibration perspective
	// and project them onto the colorCalibration image space
	// and undistort them
/*
	Mat pcMat = Mat(pointCloud);
	
	//cout << "PC " << pcMat << endl;
	//	cout << "Rot Depth->Color " << rotationDepthToColor << endl;
	//	cout << "Trans Depth->Color " << translationDepthToColor << endl;
	//	cout << "Intrs Cam " << colorCalibration.getDistortedIntrinsics().getCameraMatrix() << endl;
	//	cout << "Intrs Dist Coef " << colorCalibration.getDistCoeffs() << endl;

	projectPoints(pcMat,
				  rotationDepthToColor, translationDepthToColor,
				  colorCalibration.getDistortedIntrinsics().getCameraMatrix(),
				  colorCalibration.getDistCoeffs(),
				  imagePoints);

	// get the color at each of the projectedPoints inside curColor
	// add them into pointCloudColors
	int w = currentColorImage->getTextureReference().getWidth();
	int h = currentColorImage->getTextureReference().getHeight();
	int n = w * h;
	unsigned char* pixels = currentColorImage->getPixels();
	for(int i = 0; i < imagePoints.size(); i++) {
		int j = (int) imagePoints[i].y * w + (int) imagePoints[i].x;
		//pointCloudColors.push_back(Point3f(1, 1, 1));
		if(j < 0 || j >= n) {
			pointCloudColors.push_back(Point3f(0, 0, 0));
		} else {
			j *= 3;
			pointCloudColors.push_back(Point3f(pixels[j + 0] / 255.f, pixels[j + 1] / 255.f, pixels[j + 2] / 255.f));
		}
	}
	*/
}

void ofxRGBDAlignment::drawCalibration(bool left){
    if(left){
        depthCalibration.draw3d();
    }
    else{
        colorCalibration.draw3d();
    }
}

ofVec3f ofxRGBDAlignment::getMeshCenter(){
	return meshCenter;
}

float ofxRGBDAlignment::getMeshDistance(){
	return meshDistance;
}

void ofxRGBDAlignment::drawMesh() {
	glPushMatrix();
	glScaled(1, -1, 1);

	glEnable(GL_DEPTH_TEST);
	currentColorImage->getTextureReference().bind();
	mesh.drawFaces();
    //vbo.drawElements(GL_TRIANGLES, indeces.size()/3);
	currentColorImage->getTextureReference().unbind();
	glDisable(GL_DEPTH_TEST);
	
	glPopMatrix();


}

void ofxRGBDAlignment::drawPointCloud() {
	
//	if(colorCalibration.isReady()){
//		return;
//	}
	
	ofPushStyle();
	
	ofSetColor(255);
	glPointSize(2);
    
	glPushMatrix();
	glScaled(1, -1, 1);
	
	currentColorImage->getTextureReference().bind();
	
	glEnable(GL_DEPTH_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
	//glColorPointer(3, GL_FLOAT, sizeof(ofVec3f), &(pointCloudColors[0].x));
	glTexCoordPointer(2, GL_FLOAT, sizeof(ofVec2f), &(mesh.getTexCoords()[0].x));
	glVertexPointer(3, GL_FLOAT, sizeof(ofVec3f), &(mesh.getVertices()[0].x));
                   
	glDrawArrays(GL_POINTS, 0, mesh.getVertices().size());
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
	glDisable(GL_DEPTH_TEST);
	
	currentColorImage->getTextureReference().unbind();
	
	glPopMatrix();
	
	ofPopStyle();
}

