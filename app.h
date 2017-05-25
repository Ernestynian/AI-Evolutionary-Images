#ifndef APP_H
#define APP_H

#include <opencv2/opencv.hpp>
#include "population.h"

using namespace cv;

class App {
public:
	App();

	void run();
private:
	void drawImages(Mat image1, Mat image2, Mat image3, int pid);
	char buffer[128];

	Scalar white, black;
	Point pPos;
	Point fPos;
	
	double worstFitness;
	double bestFitness;
	
	const char* windowTitle = "AI Evolutionary Images";	
};

#endif /* APP_H */

