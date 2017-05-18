#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "App.h"



App::App() {
	// read an image
	goalImage = imread("Mona_Lisa.jpg");

	resize(goalImage, goalImage, Size(), 0.5, 0.5);
	
	goalImage.copyTo(randImage);

	imageSize.width  = goalImage.cols;
	imageSize.height = goalImage.rows;
}


void App::run() {
	drawImages(goalImage, randImage);
	waitKey(5000);
}



void App::drawImages(Mat image1, Mat image2) {
	Mat dst = Mat(image1.rows, image1.cols * 2, CV_8UC3, Scalar(0, 0, 0));
	
	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);

	namedWindow(windowTitle);
	imshow(windowTitle, dst);
}


void App::drawRandomPolygon() {
	RNG rng(0xFFFFFFFF);

	int points = rng.uniform(3, 6);
	Point* pt = new Point[points];

	for(int i = 0; i < points; ++i) {
		pt[i].x = rng.uniform(0, imageSize.width);
		pt[i].y = rng.uniform(0, imageSize.height);
	}

	delete[] pt;
}