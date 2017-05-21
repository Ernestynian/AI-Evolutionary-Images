#ifndef POPULATION_H
#define POPULATION_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>

#include "normalizedGrade.h"
#include "renderer.h"

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
	
	void createImages();
	Mat topResult();
	unsigned long long topFitness();
	
private:
	Point** copySolution(Point** solution);
	
	const double selectionRate = 0.15;
	const int    mutationChance = 5;
	const int    mutTriChance = 40;

	Renderer* renderer;
	
	Scalar** colors;
	Point*** solutions;
	bool* selected;
	
	int cols, rows;
	int populationSize;
	int triangleCount;
	unsigned long long* grades;
	unsigned long long worst, best;
	int bestIndex;
	Mat* images;
	
	NormalizedGrade* normGrades;
	
	RNG rng;
};

#endif /* POPULATION_H */

