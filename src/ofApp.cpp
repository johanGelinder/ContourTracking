
#include "ofApp.h"

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424
#define DEPTH_TOTAL DEPTH_WIDTH*DEPTH_HEIGHT
#define CONTOUR_FACTOR 2.024


//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(60);
	ofBackground(0);

	// Gui
	kinectParameterGroup.setName("Kinect parameters");
	kinectParameterGroup.add(nearClipping);
	kinectParameterGroup.add(farClipping);
	kinectParameterGroup.add(ignoreClipping);
	kinectParameterGroup.add(reverseColors);

	//subtractionParameterGroup.setName("Background learning parameters");
	//subtractionParameterGroup.add(bLearnSubtraction);
	//subtractionParameterGroup.add(bResetSubtraction);
	//subtractionParameterGroup.add(bSaveSubtractionTexture);
//	bSaveSubtractionTexture.addListener(this, &ofApp::onSaveSubtractionTexture);
	//subtractionParameterGroup.add(bLoadSubtractionTexture);
	//bLoadSubtractionTexture.addListener(this, &ofApp::onLoadSubtractionTexture);
	//subtractionParameterGroup.add(bLoadSubtractionTextureOnStart);

	////shaderParameterGroup.setName("Background subtraction parameters");
	////shaderParameterGroup.add(setSubtractionActive);
	////shaderParameterGroup.add(minReturnColor);
	////shaderParameterGroup.add(maxReturnColor);
	////shaderParameterGroup.add(clampColor);

	contourFinderParameterGroup.setName("Contour finder parameters");
	contourFinderParameterGroup.add(enableContour);
	contourFinderParameterGroup.add(erode);
	contourFinderParameterGroup.add(dilate);
	contourFinderParameterGroup.add(minArea);
	contourFinderParameterGroup.add(maxArea);
	contourFinderParameterGroup.add(threshold);
	contourFinderParameterGroup.add(findHoles);
	ofAddListener(contourFinderParameterGroup.parameterChangedE(), this, &ofApp::onContourFinderParameterChange);

	gui.setDefaultWidth(400);
	gui.setup();
	gui.setPosition(0, 0);
	gui.add(kinectParameterGroup);
	//gui.add(subtractionParameterGroup);
	//gui.add(shaderParameterGroup);
	gui.add(contourFinderParameterGroup);
	gui.loadFromFile("settings.xml");

	// Before converting
	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();

	kinectDepthPixelsRemapped.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, 1);
	kinectRemappingShader.load("shaders/kinectRemapper");
	kinectRemappedTexture.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	kinectRemappedFbo.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	kinectRemappedFbo.begin();
	ofClear(0);
	kinectRemappedFbo.end();

	// After converting
	kinectDepthTexture.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	subtractionTexture.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	subtractionPixels.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, 1);
	subtractionShaderOutput.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);

	// Shader
	/*subtractionShader.load("shaders/backgroundSubtraction");

	subtractionFbo.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	subtractionFbo.begin();
	ofClear(0);
	subtractionFbo.end();*/

	// Contour finder
	contourPixels.reset(new ofPixels);
	contourPixels->allocate(DEPTH_WIDTH, DEPTH_HEIGHT, GL_R16);
	
	contourImage.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_IMAGE_COLOR);

	debug = false;

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
	

	// Subtraction shader

	//subtractionShader.begin();
	//subtractionShader.setUniformTexture("_depthTexture", kinectRemappedFbo.getTexture(), 1);
	//subtractionShader.setUniformTexture("_subtractionTexture", subtractionTexture, 2);
	//subtractionShader.setUniform1f("_minReturnColor", minReturnColor);
	//subtractionShader.setUniform1f("_maxReturnColor", maxReturnColor);
	//subtractionShader.setUniform1i("_clampColor", clampColor ? 1 : 0);
	//subtractionShader.setUniform1i("_setSubtractionActive", setSubtractionActive ? 1 : 0);
	//subtractionFbo.begin();
	//ofClear(0);
	//subtractionShaderOutput.draw(0, 0);
	//subtractionFbo.end();
	//subtractionShader.end();

	////////////////////////////////////////////////////////////////////////////////////
	// Contour finder
	////////////////////////////////////////////////////////////////////////////////////

	// From texture to image
	//subtractionFbo.readToPixels(contourPixels);



	//--------------------------------------------------------------
	// read the pixels from the remapped fbo
	//--------------------------------------------------------------
	//ofPixels p;
	kinectRemappedFbo.readToPixels(*contourPixels);

	// Erode, and dilate to delete lines
	ofxCv::erode(*contourPixels, *contourPixels, erode);
	ofxCv::dilate(*contourPixels, *contourPixels, dilate);

	//contourFinder.findContours(contourPixels);

	contours.setMinArea(minArea);
	contours.setMaxArea(maxArea);
	contours.update(contourPixels.get());
}

//--------------------------------------------------------------
void ofApp::draw() {




	//ofVec2f sizeWin = ofGetWindowSize();
	//ofVec2f sizeImg(contourPixels->getWidth(), contourPixels->getHeight());
	//ofVec2f ratio = sizeWin / sizeImg;

	//for (int i = 0; i<contours.size(); i++) {

	//	ofPolyline & poly = contours.getPolyline(i);
	//	float radius = 0;
	//	cv::Point2f c = contours.getMinEnclosingCircle(i, radius);
	//	ofVec2f radius2 = radius * ratio;
	//	ofVec2f centroid(c.x * ratio.x, c.y * ratio.y);
	//	cv::Rect rect = contours.getBoundingRect(i);
	//	ofVec4f boundary(rect.x * ratio.x, rect.y * ratio.y, (rect.x + rect.width) * ratio.x, (rect.y + rect.height) * ratio.y);

	//	//if (poly.size() <= contourPoly.size()) {
	//	//	vector<ofPoint> & points = poly.getVertices();
	//	//	ofVec2f *dst = &contourPoly[0];
	//	//	ofVec3f *src = points.data();
	//	//	for (int j = 0; j<points.size(); j++) {
	//	//		dst->x = src->x * ratio.x;
	//	//		dst->y = src->y * ratio.y;
	//	//		dst++;
	//	//		src++;
	//	//	}
	//	//}
	//}

	//if (!debug) return;


	//if (enableContour) {
	/*	ofPushMatrix();
		ofScale(CONTOUR_FACTOR, CONTOUR_FACTOR);
		if (contourPixels.isAllocated()) contourImage.setFromPixels(contourPixels);
		contourImage.draw(0, 0);
		contourFinder.draw();
		ofPopMatrix();*/
//	}

	//if (showTracking) {
		ofPushMatrix();
		//--------------------------------------------------------------
		// if the contour is not empty, then scale it to fit the sceen
		//--------------------------------------------------------------
		if(contourPixels != nullptr)
			ofScale((float)ofGetWindowWidth() / (float)contourPixels->getWidth(), (float)ofGetWindowHeight() / (float)contourPixels->getHeight());

		//--------------------------------------------------------------
		// draw the fbo
		//--------------------------------------------------------------
		ofSetColor(255);
		kinectRemappedFbo.draw(0, 0);

		//--------------------------------------------------------------
		// draw the contour
		//--------------------------------------------------------------
		if(contours.isFrameNew())
			contours.drawContours();

		//--------------------------------------------------------------
		// draw a hollow circle around the contour
		//--------------------------------------------------------------
		ofSetColor(255);
		ofNoFill();
		for (int i = 0; i<contours.size(); i++) {
			float radius;
			cv::Point2f centroid = contours.getMinEnclosingCircle(i, radius);
			ofEllipse(ofPoint(centroid.x, centroid.y), radius * 2, radius * 2);
		}

		ofPopMatrix();

	//}

	ofSetWindowTitle(ofToString(ofGetFrameRate()));

	if(debug)gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

 if (key == 'd') debug = !debug;
}

//--------------------------------------------------------------
void ofApp::exit() {
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



