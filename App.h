#ifndef APP_H
#define APP_H

#include <opencv2/opencv.hpp>
using namespace cv;

class App {
public:
	App();
	
	void run();
private:
	void drawImages(Mat image1, Mat image2);
	void drawRandomPolygon();
	
	Size2i imageSize;
	
	Mat goalImage;
	Mat randImage;
	
	const char* windowTitle = "AI Evolutionary Images";
};

#endif /* APP_H */

