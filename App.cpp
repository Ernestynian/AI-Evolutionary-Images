#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "App.h"



App::App() : rng(0xFFFFFFFF) {
	// read an image
	goalImage = imread("Mona_Lisa.jpg");

	resize(goalImage, goalImage, Size(), 0.5, 0.5);
	
	goalImage.copyTo(blankImage);
	blankImage = cv::Scalar(255, 255, 255);

	imageSize.width  = goalImage.cols;
	imageSize.height = goalImage.rows;
}


void App::run() {
	namedWindow(windowTitle);
	
	for (int x = 0; x < 100; ++x) {
		blankImage.copyTo(randImage);
		for (int i = 0; i < 50; ++i)
			drawRandomPolygon(randImage);
		drawImages(randImage, goalImage);
		if (waitKey(1) < 255)
			break;
	}
	
	waitKey();
}



void App::drawImages(Mat image1, Mat image2) {
	Mat dst = Mat(image1.rows, image1.cols * 2, CV_8UC3, Scalar(0, 0, 0));
	
	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);

	imshow(windowTitle, dst);
}


void App::drawRandomPolygon(Mat& img) {
	int points = rng.uniform(3, 6);
	Point* pts = new Point[points];
	int* npts  = new int[2];

	for(int i = 0; i < points; ++i) {
		pts[i].x = rng.uniform(0, imageSize.width);
		pts[i].y = rng.uniform(0, imageSize.height);
	}
	
	npts[0] = points;
	
	Scalar color = Scalar(rng.uniform(0, 255),
						  rng.uniform(0, 255),
						  rng.uniform(0, 255));
	
	cv::Mat roi;
	img.copyTo(roi);
	fillPoly(roi, (const Point**)&pts, npts, 1, color);
	cv::addWeighted(img, 0.5, roi, 1.0 - 0.5, 0.0, img);
	
	delete[] npts;
	delete[] pts;
}