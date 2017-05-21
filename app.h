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
	void drawImages(Mat image1, Mat image2, Mat image3, int pid, unsigned long long fitness);
	char buffer[128];

	Scalar white;
	Point pPos;
	Point fPos;
	
	const int populations    = 5000;
	
	const int triangleCount  = 50;
	const int populationSize = 50;
	
	const char* windowTitle = "AI Evolutionary Images";	
};

#endif /* APP_H */

