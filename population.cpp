#include <algorithm>
#include <opencv2/core.hpp>

#include "population.h"


Population::Population(Mat& target, int populationSize, int triangleCount)
: rng(time(NULL)) {
	this->populationSize = populationSize;
	this->triangleCount  = triangleCount;
	this->target = &target;
	this->cols = target.cols;
	this->rows = target.rows;

	renderer = new Renderer(cols, rows);
	renderer->prepareOpenGL(target);
	
	grades      = new uint64[populationSize];
	solutions   = new Point2f**[populationSize];
	colors      = new Scalar*[populationSize];
	
	parentsAmount = floor(populationSize * selectionRate);
	childsAmount  = ceil(populationSize * (1.f - selectionRate));
	selected      = new bool[populationSize];
	p_solutions   = new Point2f**[parentsAmount];
	p_colors      = new Scalar*[parentsAmount];

	normGrades = new NormalizedGrade[populationSize];

	images = new Mat[populationSize];

	for(int i = 0; i < populationSize; i++) {
		solutions  [i] = new Point2f*[triangleCount];
		colors     [i] = new Scalar[triangleCount];
		images     [i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
		
		if (i < parentsAmount) {
			p_solutions[i] = new Point2f*[triangleCount];
			p_colors   [i] = new Scalar[triangleCount];
		}

		for(int j = 0; j < triangleCount; j++) {
			colors[i][j] = Scalar(rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
							  max(rng.uniform(0.3f, 0.8f) 
								* rng.uniform(0.3f, 0.8f), 0.2f));
			
			float x = rng.uniform(0.f, 2.f) - 1.0f;
			float y = rng.uniform(0.f, 2.f) - 1.0f;
			
			float a = 0.0;
			float b = 2.0;
			float c = (b - a) / 2;
			
			solutions[i][j]   = new Point2f[3];
			solutions[i][j][0].x = x + rng.uniform(a, b) - c;
			solutions[i][j][0].y = y + rng.uniform(a, b) - c;
			solutions[i][j][1].x = x + rng.uniform(a, b) - c;
			solutions[i][j][1].y = y + rng.uniform(a, b) - c;
			solutions[i][j][2].x = x + rng.uniform(a, b) - c;
			solutions[i][j][2].y = y + rng.uniform(a, b) - c;
			
			if (i < parentsAmount)
				p_solutions[i][j] = new Point2f[3];
		}
	}
}


Population::~Population() {
	for(int i = 0; i < populationSize; i++) {
		for(int j = 0; j < triangleCount; j++) {
			delete[] solutions[i][j];
		}

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


void Population::fitness() {
	worst = 0;
	best = LLONG_MAX;
	bestIndex = 0;
	
	Mat temp;
	for(int i = 0; i < populationSize; i++) {
		absdiff(images[i], *target, temp);
        temp.convertTo(temp, CV_16UC3); //should be made optional in some way
        temp = temp.mul(temp);
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


uint64 Population::topFitness() {
	Mat temp;
	absdiff(images[bestIndex], *target, temp);
	Scalar s = sum(temp);
	return s[0] + s[1] + s[2];
}


void Population::selection(SelectionType type) {
	if (type == SelectionType::Roulette)
		selectionRoulette();
	else
		selectionBestOnes();
}


void Population::crossover(CrossoverType type) {
	if (type == CrossoverType::Kill)
		// Create new population from parents
		crossoverKill();
	else
		crossoverWithParents();
}


void Population::mutation(MutationType type) {
	if (type = MutationType::Uniform)
		mutationUniform();
	else
		mutationGauss();
}


////////////////////////////////
// GENETIC ALGORITHMS METHODS //
////////////////////////////////


// There are two 1.f because one of the grades is always 0
void Population::selectionRoulette() {	
	// Normalize
	double sum = 0.f;
	for(int i = 0; i < populationSize; ++i) {
		selected[i] = false;
		sum += grades[i] ; // TODO: change to grades[i] * grades[i]
	}

	assert(sum != 0);

	for(int i = 0; i < populationSize; ++i)
		normGrades[i].set((double)(grades[i]) / sum, i);

	// Sort
	std::sort(normGrades, normGrades + populationSize, NormalizedGrade::descending);

	// Calculate accumulated
	for(int i = 1; i < populationSize; ++i)
		normGrades[i].accumulate(normGrades[i - 1].getAccumulated());

	// Choose
	for(int i = 0; i < parentsAmount; i++) {
		double R = rng.uniform(0.f, 1.f);

		int j = 0;
		for(; j < populationSize; ++j) {
			if(!selected[ normGrades[j].getID() ])
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


void Population::selectionBestOnes() {
    // Normalize
	double sum = 0.f;
	for(int i = 0; i < populationSize; ++i) {
		selected[i] = false;
		sum += grades[i]; // TODO: change to grades[i] * grades[i]
	}

	//assert(sum != 0);

	for(int i = 0; i < populationSize; ++i)
		normGrades[i].set((double)(grades[i]) / sum, i);

	// Sort
	std::sort(normGrades, normGrades + populationSize, NormalizedGrade::descending);

	// Choose
	for(int i = 0; i < parentsAmount; i++) {		
		int ID = normGrades[i].getID();
		selected[ID] = true;
		
		for(int j = 0; j < triangleCount; ++j) {
			for(int k = 0; k < 3; ++k) {
				p_solutions[i][j][k] = solutions[ID][j][k];
				p_colors[i][j][k] = colors[ID][j][k];
			}
			p_colors[i][j][3] = colors[ID][j][3];
		}
	}
}


void Population::crossoverKill() {
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
				colors[i][j][k]    = p_colors[src][j][k];
			}
			colors[i][j][3] = p_colors[src][j][3];
		}
	}	
}


void Population::crossoverWithParents() {
	// Better than Kill
	int lastNotSelected = 0;

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
				colors[lastNotSelected][j][k]    = colors[src][j][k];
			}
			colors[lastNotSelected][j][3] = colors[src][j][3];
		}

		lastNotSelected++;
	}
}


void Population::mutationUniform() {
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
	// TODO: seems to be worse, could it be a bad implementation?
	std::mt19937 e2(rd());
	
	for(int i = 0; i < populationSize; i++) {		
		for (int j = 0; j < triangleCount; ++j) {		
			if(rng.uniform(0.0, 1.0) > mutationChance)
				continue;
			
			if(rng.uniform(0, 2)) {
				int k = rng.uniform(0, 3);
				
				if (rng.uniform(0, 2)) {
					std::normal_distribution<float> dist(solutions[i][j][k].x, 0.5);
					float r1 = dist(e2);
					
					solutions[i][j][k].x = r1;
					if (solutions[i][j][k].x > 1.0)
						solutions[i][j][k].x = 1.0;
					else if (solutions[i][j][k].x < -1.0)
						solutions[i][j][k].x = -1.0;
				} else {
					std::normal_distribution<float> dist(solutions[i][j][k].y, 0.5);
					float r1 = dist(e2);
					
					solutions[i][j][k].y = r1;
					if (solutions[i][j][k].y > 1.0)
						solutions[i][j][k].y = 1.0;
					else if (solutions[i][j][k].y < -1.0)
						solutions[i][j][k].y = -1.0;
				}				
			} else {
				int k = rng.uniform(0, 4);

				std::normal_distribution<float> dist(colors[i][j][k], 0.25);
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
