#include <algorithm>
#include <cstdio>

#include "population.h"


Population::Population(int populationSize, int triangleCount, int cols, int rows)
: rng(time(NULL)) {
	this->populationSize = populationSize;
	this->triangleCount = triangleCount;
	this->cols = cols;
	this->rows = rows;

	grades = new unsigned long long[populationSize];
	solutions = new Point**[populationSize];
	colors = new Scalar*[populationSize];
	selected = new bool[populationSize];

	normGrades = new NormalizedGrade[populationSize];

	images = new Mat[populationSize]; //(rows, cols, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < populationSize; i++) {
		solutions[i] = new Point*[triangleCount];
		colors[i] = new Scalar[triangleCount];
		images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

		for(int j = 0; j < triangleCount; j++) {
			colors[i][j] = Scalar(rng.uniform(0, 255),
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
	/* could be used as a second trivial method
	
	Mat empty = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
	Mat overlay = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
	
	double alpha = 0.2;
	
	for(int i = 0; i < populationSize; i++) {
		empty.copyTo(images[i]);
		empty.copyTo(overlay);
		
		for(int j = 0; j < triangleCount; j++) {
			fillConvexPoly(overlay, solutions[i][j], 3, colors[i][j]);
			
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
		absdiff(target, images[i], temp);
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
	double sum = 0.0;
	for(int i = 0; i < populationSize; ++i) {
		selected[i] = false;
		sum += grades[i];
	}

	if(sum == 0) {
		printf("select: sum was 0\n");
		//return; // FIXME
	}

	for(int i = 0; i < populationSize; ++i)
		normGrades[i].set((double)grades[i] / sum, i);

	// Sort
	std::sort(normGrades, normGrades + populationSize, NormalizedGrade::descending);

	// Calculate accumulated
	for(int i = 1; i < populationSize; ++i)
		normGrades[i].addAccumulated(normGrades[i - 1].getAccumulated());

	// There are two 1.0 because one of the grades is always 0
	//for (int i = 0; i < populationSize; ++i)
	//    printf("%d acc: %f\n", i, normGrades[i].getAccumulated());

	// Choose
	int parentsAmount = floor(populationSize * selectionRate);
	for(int i = 0; i < parentsAmount; i++) {
		double R = rng.uniform(0.0, 1.0);

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

	int childsAmount = ceil(populationSize * (1.0 - selectionRate));
	for(int i = 0; i < childsAmount; i++) {
		int a, b; // parents
		do {
			a = rng.uniform(0, populationSize);
			b = rng.uniform(0, populationSize);
		} while(!selected[a] || !selected[b] || a == b);

		// Find empty slot for crossover child
		while(selected[lastNotSelected])
			lastNotSelected++;

		int bStart = rng.uniform(0, triangleCount);
		int bEnd = rng.uniform(bStart + 1, triangleCount);

		//printf("%d is child of %d %d (range %d -> %d)\n", lastNotSelected, a, b, bStart, bEnd);

		for(int j = 0; j < triangleCount; ++j) {
			int src = (j >= bStart && j < bEnd) ? b : a;

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
		if(rng.uniform(0, 100) >= mutationChance)
			continue;

		for(int j = 0; j < triangleCount; j++) {
			if(rng.uniform(0, 100) >= mutTriChance)
				continue;

			for(int k = 0; k < 3; k++) {
				if(rng.uniform(0, 2)) {
					int r1 = (rng.uniform(1, cols * 2) - cols) / 20;

					if(solutions[i][j][k].x + r1 < cols
					&& solutions[i][j][k].x + r1 > 0)
						solutions[i][j][k].x += r1;

					int r2 = (rng.uniform(1, rows * 2) - rows) / 20;

					if(solutions[i][j][k].y + r2 < rows
					&& solutions[i][j][k].y + r2 > 0)
						solutions[i][j][k].y += r2;
				} else {
					int r3 = (rng.uniform(1, 255 * 2) - 255) / 20;

					if(colors[i][j][k] + r3 < 255
					&& colors[i][j][k] + r3 > 0)
						colors[i][j][k] += r3;
				}
			}
		}
	}
}