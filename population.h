#ifndef POPULATION_H
#define POPULATION_H

#include <random>
#include <ctime>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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
	void mutationGauss();
	void fitness(Mat& target);
	
	void createImages();
	Mat topResult();
	unsigned long long topFitness();
	
private:
	Point** copySolution(Point** solution);
	
	const double selectionRate  = 0.15;
	const double mutationChance = 0.01;
	const float  mutationSize   = 0.10;

	Renderer* renderer;
	
	Scalar** colors;
	Point2f*** solutions;
	// parents
	int parentsAmount;
	bool* selected;
	Scalar** p_colors;
	Point2f*** p_solutions;
	
	int cols, rows;
	int populationSize;
	int triangleCount;
	unsigned long long* grades;
	unsigned long long worst, best;
	int bestIndex;
	Mat* images;
	
	NormalizedGrade* normGrades;
	
	RNG rng;
	std::random_device rd;
};

#endif /* POPULATION_H */

