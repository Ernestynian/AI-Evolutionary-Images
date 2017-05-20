#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "app.h"


App::App() 
	: white(255,255,255),
	  pPos(5, 16),
	  fPos(5, 36)
{
	
}


void App::run() {
	Mat input = imread((const char*[]){
		"MonaLisa.jpg",
		"Cat.jpg",
	}[0]);

	Population population(populationSize, triangleCount,
						  input.cols, input.rows);

	population.fitness(input);

	int i = 0;
	for(; i < populations; ++i) {
		population.selectionRoulette();
		population.crossover();
		population.mutation();
		
		population.fitness(input);

		drawImages(input, population.topResult(), i, population.topFitness());

		// Stop when key is pressed
		if (waitKey(1) == 'q')
			break;
	}
	
	drawImages(input, population.topResult(), i, population.topFitness());
	waitKey(0);
}


void App::drawImages(Mat image1, Mat image2, int pid, unsigned long long fitness) {
	Mat dst = Mat(image1.rows, image1.cols * 2, CV_8UC3, Scalar(0, 0, 0));

	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);

	sprintf(buffer, "P: %d", pid + 1);
	putText(dst, buffer, pPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	sprintf(buffer, "F: %lld", fitness);
	putText(dst, buffer, fPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	imshow(windowTitle, dst);
}

