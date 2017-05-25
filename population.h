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


enum class SelectionType {
	Roulette,
	BestOnes,
	Random
};

enum class CrossoverType {
	Kill,
	WithParents,
	Random
};

enum class MutationType {
	Uniform,
	Gauss,
	Random
};

class Population {
public:
	Population(Mat& target);
	~Population();
	
	void setMutationChance(float min, float max);
	
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
	
	double selectionRate      = 0.15;
	double mutationChance     = 0.01;
	float  mutationSize       = 0.10;

	const int triangleCount   = 250;
	const int populationSize  = 50;
	int cols, rows;
	
	Renderer* renderer;
	Mat* target;
	Mat bestImage;
	uint64 bestFitness;
	
	// GENES
	Scalar** colors;
	Point2i*** solutions;
	bool* selected;
	Scalar** p_colors;
	Point2i*** p_solutions;
	
	int parentsAmount, childsAmount;
	
	uint64* grades;
	uint64 worst, best;
	int bestIndex;
	
	NormalizedGrade* normGrades;
	
	RNG rng;
	std::random_device rd;
};

#endif /* POPULATION_H */

