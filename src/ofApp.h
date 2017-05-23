
#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxKinectForWindows2.h"
#include "contours.h"

#define BOUNDING_BOX_SIZE 10

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void exit();
	void onContourFinderParameterChange(ofAbstractParameter& e);
	void keyReleased(int key);

	ofxPanel gui;
	ofParameterGroup kinectParameterGroup;
	ofParameter<float> nearClipping{ "Near clipping", 0, 100, 2750 };
	ofParameter<float> farClipping{ "Far clipping", 0, 1000, 4750 };
	ofParameter<bool> ignoreClipping{ "Ignore clipping", false };
	ofParameter<bool> reverseColors{ "Inverse clipping", false };


	ofParameterGroup shaderParameterGroup;
	ofParameter<bool> setSubtractionActive{ "Activate subtraction", false };
	ofParameter<float> minReturnColor{ "Minimum return color", 0, 0, 255 };
	ofParameter<float> maxReturnColor{ "Maximum return color", 0, 0, 255 };
	ofParameter<bool> clampColor{ "Clamp all color to 255", false };

	ofParameterGroup contourFinderParameterGroup;
	ofParameter<bool> enableContour{ "Enable contour finder", false };
	ofParameter<int> dilate{ "Dilate", 2, 1, 10 };
	ofParameter<int> erode{ "Erode", 1, 1, 10 };
	ofParameter<float> minArea{ "Minimum area", 1000, 1, 5000 };
	ofParameter<float> maxArea{ "Maximum area", 10000, 10000, 500000 };
	ofParameter<float> threshold{ "Threshold", 0, 0, 255 };
	ofParameter<bool> findHoles{ "Find holes", false };


	// Kinect depth data
	ofxKFW2::Device kinect;
	ofTexture kinectDepthTexture;
	ofShortPixels kinectDepthPixels;
	ofPixels kinectDepthPixelsRemapped;

	ofShader kinectRemappingShader;
	ofFbo kinectRemappedFbo;
	ofTexture kinectRemappedTexture;

	ofTexture subtractionTexture;
	ofPixels subtractionPixels;

	// Shader
	ofShader subtractionShader;
	ofTexture subtractionShaderOutput;
	ofFbo subtractionFbo;

	// Contour finder
	ofxCv::ContourFinder contourFinder;
	ofPtr<ofPixels> contourPixels;
	ofImage contourImage;

	bool debug;


	Contours contours;
	//float minArea;
	//float maxArea;
	float blobOpacity;
	bool showInput;
	bool showTracking;
	ofPtr<ofPixels> pixels;
	ofPtr<ofPixels> pixelsBack;
	ofFbo fbo;
	ofTexture tex;
};
