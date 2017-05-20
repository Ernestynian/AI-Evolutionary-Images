#include <algorithm>
#include <stdio.h> // debug

#include "population.h"

Population::Population(int populationSize, int triangleCount, int cols, int rows)
: rng(time(NULL)) {
    this->populationSize = populationSize;
    this->triangleCount = triangleCount;
    this->cols = cols;
    this->rows = rows;

    grades = new long long[populationSize];
    solutions = new Point**[populationSize];
    colors = new Scalar*[populationSize];
    selected = new bool[populationSize];

    normGrades = new NormalizedGrade[populationSize];

    images = new Mat[populationSize]; //(rows, cols, CV_8UC3, Scalar(0, 0, 0));

    for (int i = 0; i < populationSize; i++) {
        solutions[i] = new Point*[triangleCount];
        colors[i] = new Scalar[triangleCount];
        images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));

        RNG rng(time(NULL));
        for (int j = 0; j < triangleCount; j++) {
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
}

Population::~Population() {
    for (int i = 0; i < populationSize; i++) {
        for (int j = 0; j < triangleCount; j++)
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
}

void Population::createImages() {
    for (int i = 0; i < populationSize; i++) {
        images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
        for (int j = 0; j < triangleCount; j++) {
            const Point * ppt[1] = {solutions[i][j]};
            int npt[] = {3};
            fillPoly(images[i], ppt, npt, 1, colors[i][j]);
        }
    }
}

void Population::fitness(Mat& target) {
    createImages();

    Mat temp; //(rows, cols);
    for (int i = 0; i < populationSize; i++) {
//        temp = target - images[i];
//        
//        temp = abs(temp);
//        imshow("temp", temp);
        grades[i] = 0;
        for(int j = 0; j < rows; j++)
        {
            for(int k = 0; k < cols; k++)
            {
                grades[i] += abs(target.at<Vec3b>(j,k)[0] - images[i].at<Vec3b>(j,k)[0]); 
                grades[i] += abs(target.at<Vec3b>(j,k)[1] - images[i].at<Vec3b>(j,k)[1]); 
                grades[i] += abs(target.at<Vec3b>(j,k)[2] - images[i].at<Vec3b>(j,k)[2]);
            }
        }
        grades[i] = LLONG_MAX - grades[i];
//                grades[i] = sum(temp)[0] + sum(temp)[1] + sum(temp)[2];//
    }
}

Mat Population::topResult() {
    int index = 0;
    long long best = grades[0];

    for (int i = 1; i < populationSize; i++) {
        if (grades[i] < best) {
            index = i;
            best = grades[i];
        }
    }
    printf("%lld\n", grades[index]);
    return images[index];
}

void Population::selectionStochastic() {
    double* cumulatedSums = new double[populationSize];
    cumulatedSums[0] = grades[0];
    for (int i = 1; i < populationSize; i++) {
        cumulatedSums[i] = grades[i] + cumulatedSums[i - 1];
    }

    for (int i = 0; i < populationSize; i++) {
        int rand = rng.uniform(0, (int) grades[populationSize - 1]);

        int j = 0;
        for (; j < populationSize; j++) {
            if (cumulatedSums[j] > rand)
                break;
        }

        //TO BE CONTINUED
    }
}


// Roulette Wheel Selection

void Population::selectionRoulette() {
    // Normalize
    double sum = 0.0;
    for (int i = 0; i < populationSize; ++i) {
        selected[i] = false;
        sum += grades[i];
    }

    for (int i = 0; i < populationSize; ++i)
        normGrades[i].set(grades[i] / sum, i);

    // Sort
    std::sort(normGrades, normGrades + populationSize, NormalizedGrade::descending);

    // Calculate accumulated
    for (int i = 1; i < populationSize; ++i)
        normGrades[i].addAccumulated(normGrades[i - 1].getAccumulated());

    for (int i = 0; i < populationSize; ++i)
        printf("%d acc: %f\n", i, normGrades[i].getAccumulated());

    // Choose
    for (int i = 0; i < populationSize / 2; i++) { // TODO: change populationSize / 2
        int j = 0;
        do {
            double R = rng.uniform(0.0, 1.0);

            for (j = 0; j < populationSize; ++j) {
                if (selected[ normGrades[j].getID() ])
                    continue;

                if (normGrades[j].getAccumulated() >= R)
                    break;
            }
        } while (j == 10);

        selected[ normGrades[j].getID() ] = true;
        //printf("%d (%d)\n", j, normGrades[j].getID());
    }
}

void Population::crossover() {
    int lastNotSelected = 0;

    for (int i = 0; i < populationSize / 2; i++) {
        int a, b; // parents
        do {
            a = rng.uniform(0, populationSize);
            b = rng.uniform(0, populationSize);
        } while (!selected[a] || !selected[b] || a == b);

        // Find empty slot for crossover child
        while (selected[lastNotSelected])
            lastNotSelected++;

        int bStart = rng.uniform(0, triangleCount);
        int bEnd = rng.uniform(bStart, triangleCount);

        //printf("%d is child of %d %d (range %d -> %d)\n", lastNotSelected, a, b, bStart, bEnd);

        for (int j = 0; j < triangleCount; ++j) {
            int src = (j >= bStart && j < bEnd) ? b : a;

            for (int k = 0; k < 3; ++k) {
                solutions[lastNotSelected][j][k].x = solutions[src][j][k].x;
                solutions[lastNotSelected][j][k].y = solutions[src][j][k].y;

                colors[lastNotSelected][j][k] = colors[src][j][k];
            }
        }

        lastNotSelected++;
    }
}

void Population::mutation() {
    // TODO: add probability to even mutate
    for (int i = 0; i < populationSize; i++) {
        for (int j = 0; j < triangleCount; j++) {
            for (int k = 0; k < 3; k++) {
                int r1 = ((rng.uniform(1, cols*2) - cols)/2)/10; //that 10 is a parameter, describing scale of mutation
                if (solutions[i][j][k].x + r1 < cols
                        && solutions[i][j][k].x + r1 >= 0) {
                    solutions[i][j][k].x += r1;
                }
                //else
                //    solutions[i][j][k].x -=
                int r2 = ((rng.uniform(1, rows*2) - rows)/2)/10; //that 10 is a parameter, describing scale of mutation
                if (solutions[i][j][k].y + r2 < rows
                        && solutions[i][j][k].y + r2 >= 0) {//<= or < ???
                    solutions[i][j][k].y += r2;
                }
                //else
                //    solutions[i][j][k].x -=
                int r3 = ((rng.uniform(1, 255*2) - 255)/2)/10; //same here
                if (colors[i][j][k] + r3 < 255 && colors[i][j][k] + r3 >= 0)
                    colors[i][j][k] += r3;
            }
        }
    }
}