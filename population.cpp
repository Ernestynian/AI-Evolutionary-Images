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
	
	parentsAmount = floor(populationSize * selectionRate);
	selected = new bool[populationSize];
	p_solutions = new Point2f**[parentsAmount];
	p_colors = new Scalar*[parentsAmount];

	normGrades = new NormalizedGrade[populationSize];

	images = new Mat[populationSize];

	for(int i = 0; i < populationSize; i++) {
		solutions[i] = new Point2f*[triangleCount];
		colors[i] = new Scalar[triangleCount];
		images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
		
		if (i < parentsAmount) {
			p_solutions[i] = new Point2f*[triangleCount];
			p_colors[i] = new Scalar[triangleCount];
		}

		for(int j = 0; j < triangleCount; j++) {
			colors[i][j] = Scalar(rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
								  rng.uniform(0.2f, 1.f));
			
			// TODO: center point of spawn, spread them
			//float x = rng.uniform(0.f, 2.f) - 1.0f;
			//float y = rng.uniform(0.f, 2.f) - 1.0f;
			
			if (i < parentsAmount)
				p_solutions[i][j] = new Point2f[3];
			
			solutions[i][j] = new Point2f[3];
			solutions[i][j][0].x = rng.uniform(0.f, 2.f) - 1.0f;
			solutions[i][j][0].y = rng.uniform(0.f, 2.f) - 1.0f;
			solutions[i][j][1].x = rng.uniform(0.f, 2.f) - 1.0f;
			solutions[i][j][1].y = rng.uniform(0.f, 2.f) - 1.0f;
			solutions[i][j][2].x = rng.uniform(0.f, 2.f) - 1.0f;
			solutions[i][j][2].y = rng.uniform(0.f, 2.f) - 1.0f;
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
		/*for(int x = 0; x < temp.rows; ++x) {
			uchar* p = temp.ptr<uchar>(x);
			for (int y = 0; y < temp.cols * temp.channels(); ++y) {
				p[y] *= p[y];
			}
		}*/
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


// There are two 1.f because one of the grades is always 0
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

	// Choose
	//int parentsAmount = floor(populationSize * selectionRate);
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
		
		int ID = normGrades[j].getID();
		selected[ID] = true;
		
		for(j = 0; j < triangleCount; ++j) {
			for(int k = 0; k < 3; ++k) {
				p_solutions[i][j][k] = solutions[ID][j][k];
				p_colors[i][j][k] = colors[ID][j][k];
			}
			p_colors[i][j][3] = colors[ID][j][3];
		}
	}
}


void Population::crossover() {
	// create new population from parents
	for(int i = 0; i < populationSize; i++) {
		int a, b; // parents
		do {
			a = rng.uniform(0, parentsAmount);
			b = rng.uniform(0, parentsAmount);
		} while(a == b);

		// Find empty slot for crossover child
		for(int j = 0; j < triangleCount; ++j) {
			int src = rng.uniform(0, 2) ? b : a;

			for(int k = 0; k < 3; ++k) {
				solutions[i][j][k] = p_solutions[src][j][k];

				colors[i][j][k] = p_colors[src][j][k];
			}
			
			colors[i][j][3] = p_colors[src][j][3];
		}
	}
}


/*void Population::crossover() {
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

		for(int j = 0; j < triangleCount; ++j) {
			int src = rng.uniform(0, 2) ? b : a;

			for(int k = 0; k < 3; ++k) {
				solutions[lastNotSelected][j][k] = solutions[src][j][k];

				colors[lastNotSelected][j][k] = colors[src][j][k];
			}
			
			colors[lastNotSelected][j][3] = colors[src][j][3];
		}

		lastNotSelected++;
	}
}*/


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


void Population::mutationGauss() {
	std::mt19937 e2(rd());
	
	for(int i = 0; i < populationSize; i++) {		
		for (int j = 0; j < triangleCount; ++j) {		
			if(rng.uniform(0.0, 1.0) > mutationChance)
				continue;
			
			if(rng.uniform(0, 2)) {
				int k = rng.uniform(0, 3);
				
				if (rng.uniform(0, 2)) {
					std::normal_distribution<float> dist(solutions[i][j][k].x, 1.0);
					float r1 = dist(e2);
					
					solutions[i][j][k].x = r1;
					if (solutions[i][j][k].x > 1.0)
						solutions[i][j][k].x = 1.0;
					else if (solutions[i][j][k].x < -1.0)
						solutions[i][j][k].x = -1.0;
				} else {
					std::normal_distribution<float> dist(solutions[i][j][k].y, 1.0);
					float r1 = dist(e2);
					
					solutions[i][j][k].y = r1;
					if (solutions[i][j][k].y > 1.0)
						solutions[i][j][k].y = 1.0;
					else if (solutions[i][j][k].y < -1.0)
						solutions[i][j][k].y = -1.0;
				}				
			} else {
				int k = rng.uniform(0, 4);

				std::normal_distribution<float> dist(colors[i][j][k], 0.5);
				float r1 = dist(e2);

				colors[i][j][k] = r1;
				
				if (colors[i][j][k] > 1.0)
					colors[i][j][k] = 1.0;
				else if (colors[i][j][k] < 0.0)
					colors[i][j][k] = 0.0;
			}
		}
	}
}