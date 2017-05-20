#include "population.h"
#include <stdio.h>//debug

using namespace cv;

//generation of new random population
Population::Population(int population_size, int triangle_count, int cols, int rows){
    this->population_size = population_size;
    this->triangle_count = triangle_count;
    this->cols = cols;
    this->rows = rows;
    
    grades = new double[population_size];
    solutions = new Point**[population_size];
    colo = new Scalar*[population_size];
    images = new Mat[population_size];//(rows, cols, CV_8UC3, Scalar(0, 0, 0));
    
    for(int i = 0; i < population_size; i++){
        solutions[i] = new Point*[triangle_count];
        colo[i] = new Scalar[triangle_count];
        images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
        
        RNG rng(time(NULL));
        for(int j = 0; j< triangle_count; j++){
            
            solutions[i][j] = new Point[3];
            colo[i][j] = Scalar(
                    rng.uniform(0, 255), 
                    rng.uniform(0, 255), 
                    rng.uniform(0, 255));
            
            solutions[i][j][0].x = rng.uniform(0, cols);
            solutions[i][j][0].y = rng.uniform(0, rows);
            solutions[i][j][1].x = rng.uniform(0, cols);
            solutions[i][j][1].y = rng.uniform(0, rows);
            solutions[i][j][2].x = rng.uniform(0, cols);
            solutions[i][j][2].y = rng.uniform(0, rows);
        }
    }
}

void Population::createImages(){
    for(int i = 0; i < population_size; i++){
        images[i] = Mat(rows, cols, CV_8UC3, Scalar(0, 0, 0));
        for(int j = 0; j< triangle_count; j++){
            const Point* ppt[1] = { solutions[i][j] };
            int npt[] = {3};
            fillPoly(images[i], ppt, npt, 1, colo[i][j]);
        }
    }
}

void Population::calculateGrades(Mat& target){
    createImages();
    
    Mat temp;//(rows, cols);
    for(int i = 0; i < population_size; i++){
        temp = target - images[i];
        grades[i] = 1 / sum(temp)[0];
    }
}

Mat Population::topResult(){
    int index = 0;
    double temp = 0.0f;
    for(int i = 0; i < population_size; i++){
        if(grades[i] > temp){
            index = i;
            temp = grades[i];
        }
    }
    
    return images[index];
}

void Population::mutation(){//add probability to even mutate
    RNG rng(time(NULL));
    for(int i = 0; i < population_size; i++){
        for(int j = 0; j < triangle_count; j++){
            for(int k = 0; k < 3; k++){
                int r1 = rng.uniform(-cols/10, cols/10);//that 10 is a parameter, describing scale of mutation
                if(solutions[i][j][k].x + r1 < cols 
                        && solutions[i][j][k].x + r1 >= 0){
                    solutions[i][j][k].x += r1;
                }
                //else
                //    solutions[i][j][k].x -=
                int r2 = rng.uniform(-rows/10, rows/10);//that 10 is a parameter, describing scale of mutation
                if(solutions[i][j][k].y + r1 < rows
                        && solutions[i][j][k].y + r1 >= 0){//<= or < ???
                    solutions[i][j][k].y += r1;
                }
                //else
                //    solutions[i][j][k].x -=
                int r3 = rng.uniform(-255/10,255/10);//same here
                if(colo[i][j][k] + r3 < 255 && colo[i][j][k] + r3 >= 0)
                    colo[i] += r3;
            }
        }
    }
}

void Population::selection(){
    double* cumulatedSums = new double[population_size];
    cumulatedSums[0] = grades[0];
    for(int i = 1; i < population_size; i++){
        cumulatedSums[i] = grades[i] + cumulatedSums[i-1];
    }
    RNG rng(time(NULL));//it makes little errors, as casting will appear
    for(int i = 0; i < population_size; i++){
        int rand = rng.uniform(0,(int)grades[population_size-1]);
        
        int j = 0;
        for(; j < population_size; j++){
            if(cumulatedSums[j] > rand)
                break;
        }
        
        //TO BE CONTINUED
    }
}

void Population::crossover(){
    
}