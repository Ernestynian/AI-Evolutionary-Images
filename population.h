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


enum SelectionType {
	Roulette,
	BestOnes
};

enum CrossoverType {
	Kill,
	WithParents
};

enum MutationType {
	Uniform,
	Gauss
};

class Population {
public:
	Population(Mat& target);
	~Population();
	
	void selection(SelectionType type);
	void crossover(CrossoverType type);
	void mutation(MutationType type);
	
	void fitness();
	
	Mat topResult();
	uint64 topFitness();
    
    void saveBestAs(const char* name);
	
private:
	void selectionBestOnes();
	void selectionRoulette();
	
	void crossoverKill();
	void crossoverWithParents();
	
	void mutationUniform();
	void mutationGauss();
	
	Point** copySolution(Point** solution);
	
	// TODO: dynamic changes
	const double selectionRate  = 0.15;
	const double mutationChance = 0.01;
	const float  mutationSize   = 0.10;

	const int triangleCount     = 150;
	const int populationSize    = 50;
	int cols, rows;
	
	Renderer* renderer;
	Mat* target;
	Mat bestImage;
	
	// GENES
	Scalar** colors;
	Point2f*** solutions;
	bool* selected;
	Scalar** p_colors;
	Point2f*** p_solutions;
	
	int parentsAmount, childsAmount;
	
	uint64* grades;
	uint64 worst, best;
	int bestIndex;
	Mat* images;
	
	NormalizedGrade* normGrades;
	
	RNG rng;
	std::random_device rd;
};

#endif /* POPULATION_H */

