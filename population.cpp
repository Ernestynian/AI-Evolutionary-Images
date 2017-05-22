#include <algorithm>
#include <cstdio>
#include <opencv2/core.hpp>

#include "population.h"


Population::Population(int populationSize, int triangleCount, int cols, int rows)
: rng(time(NULL)) {
	this->populationSize = populationSize;
	this->triangleCount = triangleCount;
	this->cols = cols;
	this->rows = rows;

	grades = new unsigned long long[populationSize];
	solutions = new Point2f**[populationSize];
	colors = new Scalar*[populationSize];
	selected = new bool[populationSize];

	normGrades = new NormalizedGrade[populationSize];

	images = new Mat[populationSize]; //(rows, cols, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < populationSize; i++) {
		solutions[i] = new Point2f*[triangleCount];
		colors[i] = new Scalar[triangleCount];
		images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

		for(int j = 0; j < triangleCount; j++) {
			colors[i][j] = Scalar(rng.uniform(0.f, 1.f),
								  rng.uniform(0.f, 1.f),
								  rng.uniform(0.f, 1.f),
								  rng.uniform(0.2f, 1.f));

			solutions[i][j] = new Point2f[3];
			float sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][0].x = rng.uniform(0.f, 1.f) * sign;
			sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][0].y = rng.uniform(0.f, 1.f) * sign;
			sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][1].x = rng.uniform(0.f, 1.f) * sign;
			sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][1].y = rng.uniform(0.f, 1.f) * sign;
			sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][2].x = rng.uniform(0.f, 1.f) * sign;
			sign = rng.uniform(0, 2) ? 1.f : -1.f;
			solutions[i][j][2].y = rng.uniform(0.f, 1.f) * sign;
		}
	}
	
	
	renderer = new Renderer(cols, rows);
}


Population::~Population() {
	for(int i = 0; i < populationSize; i++) {
		for(int j = 0; j < triangleCount; j++)
			delete[] solutions[i][j];

		delete[] solutions[i];
		//delete[] colors[i];
	}

	delete[] grades;
	delete[] solutions;
	delete[] colors;
	delete[] selected;

	delete[] normGrades;

	delete[] images;
	
	delete renderer;
}


void Population::createImages() {
	// could be used as a second trivial method
	
	/*int wh = cols / 2;
	int hh = rows / 2;
	
	Mat empty = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
	Mat overlay = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
	
	double alpha = 0.2;
	
	for(int i = 0; i < populationSize; i++) {
		empty.copyTo(images[i]);
		empty.copyTo(overlay);
		
		for(int j = 0; j < triangleCount; j++) {
			Point p[] = {
				Point(solutions[i][j][0].x * wh + wh, solutions[i][j][0].y * hh + hh),
				Point(solutions[i][j][1].x * wh + wh, solutions[i][j][1].y * hh + hh),
				Point(solutions[i][j][2].x * wh + wh, solutions[i][j][2].y * hh + hh),
			};
			Scalar c = Scalar(colors[i][j][0] * 255.0, colors[i][j][1] * 255.0, colors[i][j][2] * 255.0);
			fillConvexPoly(overlay, p, 3, c);
			
			addWeighted(overlay, alpha, images[i], 1 - alpha, 0, images[i]);
		}
	}*/
	
	for(int i = 0; i < populationSize; i++)
		renderer->render(solutions[i], colors[i], triangleCount, images[i]);
}


void Population::fitness(Mat& target) {
	worst = 0;
	best = LLONG_MAX;
	bestIndex = 0;

	Mat temp;
	for(int i = 0; i < populationSize; i++) {
		absdiff(images[i], target, temp);
		Scalar s = sum(temp);
		grades[i] = s[0] + s[1] + s[2];

		if(grades[i] > worst)
			worst = grades[i];

		if(grades[i] < best) {
			bestIndex = i;
			best = grades[i];
		}
	}

	for(int i = 0; i < populationSize; i++) {
		//printf("%d: %lld\n", i, grades[i]);
		grades[i] = worst - grades[i];
	}
}


Mat Population::topResult() {
	return images[bestIndex];
}


unsigned long long Population::topFitness() {
	return grades[bestIndex] + worst;
}


void Population::selectionStochastic() {
	// TODO
}


void Population::selectionRoulette() {
	// Normalize
	double sum = 0.f;
	for(int i = 0; i < populationSize; ++i) {
		selected[i] = false;
		sum += grades[i];
	}

	//assert(sum != 0);

	for(int i = 0; i < populationSize; ++i)
		normGrades[i].set((double)grades[i] / sum, i);

	// Sort
	std::sort(normGrades, normGrades + populationSize, NormalizedGrade::descending);

	// Calculate accumulated
	for(int i = 1; i < populationSize; ++i)
		normGrades[i].addAccumulated(normGrades[i - 1].getAccumulated());

	// There are two 1.f because one of the grades is always 0
	//for (int i = 0; i < populationSize; ++i)
	//    printf("%d acc: %f\n", i, normGrades[i].getAccumulated());

	// Choose
	int parentsAmount = floor(populationSize * selectionRate);
	for(int i = 0; i < parentsAmount; i++) {
		double R = rng.uniform(0.f, 1.f);

		int j = 0;
		for(; j < populationSize; ++j) {
			if(selected[ normGrades[j].getID() ])
				continue;

			if(normGrades[j].getAccumulated() > R)
				break;
		}

		if(j == populationSize) {
			j = 0;
			while(selected[ normGrades[j].getID() ])
				j++;
		}

		selected[ normGrades[j].getID() ] = true;
	}
}


void Population::crossover() {
	int lastNotSelected = 0;

	int childsAmount = ceil(populationSize * (1.f - selectionRate));
	for(int i = 0; i < childsAmount; i++) {
		int a, b; // parents
		do {
			a = rng.uniform(0, populationSize);
			b = rng.uniform(0, populationSize);
		} while(!selected[a] || !selected[b] || a == b);

		// Find empty slot for crossover child
		while(selected[lastNotSelected])
			lastNotSelected++;

		//int bStart = rng.uniform(0, int(triangleCount * 0.5));
		//int bEnd = rng.uniform(bStart + 1, triangleCount);
		//int bEnd = bStart + triangleCount * 0.5;
		
		//printf("%d is child of %2d %2d (range %2d -> %2d)\n", lastNotSelected, a, b, bStart, bEnd);

		for(int j = 0; j < triangleCount; ++j) {
			int src = rng.uniform(0, 2) ? b : a;

			for(int k = 0; k < 3; ++k) {
				solutions[lastNotSelected][j][k].x = solutions[src][j][k].x;
				solutions[lastNotSelected][j][k].y = solutions[src][j][k].y;

				colors[lastNotSelected][j][k] = colors[src][j][k];
			}
		}

		lastNotSelected++;
	}
}


void Population::mutation() {
	for(int i = 0; i < populationSize; i++) {		
		for (int j = 0; j < triangleCount; ++j) {
			for (int k = 0; k < 3; ++k) {
				if(rng.uniform(0.0, 1.0) > mutationChance)
					continue;
				
				int sign = rng.uniform(0, 2) ? 1 : -1;
				float r1 = rng.uniform(0.0f, mutationSize) * sign;

				solutions[i][j][k].x += r1;

				if(solutions[i][j][k].x > 1.0)
					solutions[i][j][k].x = 1.0;
				else if(solutions[i][j][k].x < -1.0)
					solutions[i][j][k].x = -1.0;
			}
			
			for (int k = 0; k < 3; ++k) {
				if(rng.uniform(0.0, 1.0) > mutationChance)
					continue;
				
				int sign = rng.uniform(0, 2) ? 1 : -1;
				float r1 = rng.uniform(0.0f, mutationSize) * sign;

				solutions[i][j][k].y += r1;

				if(solutions[i][j][k].y > 1.0)
					solutions[i][j][k].y = 1.0;
				else if(solutions[i][j][k].y < -1.0)
					solutions[i][j][k].y = -1.0;
			}
			
			for (int k = 0; k < 4; ++k) {
				if(rng.uniform(0.0, 1.0) > mutationChance)
					continue;
				
				int sign = rng.uniform(0, 2) ? 1 : -1;
				float r1 = rng.uniform(0.0f, mutationSize) * sign;

				colors[i][j][k] += r1;

				if(colors[i][j][k] > 1.0)
					colors[i][j][k] = 1.0;
				else if(colors[i][j][k] < 0.0)
					colors[i][j][k] = 0.0;
			}
		}
	}
}