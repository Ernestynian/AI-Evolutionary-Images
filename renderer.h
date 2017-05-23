#ifndef RENDERER_H
#define RENDERER_H

#define GL_GLEXT_PROTOTYPES 1

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(int width, int height);
	void prepareOpenGL(int width, int height);
	
	void render(Point2f** v, Scalar* c, int tris, Mat& out);
	
private:
	void renderCPU(Point2f** v, Scalar* c, int tris, Mat& out);
	void renderGPU(Point2f** v, Scalar* c, int tris, Mat& out);	
};

#endif /* RENDERER_H */
