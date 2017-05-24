
#include "contours.h"

Contours::Contours() {
	needsToUpdate = false;
	//finder.reset(new ofxCv::ContourFinder);
	//finderBack.reset(new ofxCv::ContourFinder);
	front.reset(new ContourBuffer);
	back.reset(new ContourBuffer);

	//back->finder.setMaxArea(500);
	//back->finder.setMinArea(10);
}

void Contours::update(ofPixels *pixels) {
	lock();
	this->pixels = pixels;
	needsToUpdate = true;
	unlock();
}

bool Contours::isFrameNew() {
	bool isnew = hasNewImage;
	hasNewImage = false;
	return isnew;
}

void Contours::threadedFunction() {

	while (isThreadRunning()) {

		if (needsToUpdate) {

			//ofThread::lock();

			if (pixels != NULL && pixels->isAllocated())
				back->finder.findContours(*pixels);

			back->circleCenter.resize(back->finder.size());
			back->circleRadius.resize(back->finder.size());
			for (int i = 0; i<back->finder.size(); i++) {
				back->circleCenter[i] = back->finder.getMinEnclosingCircle(i, back->circleRadius[i]);
			}

			needsToUpdate = false;
			hasNewImage = true;

			swap(front, back);

			//ofThread::unlock();
		}
		else
			ofSleepMillis(1);
	}
}