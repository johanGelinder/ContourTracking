
#pragma once

#include "ofMain.h"
#include "ofxCv.h"

typedef struct {
	ofxCv::ContourFinder finder;
	vector<cv::Point> circleCenter;
	vector<float> circleRadius;
} ContourBuffer;

class Contours : public ofThread {
public:

	Contours();

	void update(ofPixels *pixels);
	bool isFrameNew();

	void threadedFunction();

	void setMinArea(float minArea) { back->finder.setMinArea(minArea); }
	void setMaxArea(float maxArea) { back->finder.setMaxArea(maxArea); }
	int size() { return front->finder.size(); }
	ofPolyline& getPolyline(unsigned int i) { return front->finder.getPolyline(i); }
	cv::Point2f getMinEnclosingCircle(unsigned int i, float& radius) { radius = front->circleRadius[i]; return front->circleCenter[i]; }
	cv::Rect getBoundingRect(unsigned int i) { return front->finder.getBoundingRect(i); }

	void drawContours() { front->finder.draw(); }

private:
	ofPtr<ContourBuffer> front;
	ofPtr<ContourBuffer> back;

	//ofPtr<ofxCv::ContourFinder> finder;
	//ofPtr<ofxCv::ContourFinder> finderBack;
	ofPixels *pixels;
	bool needsToUpdate;
	bool hasNewImage;

	//vector<cv::Point> circleCenter;
	//vector<float> circleRadius;
};