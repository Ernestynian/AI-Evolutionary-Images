#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "app.h"


void App::run() {
	Mat input = imread((const char*[]){
		"MonaLisa.jpg",
		"Cat.jpg",
	}[0]);

	Population population(populationSize, triangleCount,
						  input.cols, input.rows);

	population.fitness(input);

	for(int i = 0; i < populations; ++i) {
		population.selection();
		population.crossover();
		population.mutation();
		
		population.fitness(input);

		drawImages(input, population.topResult(), i);

		// Stop when key is pressed
		if (waitKey(1000) == 'q')
			break;
	}
}


void App::drawImages(Mat image1, Mat image2, int pid) {
	Mat dst = Mat(image1.rows, image1.cols * 2, CV_8UC3, Scalar(0, 0, 0));

	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);

	sprintf(buffer, "Population: %d", pid + 1);
	putText(dst, buffer, cvPoint(5, 16), 
		FONT_HERSHEY_PLAIN, 1.0, cvScalar(255,255,255), 1, CV_AA);
	
	imshow(windowTitle, dst);
}

