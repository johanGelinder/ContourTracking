
#include "ofApp.h"

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424
#define DEPTH_TOTAL DEPTH_WIDTH*DEPTH_HEIGHT
#define CONTOUR_FACTOR 2.024

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofBackground(0);

	//----------------------------------------------------------
	// GUI setup
	//----------------------------------------------------------
	kinectParameterGroup.setName("Kinect parameters");
	kinectParameterGroup.add(nearClipping);
	kinectParameterGroup.add(farClipping);
	kinectParameterGroup.add(ignoreClipping);
	kinectParameterGroup.add(reverseColors);

	contourFinderParameterGroup.setName("Contour finder parameters");
	contourFinderParameterGroup.add(enableContour);
	contourFinderParameterGroup.add(erode);
	contourFinderParameterGroup.add(dilate);
	contourFinderParameterGroup.add(minArea);
	contourFinderParameterGroup.add(maxArea);
	//contourFinderParameterGroup.add(threshold);
	//contourFinderParameterGroup.add(findHoles);

	//----------------------------------------------------------
	// add a listener to update the parameters
	//----------------------------------------------------------
	ofAddListener(contourFinderParameterGroup.parameterChangedE(), this, &ofApp::onContourFinderParameterChange);

	gui.setDefaultWidth(400);
	gui.setup();
	gui.setPosition(0, 0);
	gui.add(kinectParameterGroup);
	gui.add(contourFinderParameterGroup);
	gui.loadFromFile("settings.xml");

	//----------------------------------------------------------
	// kinect setup
	//----------------------------------------------------------
	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();

	//----------------------------------------------------------
	// allocate the fbo and load the shader into i
	//----------------------------------------------------------
	kinectDepthPixelsRemapped.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, 1);
	kinectRemappingShader.load("shaders/kinectRemapper");
	kinectRemappedTexture.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	kinectRemappedFbo.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	kinectRemappedFbo.begin();
	ofClear(0);
	kinectRemappedFbo.end();

	//----------------------------------------------------------
	//  allocate the contour pixels
	//----------------------------------------------------------
	contourPixels.reset(new ofPixels);
	contourPixels->allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);

	//----------------------------------------------------------
	// start the contour thread
	//----------------------------------------------------------
	contours.startThread();
}

//--------------------------------------------------------------
void ofApp::update() {

	//----------------------------------------------------------
	// update the kinect
	//----------------------------------------------------------
	kinect.update();
	if (!kinect.isFrameNew()) return;

	//----------------------------------------------------------
	// re-map kinect depth image to make it visible on screen
	//----------------------------------------------------------
	kinectDepthTexture = kinect.getDepthSource()->getTexture();
	kinectRemappingShader.begin();
	kinectRemappingShader.setUniformTexture("_depthTexture", kinectDepthTexture, 1);
	kinectRemappingShader.setUniform1f("_nearClipping", nearClipping);
	kinectRemappingShader.setUniform1f("_farClipping", farClipping);
	kinectRemappingShader.setUniform1i("_reverseColors", reverseColors ? 1 : 0);
	kinectRemappingShader.setUniform1i("_ignoreClipping", ignoreClipping ? 1 : 0);
	kinectRemappedFbo.begin();
	kinectRemappedTexture.draw(0, 0);
	kinectRemappedFbo.end();
	kinectRemappingShader.end();

	//----------------------------------------------------------
	// read the pixels from the remapped fbo
	//----------------------------------------------------------
	kinectRemappedFbo.readToPixels(*contourPixels);

	//----------------------------------------------------------
	// Erode, and dilate to delete lines
	//----------------------------------------------------------
	ofxCv::erode(*contourPixels, *contourPixels, erode);
	ofxCv::dilate(*contourPixels, *contourPixels, dilate);

	//----------------------------------------------------------
	// set the min & max area
	//----------------------------------------------------------
	contours.setMinArea(minArea);
	contours.setMaxArea(maxArea);
	contours.update(contourPixels.get());
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofPushMatrix();
	//----------------------------------------------------------
	// if the contour is not empty, then scale it to fit the sceen
	//----------------------------------------------------------
	if (contourPixels != nullptr)
		ofScale((float)ofGetWindowWidth() / (float)contourPixels->getWidth(), (float)ofGetWindowHeight() / (float)contourPixels->getHeight());

	//----------------------------------------------------------
	// draw the fbo
	//----------------------------------------------------------
	ofSetColor(255);
	kinectRemappedFbo.draw(0, 0);

	//----------------------------------------------------------
	// draw the contour
	//----------------------------------------------------------
	if (contours.isFrameNew())
		contours.drawContours();

	//----------------------------------------------------------
	// draw a hollow circle around the contour
	//----------------------------------------------------------
	ofSetColor(255);
	ofNoFill();
	for (int i = 0; i < contours.size(); i++) {
		float radius;
		cv::Point2f centroid = contours.getMinEnclosingCircle(i, radius);
		ofEllipse(ofPoint(centroid.x, centroid.y), radius * 2, radius * 2);
	}

	ofPopMatrix();
	ofSetWindowTitle(ofToString(ofGetFrameRate()));

	//----------------------------------------------------------
	// display the GUI
	//----------------------------------------------------------
	if (debug)gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

	if (key == 'd') debug = !debug;
}

//--------------------------------------------------------------
void ofApp::exit() {

	//----------------------------------------------------------
	// close the kinect and wait for thread to finnish 
	// before exiting the application
	//----------------------------------------------------------
	kinect.close();

	ofLog() << "Waiting for contours thread to finish";
	contours.waitForThread();
	ofLog() << "Done.";
}

void ofApp::onContourFinderParameterChange(ofAbstractParameter& e) {
	// Find contour
	string parameterName = e.getName();

	if (parameterName == minArea.getName()) contourFinder.setMinArea(minArea);
	else if (parameterName == maxArea.getName()) contourFinder.setMaxArea(maxArea);
	else if (parameterName == threshold.getName()) contourFinder.setThreshold(threshold);
	else if (parameterName == findHoles.getName()) contourFinder.setFindHoles(findHoles);
}



