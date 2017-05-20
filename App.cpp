#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "App.h"

void App::run() {
	namedWindow(windowTitle);
        
        Mat input = imread("Mona_Lisa.jpg");
	
        //generacja populacji początkowej
        Population population(population_size, 
                triangle_count, 
                input.cols, 
                input.rows);
        
        population.calculateGrades(input);
        
	for (int x = 0; x < 50; ++x) {//liczba pokoleń 
            //reprodukcja
            population.selection();
            //krzyżowanie
            population.crossover();
            //mutacja
            population.mutation();
            //ocena - long long
            population.calculateGrades(input);
            
            drawImages(input, population.topResult());
            
            waitKey();
	}
}



void App::drawImages(Mat image1, Mat image2) {
	Mat dst = Mat(image1.rows, image1.cols * 2, CV_8UC3, Scalar(0, 0, 0));
	
	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);

	imshow(windowTitle, dst);
}

