#include <chrono>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "app.h"

using namespace std;
using namespace std::chrono;


App::App() 
	: white(255,255,255),
	  black(0, 0, 0),
	  pPos(5, 16),
	  fPos(5, 36)
{
	bestFitness = ULONG_MAX;
}


void App::run() {
	Mat input = imread((const char*[]) { "",
		"BitwaPodGrunwaldem.jpg", // 1
		"cat.jpg",		          // 2
		"mona.jpg",		          // 3
		"spongebob.jpg",          // 4
		"saitama.jpg",            // 5
		"scream.jpg",             // 6
        "result.svg.png"          // 7
	}[ 6 ]);
	
	int newWidth = input.cols - (input.cols % 4);
	resize(input, input, Size(newWidth, input.rows), 0, 0, INTER_CUBIC);
	
	worstFitness = (uint64)input.cols * (uint64)input.rows * 3 * 255;
	
	Population population(input);
	population.fitness();

	Mat bestImage;
	
	population.setMutationChance(0.02f, 0.04f);
	
	int i = 0;
	for(;; ++i) {
		if (i % 10 == 0) {
			if (i > 1000)
				population.setMutationChance(0.002f, 0.01f);
			else if (i > 100)
				population.setMutationChance(0.01f, 0.02f);
		}
		
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		population.selection(SelectionType::BestOnes);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		population.crossover(CrossoverType::Kill);
		high_resolution_clock::time_point t3 = high_resolution_clock::now();
		population.mutation(MutationType::Uniform);
		high_resolution_clock::time_point t4 = high_resolution_clock::now();
		
		population.fitness();
		high_resolution_clock::time_point t5 = high_resolution_clock::now();

		/*auto durationSel = duration_cast<microseconds>( t2 - t1 ).count();
		auto durationCro = duration_cast<microseconds>( t3 - t2 ).count();
		auto durationMut = duration_cast<microseconds>( t4 - t3 ).count();
		auto durationCre = duration_cast<microseconds>( t5 - t4 ).count();
		
		cout << "SCM: "
			 << std::setw(3) << durationSel
			 << std::setw(5) << durationCro 
		     << std::setw(4) << durationMut
		     << std::setw(8) << durationCre << "\n";*/
		
		uint64 currentFitness = population.topFitness();
		if (currentFitness < bestFitness) {
			bestFitness = currentFitness;
			population.topResult().copyTo(bestImage);
		}
		
		drawImages(population.topResult(), input, bestImage, i);
		
		// Stop when key is pressed
		if (waitKey(1) == 'q')
			break;
	}
	
	drawImages(population.topResult(), input, bestImage, i);
    population.saveBestAs("result.svg");
	waitKey(0);
}


void App::drawImages(Mat image1, Mat image2, Mat image3, int pid) {
	Mat dst = Mat(image1.rows, image1.cols * 3, CV_8UC3, Scalar(0, 0, 0));

	Mat output = dst(Rect(0, 0, image1.cols, image1.rows));
	image1.copyTo(output);
	output = dst(Rect(image1.cols, 0, image1.cols, image1.rows));
	image2.copyTo(output);
	output = dst(Rect(image1.cols * 2, 0, image1.cols, image1.rows));
	image3.copyTo(output);

	sprintf(buffer, "Gen: %d", pid + 1);
	putText(dst, buffer, pPos, FONT_HERSHEY_PLAIN, 1.0, black, 2, CV_AA);
	putText(dst, buffer, pPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	sprintf(buffer, "Fit: %.2f%%", 100.0 * (1.0 - bestFitness / worstFitness) );
	putText(dst, buffer, fPos, FONT_HERSHEY_PLAIN, 1.0, black, 2, CV_AA);
	putText(dst, buffer, fPos, FONT_HERSHEY_PLAIN, 1.0, white, 1, CV_AA);
	
	imshow(windowTitle, dst);
}

