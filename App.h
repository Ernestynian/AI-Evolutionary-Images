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
	void drawImages(Mat image1, Mat image2);
	
	const char* windowTitle = "AI Evolutionary Images";
        
        const int triangle_count = 10;
        const int population_size = 100;
};

#endif /* APP_H */

