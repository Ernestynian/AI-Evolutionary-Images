#ifndef RENDERER_H
#define RENDERER_H

#include <opencv2/opencv.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(int width, int height);
	void prepareOpenGL();
	
	void render(Point** v, Scalar* c, int tris, Mat& out);
	
private:
	double width;
	double height;
};

#endif /* RENDERER_H */

