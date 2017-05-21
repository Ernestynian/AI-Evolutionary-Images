#ifndef RENDERER_H
#define RENDERER_H

#include <opencv2/opencv.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(int width, int height);
	void prepareOpenGL(int width, int height);
	
	void render(Point** v, Scalar* c, int tris, Mat& out);
	
private:
};

#endif /* RENDERER_H */

