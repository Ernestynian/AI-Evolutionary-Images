#ifndef POPULATION_H
#define POPULATION_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>

#include "normalizedGrade.h"

using namespace cv;

class Population {
public:
	Population(int population_size, int triangle_count, int cols, int rows);
	~Population();
	
	void selectionStochastic();
	void selectionRoulette();
	void crossover();
	void mutation();
	void fitness(Mat& target);
	
	//Mat asImage(Size size, int id);
	void createImages();
	Mat topResult();
	
private:
	Point** copySolution(Point** solution);
	
	Scalar** colors;
	Point*** solutions;
	bool* selected;
	
	int cols, rows;
	int populationSize;
	int triangleCount;
	double* grades;
	Mat* images;
	
	NormalizedGrade* normGrades;
	
	RNG rng;
};

#endif /* POPULATION_H */

