//
//  ofxRGBDRenderSettings.cpp
//  ScreenLabRenderer
//
//  Created by James George on 4/16/12.
//

#include "ofxRGBDRenderSettings.h"
#include "ofxXmlSettings.h"

ofxRGBDRenderSettings::ofxRGBDRenderSettings(){
    mirror = false;
    edgeClip = 5000;
    zThreshold = 5000;
    
    offset = ofVec2f(0,0);
    simplify = 2;
    startFrame = -1;
    endFrame = -1;
    drawPointcloud = true;
    drawWireframe = false;
    drawBackdrop = false;
    
	wireFrameSize = 1.;
    pointSize = 1.;


    fillHoles = false;
    kernelSize = 3;
    iterations = 1;
    
    cameraPosition.resetTransform();
}

void ofxRGBDRenderSettings::saveToXml(string xmlFile){
    ofxXmlSettings saveFile;
	saveFile.addValue("offsetx", offset.x);
    saveFile.addValue("offsety", offset.y);
    saveFile.addValue("simplify", simplify);
    saveFile.addValue("startFrame", startFrame);
    saveFile.addValue("endFrame", endFrame);
    
    saveFile.addValue("drawPointCloud", drawPointcloud);
    saveFile.addValue("drawWireframe", drawWireframe);
    saveFile.addValue("drawBackdrop", drawBackdrop);
    
    saveFile.addValue("wireframeSize", wireFrameSize);
    saveFile.addValue("pointSize", pointSize);
                      
    saveFile.addValue("mirror", mirror);
    saveFile.addValue("edgeClip", edgeClip);
    saveFile.addValue("zThreshold", zThreshold);
    
    saveFile.addValue("fillHoles", fillHoles);
    saveFile.addValue("kernelSize", kernelSize);
    saveFile.addValue("iterations", iterations);

    saveFile.addValue("px", cameraPosition.getPosition().x);
    saveFile.addValue("py", cameraPosition.getPosition().y);
    saveFile.addValue("pz", cameraPosition.getPosition().z);
    
    saveFile.addValue("ox", cameraPosition.getOrientationQuat().asVec4().x);
    saveFile.addValue("oy", cameraPosition.getOrientationQuat().asVec4().y);
    saveFile.addValue("oz", cameraPosition.getOrientationQuat().asVec4().z);
    saveFile.addValue("ow", cameraPosition.getOrientationQuat().asVec4().w);

	saveFile.saveFile(xmlFile);
}

void  ofxRGBDRenderSettings::loadFromXml(string xmlFile){
    ofxXmlSettings loadFile;
    if(loadFile.loadFile(xmlFile)){
        offset.x = loadFile.getValue("offsetx", 0);
        offset.y = loadFile.getValue("offsety", 0);
        simplify = loadFile.getValue("simplify", 2);
        startFrame = loadFile.getValue("startFrame", -1);
        endFrame = loadFile.getValue("endFrame", -1);
        drawPointcloud = loadFile.getValue("drawPointCloud", true);
        drawWireframe = loadFile.getValue("drawWireframe", false);
        drawBackdrop = loadFile.getValue("drawBackdrop", false);
        
        mirror = loadFile.getValue("mirror", false);
        edgeClip = loadFile.getValue("edgeClip", 5000);
        zThreshold = loadFile.getValue("zThreshold", 5000);

        fillHoles = loadFile.getValue("fillHoles", true);
        kernelSize = loadFile.getValue("kernelSize", 3);
        iterations = loadFile.getValue("iterations", 1);
        
        cameraPosition.setPosition(loadFile.getValue("px", 0),
                                   loadFile.getValue("py", 0),
                                   loadFile.getValue("pz", 0));
        
        cameraPosition.setOrientation(ofQuaternion(loadFile.getValue("ox", 0),
                                                   loadFile.getValue("oy", 0),
                                                   loadFile.getValue("oz", 0),
                                                   loadFile.getValue("ow", 0)));
        
        wireFrameSize = loadFile.getValue("wireframeSize", 1.);
        pointSize = loadFile.getValue("pointSize", 1.);
    }
}


void ofxRGBDRenderSettings::applyToRenderer(ofxRGBDRenderer& renderer){
    
	renderer.setSimplification(simplify);
    renderer.xmult = offset.x;
    renderer.ymult = offset.y;
    renderer.edgeCull = edgeClip;
    renderer.farClip = zThreshold;
    renderer.mirror = mirror;
    
    //cout << "applying settings to renderer " << << endl;

}

void ofxRGBDRenderSettings::drawWithRenderer(ofxRGBDRenderer& renderer){
	    
}
