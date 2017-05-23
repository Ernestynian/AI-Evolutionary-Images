#include <chrono>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "app.h"

using namespace std;
using namespace std::chrono;


App::App() 
	: white(255,255,255),
	  pPos(5, 16),
	  fPos(5, 36)
{
	
}


void App::run() {
	Mat input = imread((const char*[]) {
		"MonaLisa.jpg",
		"Cat.jpg",
		"mona.jpg"
	}[ 0 ]);

    //should be made conditional in some way
	//worstFitness = (long long)input.cols * (long long)input.rows * 3 * 255 * 255;
	worstFitness = (long long)input.cols * (long long)input.rows * 3 * 255;
	
	Population population(populationSize, triangleCount,
						  input.cols, input.rows);

	population.createImages();
	population.fitness(input);

	uint64 bestFitness = ULONG_MAX;
	Mat bestImage;
	
	int i = 0;
	for(; i < populations; ++i) {
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		population.selection(SelectionType::Roulette);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		population.crossover(CrossoverType::Kill);
		high_resolution_clock::time_point t3 = high_resolution_clock::now();
		population.mutation(MutationType::Uniform);
		high_resolution_clock::time_point t4 = high_resolution_clock::now();
		
		population.createImages();
		high_resolution_clock::time_point t5 = high_resolution_clock::now();
		population.fitness(input);
		high_resolution_clock::time_point t6 = high_resolution_clock::now();

		auto durationSel = duration_cast<microseconds>( t2 - t1 ).count();
		auto durationCro = duration_cast<microseconds>( t3 - t2 ).count();
		auto durationMut = duration_cast<microseconds>( t4 - t3 ).count();
		auto durationCre = duration_cast<microseconds>( t5 - t4 ).count();
		auto durationFit = duration_cast<microseconds>( t6 - t5 ).count();
		
		/*cout << "SCM: "
			 << std::setw(3) << durationSel
			 << std::setw(5) << durationCro 
		     << std::setw(4) << durationMut
		     << std::setw(8) << durationCre
		     << std::setw(6) << durationFit << "\n";*/
		
		uint64 currentFitness = population.topFitness(input);
		if (currentFitness < bestFitness) {
			bestFitness = currentFitness;
			population.topResult().copyTo(bestImage);
		}
		
		drawImages(input, bestImage, population.topResult(), i, bestFitness);
		
		// Stop when key is pressed
		if (waitKey(1) == 'q')
			break;
	}
	
	drawImages(input, bestImage, population.topResult(), i, bestFitness);
	waitKey(0);
}


void App::drawImages(Mat image1, Mat image2, Mat image3, int pid, uint64 fitness) {
	Mat dst = Mat(image1.rows, image1.cols * 3, CV_8UC3, Scalar(0, 0, 0));

	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);
	output = dst(Rect(image1.cols * 2, 0, image1.cols, image1.rows));
	image3.copyTo(output);

	sprintf(buffer, "P: %d", pid + 1);
	putText(dst, buffer, pPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	sprintf(buffer, "F: %.2f%%", 100.0 * (1.0 - (double)fitness / worstFitness) );
	putText(dst, buffer, fPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	imshow(windowTitle, dst);
}

