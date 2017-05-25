#include <algorithm>
#include <opencv2/core.hpp>

#include "population.h"
#include "svgExporter.h"


Population::Population(Mat& target)
: rng(time(NULL)) {
	this->target = &target;
	this->cols = target.cols;
	this->rows = target.rows;
	
	renderer = new Renderer(target, cols, rows);
	
	grades      = new uint64[populationSize];
	solutions   = new Point2i**[populationSize];
	colors      = new Scalar*[populationSize];
	
	parentsAmount = floor(populationSize * selectionRate);
	childsAmount  = ceil(populationSize * (1.f - selectionRate));
	selected      = new bool[populationSize];
	p_solutions   = new Point2i**[parentsAmount];
	p_colors      = new Scalar*[parentsAmount];

	normGrades = new NormalizedGrade[populationSize];

	bestImage = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < populationSize; i++) {
		solutions  [i] = new Point2i*[triangleCount];
		colors     [i] = new Scalar[triangleCount];
		
		if (i < parentsAmount) {
			p_solutions[i] = new Point2i*[triangleCount];
			p_colors   [i] = new Scalar[triangleCount];
		}

		for(int j = 0; j < triangleCount; j++) {
			colors[i][j] = Scalar(rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
								  rng.uniform(0.0f, 1.f),
							  max(rng.uniform(0.3f, 0.8f) 
								* rng.uniform(0.3f, 0.8f), 0.2f));
			
			int a = 0;
			int w = cols;
			int h = rows;
			
			int x = rng.uniform(a + w / 8, w - w / 8);
			int y = rng.uniform(a + h / 8, h - h / 8);			
			
			solutions[i][j]   = new Point2i[3];
			solutions[i][j][0].x = x + rng.uniform(a, w) - w / 2;
			solutions[i][j][0].y = y + rng.uniform(a, h) - h / 2;
			solutions[i][j][1].x = x + rng.uniform(a, w) - w / 2;
			solutions[i][j][1].y = y + rng.uniform(a, h) - h / 2;
			solutions[i][j][2].x = x + rng.uniform(a, w) - w / 2;
			solutions[i][j][2].y = y + rng.uniform(a, h) - h / 2;
			
			if (i < parentsAmount)
				p_solutions[i][j] = new Point2i[3];
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
	
	delete renderer;
}


void Population::fitness() {
	worst = 0;
	best = LLONG_MAX;
	bestIndex = 0;
	
	for(int i = 0; i < populationSize; i++) {
		grades[i] = renderer->render(solutions[i], colors[i], triangleCount);

		if(grades[i] > worst)
			worst = grades[i];

		if(grades[i] < best) {
			bestIndex = i;
			best = grades[i];
		}
	}

	for(int i = 0; i < populationSize; i++)
		grades[i] = worst - grades[i];
	
	renderer->renderImage(solutions[bestIndex], colors[bestIndex], triangleCount, bestImage);
	Mat temp;
	absdiff(bestImage, *target, temp);
	Scalar s = sum(temp);
	bestFitness = s[0] + s[1] + s[2];
}


void Population::setMutationChance(float min, float max) {
	mutationChance = rng.uniform(min, max);
}


void Population::selection(SelectionType type) {
	if (type == SelectionType::Random) {
		if (rng.uniform(0, 2))
			selectionRoulette();
		else
			selectionBestOnes();
	} else if (type == SelectionType::Roulette)
		selectionRoulette();
	else
		selectionBestOnes();
}


void Population::crossover(CrossoverType type) {
	if (type == CrossoverType::Random) {
		if (rng.uniform(0, 2))
			crossoverKill();
		else
			crossoverWithParents();
	} else if (type == CrossoverType::Kill)
		crossoverKill();
	else
		crossoverWithParents();
}


void Population::mutation(MutationType type) {
	if (type == MutationType::Random) {
		if (rng.uniform(0, 2))
			mutationUniform();
		else
			mutationGauss();
	} else if (type == MutationType::Uniform)
		mutationUniform();
	else
		mutationGauss();
}


Mat Population::topResult() {
	return bestImage;
}


uint64 Population::topFitness() {
	return bestFitness;
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
		sum += grades[i];
	}

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
		sum += grades[i];
	}

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
	// Create new population from parents
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
				int r1 = rng.uniform(0, int(cols * mutationSize));
				
				solutions[i][j][k].x += r1 * sign;

				if(solutions[i][j][k].x > cols * 1.2)
					solutions[i][j][k].x = cols * 1.2;
				else if(solutions[i][j][k].x < cols * -0.2)
					solutions[i][j][k].x = cols * -0.2;
			}
			
			for (int k = 0; k < 3; ++k) {
				if(rng.uniform(0.0, 1.0) > mutationChance)
					continue;
				
				int sign = rng.uniform(0, 2) ? 1 : -1;
				int r1 = rng.uniform(0, int(rows * mutationSize));

				solutions[i][j][k].y += r1 * sign;

				if(solutions[i][j][k].y > rows * 1.2)
					solutions[i][j][k].y = rows * 1.2;
				else if(solutions[i][j][k].y < rows * -0.2)
					solutions[i][j][k].y = rows * -0.2;
			}
			
			for (int k = 0; k < 4; ++k) {
				if(rng.uniform(0.0, 1.0) > mutationChance)
					continue;
				
				int sign = rng.uniform(0, 2) ? 1 : -1;
				float r1 = rng.uniform(0.0f, mutationSize);
				
				colors[i][j][k] += r1 * sign;

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
					std::normal_distribution<float> dist(solutions[i][j][k].x, cols / 2);
					
					solutions[i][j][k].x = dist(e2);
					
					if(solutions[i][j][k].x > cols * 1.2)
						solutions[i][j][k].x = cols * 1.2;
					else if(solutions[i][j][k].x < cols * -0.2)
						solutions[i][j][k].x = cols * -0.2;
				} else {
					std::normal_distribution<float> dist(solutions[i][j][k].y, rows / 2);
					
					solutions[i][j][k].y = dist(e2);
					
					if(solutions[i][j][k].y > rows * 1.2)
						solutions[i][j][k].y = rows * 1.2;
					else if(solutions[i][j][k].y < rows * -0.2)
						solutions[i][j][k].y = rows * -0.2;
				}				
			} else {
				int k = rng.uniform(0, 4);

				std::normal_distribution<float> dist(colors[i][j][k], 0.5);

				colors[i][j][k] = dist(e2);
				
				if (colors[i][j][k] > 1.0)
					colors[i][j][k] = 1.0;
				else if (colors[i][j][k] < 0.0)
					colors[i][j][k] = 0.0;
			}
		}
	}
}


void Population::saveBestAs(const char* name) {
    saveTrianglesToSvg(name, solutions[bestIndex], colors[bestIndex], triangleCount, cols, rows);
}
