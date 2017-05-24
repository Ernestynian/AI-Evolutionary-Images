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
	
	void render(Point2f** v, Scalar* c, int tris, uint64* grades);
	void renderGPU(Point2f** v, Scalar* c, int tris, Mat& out);
	
private:
	int width, height;
	
	void createShaders();
	uint fragShader0, fragShader1, fragShader2;
	uint p0, p1, p2;
	
	char* textFileRead(const char* fn);
	void printShaderInfoLog(const char* title, uint obj);
	void printProgramInfoLog(const char* title, uint obj);
	
	void renderGPUx(Point2f** v, Scalar* c, int tris, uint64* grades);
	void renderCPU(Point2f** v, Scalar* c, int tris, Mat& out);
	
	uint originalTexture;
	uint renderedTexture;
	uint framebufferName;
	
	uint diffTexture;
	uint framebuffer2Name;
};

#endif /* RENDERER_H */

