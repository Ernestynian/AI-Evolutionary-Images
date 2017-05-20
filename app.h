#ifndef APP_H
#define APP_H

#include <opencv2/opencv.hpp>
#include "population.h"

using namespace cv;

class App {
public:
	//App();

	void run();
private:
	void drawImages(Mat image1, Mat image2, int pid);
	char buffer[128];

	const int populations    = 50;
	
	const int triangleCount  = 10;
	const int populationSize = 100;
	
	const char* windowTitle = "AI Evolutionary Images";	
};

#endif /* APP_H */

