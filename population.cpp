#include <stdio.h> // debug
#include "population.h"


Population::Population(int populationSize, int triangleCount, int cols, int rows)
	: rng(time(NULL)) {
	this->populationSize = populationSize;
	this->triangleCount = triangleCount;
	this->cols = cols;
	this->rows = rows;

	grades    = new double[populationSize];
	solutions = new Point**[populationSize];
	colors    = new Scalar*[populationSize];
	images    = new Mat[populationSize]; //(rows, cols, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < populationSize; i++) {
		solutions[i] = new Point*[triangleCount];
		colors[i]      = new Scalar[triangleCount];
		images[i]    = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

		RNG rng(time(NULL));
		for(int j = 0; j < triangleCount; j++) {
			colors[i][j]    = Scalar(rng.uniform(0, 255),
									 rng.uniform(0, 255),
									 rng.uniform(0, 255));

			solutions[i][j] = new Point[3];
			solutions[i][j][0].x = rng.uniform(0, cols);
			solutions[i][j][0].y = rng.uniform(0, rows);
			solutions[i][j][1].x = rng.uniform(0, cols);
			solutions[i][j][1].y = rng.uniform(0, rows);
			solutions[i][j][2].x = rng.uniform(0, cols);
			solutions[i][j][2].y = rng.uniform(0, rows);
		}
	}
}


void Population::createImages() {
	for(int i = 0; i < populationSize; i++) {
		images[i] = Mat(rows, cols, CV_8UC3, Scalar(255, 255, 255));
		for(int j = 0; j < triangleCount; j++) {
			const Point* ppt[1] = {solutions[i][j]};
			int npt[] = {3};
			fillPoly(images[i], ppt, npt, 1, colors[i][j]);
		}
	}
}


void Population::fitness(Mat& target) {
	createImages();

	Mat temp; //(rows, cols);
	for(int i = 0; i < populationSize; i++) {
		temp = target - images[i];
		grades[i] = 1 / sum(temp)[0];
	}
}


Mat Population::topResult() {
	int index = 0;
	double best = grades[0];
	
	for(int i = 1; i < populationSize; i++) {
		if(grades[i] > best) {
			index = i;
			best = grades[i];
		}
	}

	return images[index];
}


void Population::selection() {
	double* cumulatedSums = new double[populationSize];
	cumulatedSums[0] = grades[0];
	for(int i = 1; i < populationSize; i++) {
		cumulatedSums[i] = grades[i] + cumulatedSums[i - 1];
	}
	
	for(int i = 0; i < populationSize; i++) {
		int rand = rng.uniform(0, (int)grades[populationSize - 1]);

		int j = 0;
		for(; j < populationSize; j++) {
			if(cumulatedSums[j] > rand)
				break;
		}

		//TO BE CONTINUED
	}
}


void Population::crossover() {

}


void Population::mutation() {
	// TODO: add probability to even mutate
	for(int i = 0; i < populationSize; i++) {
		for(int j = 0; j < triangleCount; j++) {
			for(int k = 0; k < 3; k++) {
				int r1 = rng.uniform(-cols / 10, cols / 10); //that 10 is a parameter, describing scale of mutation
				if(solutions[i][j][k].x + r1 < cols
				&& solutions[i][j][k].x + r1 >= 0) {
					solutions[i][j][k].x += r1;
				}
				//else
				//    solutions[i][j][k].x -=
				int r2 = rng.uniform(-rows / 10, rows / 10); //that 10 is a parameter, describing scale of mutation
				if(solutions[i][j][k].y + r1 < rows
				&& solutions[i][j][k].y + r1 >= 0) {//<= or < ???
					solutions[i][j][k].y += r1;
				}
				//else
				//    solutions[i][j][k].x -=
				int r3 = rng.uniform(-255 / 10, 255 / 10); //same here
				if(colors[i][j][k] + r3 < 255 && colors[i][j][k] + r3 >= 0)
					colors[i] += r3;
			}
		}
	}
}