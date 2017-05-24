#ifndef RENDERER_H
#define RENDERER_H

#define GL_GLEXT_PROTOTYPES 1

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(int width, int height);
	void prepareOpenGL(Mat& original);
	
	void render(Point2f** v, Scalar* c, int tris, Mat& out);
	
private:
	void createShaders();
	uint fragShader0, fragShader1, fragShader2;
	uint p0, p1, p2;
	
	char* textFileRead(const char* fn);
	void printShaderInfoLog(uint obj);
	void printProgramInfoLog(uint obj);
	
	void renderCPU(Point2f** v, Scalar* c, int tris, Mat& out);
	void renderGPU(Point2f** v, Scalar* c, int tris, Mat& out);
	
	uint originalTexture;
	uint renderedTexture;
	uint framebufferName;
};

#endif /* RENDERER_H */

