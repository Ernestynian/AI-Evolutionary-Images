#ifndef RENDERER_H
#define RENDERER_H

#define GL_GLEXT_PROTOTYPES 1

#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

using namespace cv;

class Renderer {
public:
	Renderer(Mat& original, int width, int height, bool isHardwareAccelerated = true);
	~Renderer();
	
	
	uint64 render(Point2i** v, Scalar* c, int tris);
	void renderImage(Point2i** v, Scalar* c, int tris, Mat& out);
	
private:
	Mat* original;
	bool isHardwareAccelerated;
	
	int width, height;
	float* columnAvgs;
	
	void prepareOpenGL();
	void createShaders();
	uint fragShader0, fragShader1, fragShader2;
	uint p0, p1, p2;
	
	char* textFileRead(const char* fn);
	void printShaderInfoLog(const char* title, uint obj);
	void printProgramInfoLog(const char* title, uint obj);
	
	void renderImageGPU(Point2i** v, Scalar* c, int tris, Mat& out);
	void renderImageCPU(Point2i** v, Scalar* c, int tris, Mat& out);
	
	uint64 renderGPU(Point2i** v, Scalar* c, int tris);
	uint64 renderCPU(Point2i** v, Scalar* c, int tris);
	
	uint originalTexture;
	uint renderedTexture;
	uint framebufferName;
	
	uint diffTexture;
	uint framebuffer2Name;
};

#endif /* RENDERER_H */

