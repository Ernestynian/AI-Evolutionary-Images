#ifndef RENDERER_H
#define RENDERER_H

#include <opencv2/opencv.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(int width, int height);
	void prepareOpenGL();
	
	void render(Point2f** v, Scalar* c, int tris, Mat& out);
	
private:
	int width;
	int height;
};

#endif /* RENDERER_H */

