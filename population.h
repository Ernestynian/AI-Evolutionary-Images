#ifndef POPULATION_H
#define POPULATION_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <time.h>

using namespace cv;

class Population {
    Scalar** colo;
    Point*** solutions;
    int cols, rows;
    int population_size;
    int triangle_count;
    double* grades;
    Mat* images;
    
    Point** copySolution(Point** solution);
public:
    Population(int population_size, int triangle_count, int cols, int rows);
    //Mat asImage(Size size, int id);
    void createImages();
    Mat topResult();
    void calculateGrades(Mat& target);
    void mutation();
    void selection();
    void crossover();
};

#endif /* POPULATION_H */

