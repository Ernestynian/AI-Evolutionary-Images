#include <cstdio>
#include <stdlib.h>
#include <malloc.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "renderer.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>


typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool(*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;



Renderer::Renderer(Mat& original, int width, int height, bool isHardwareAccelerated):
	width(width), height(height) {
	this->original = &original;
	columnAvgs = nullptr;
	
	this->isHardwareAccelerated = isHardwareAccelerated;
	if (isHardwareAccelerated) {
		// source: https://sidvind.com/wiki/Opengl/windowless
		static int visual_attribs[] = {
			None
		};
		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			None
		};

		Display* dpy = XOpenDisplay(0);
		int fbcount = 0;
		GLXFBConfig* fbc = NULL;
		GLXContext ctx;
		GLXPbuffer pbuf;

		if(!(dpy = XOpenDisplay(0))) {
			fprintf(stderr, "Failed to open display\n");
			exit(1);
		}

		// get framebuffer configs, any is usable (might want to add proper attribs)
		if(!(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount))) {
			fprintf(stderr, "Failed to get FBConfig\n");
			exit(1);
		}

		// get the required extensions
		glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
		glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB((const GLubyte *)"glXMakeContextCurrent");
		if(!(glXCreateContextAttribsARB && glXMakeContextCurrentARB)) {
			fprintf(stderr, "missing support for GLX_ARB_create_context\n");
			XFree(fbc);
			exit(1);
		}

		// create a context using glXCreateContextAttribsARB
		if(!(ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs))) {
			fprintf(stderr, "Failed to create opengl context\n");
			XFree(fbc);
			exit(1);
		}

		// create temporary pbuffer
		int pbuffer_attribs[] = {
			GLX_PBUFFER_WIDTH, width,
			GLX_PBUFFER_HEIGHT, height,
			None
		};
		pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);

		XFree(fbc);
		XSync(dpy, False);

		// try to make it the current context 
		if(!glXMakeContextCurrent(dpy, pbuf, pbuf, ctx)) {
			// some drivers does not support context without default framebuffer
			//  so fallback on using the default window.

			if(!glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx)) {
				fprintf(stderr, "failed to make current\n");
				exit(1);
			}
		}

		columnAvgs = new float[width];
		
		prepareOpenGL();
		createShaders();
	}
}


Renderer::~Renderer() {
	if (columnAvgs != nullptr)
		delete[] columnAvgs;
		
	if (isHardwareAccelerated) {
		for (int i = 0; i < 3; ++i) {
			glDeleteShader(fragShader[i]);
			glDeleteProgram(p[i]);
		}
	}
}


void Renderer::prepareOpenGL() {
	glewInit();
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);
	
	glViewport(0, 0, width, height);
	
	glClearColor(0, 0, 0, 0);

	//glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glOrtho(0, width, 0, height, 0, 1);

	/////////////////////////
	
	Mat temp;
	cv::flip(*original, temp, 0);
	glGenTextures(1, &originalTexture);
	glBindTexture(GL_TEXTURE_2D, originalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
				0, GL_BGR, GL_UNSIGNED_BYTE, original->ptr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	/////////////////////////
	
	framebufferName = 0;
	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	
	//GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	
	/////////////////////////
	
	framebuffer2Name = 0;
	glGenFramebuffers(1, &framebuffer2Name);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2Name);
	
	//GLuint renderedTexture;
	glGenTextures(1, &diffTexture);

	glBindTexture(GL_TEXTURE_2D, diffTexture);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffTexture, 0);

	glDrawBuffers(1, DrawBuffers);
	
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}


void Renderer::createShaders() {
	assert(GLEW_ARB_fragment_shader);
	
	const char* fragmentShaderCode[] = {
		textFileRead("render.frag"),
		textFileRead("diff.frag"),
		textFileRead("sum.frag")
	};	
	
	for (int i = 0; i < 3; ++i) {
		fragShader[i] = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader[i], 1, &fragmentShaderCode[i], nullptr);
		glCompileShader(fragShader[i]);
		printShaderInfoLog(fragmentShaderCode[i], fragShader[i]);
		
		p[i] = glCreateProgram();
		glAttachShader(p[i], fragShader[i]);
		glLinkProgram(p[i]);
		printProgramInfoLog(fragmentShaderCode[i], p[i]);
	}
}


uint64 Renderer::render(Point2i** v, Scalar* c, int tris) {
	if (isHardwareAccelerated)
		return renderGPU(v, c, tris);
	else
		return renderCPU(v, c, tris);
}


void Renderer::renderImage(Point2i** v, Scalar* c, int tris, Mat& out) {
	if (isHardwareAccelerated)
		renderImageGPU(v, c, tris, out);
	else
		renderImageCPU(v, c, tris, out);
}


uint64 Renderer::renderGPU(Point2i** v, Scalar* c, int tris) {
	///////// RENDER IMAGE TO TEXTURE /////////
	
	glDisable(GL_TEXTURE_2D);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(p[0]);
	
	glBegin(GL_TRIANGLES);
	for(int j = 0; j < tris; j++) {
		glColor4f(c[j][0], c[j][1], c[j][2], c[j][3]);

		glVertex2i(v[j][0].x, v[j][0].y);
		glVertex2i(v[j][1].x, v[j][1].y);
		glVertex2i(v[j][2].x, v[j][2].y);
	}
	glEnd();
	
	///////// CALCULATE PIXELS DIFFERENCE /////////
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer2Name);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(p[1]);
	GLuint tex1ID = glGetUniformLocation(p[1], "render");
	GLuint tex2ID = glGetUniformLocation(p[1], "orig");
	glUniform1i(tex1ID, 0);
	glUniform1i(tex2ID, 1);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, originalTexture);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(0,     0);
		glTexCoord2i(0, 1); glVertex2i(0,     height);
		glTexCoord2i(1, 1); glVertex2i(width, height);
		glTexCoord2i(1, 0); glVertex2i(width, 0);
	glEnd();
	
	///////// CALCULATE SUMS OF COLUMNS /////////
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(p[2]);
	GLuint texID = glGetUniformLocation(p[2], "diff");
	GLuint imhID = glGetUniformLocation(p[2], "imageHeight");
	glUniform1i(texID, 0);
	glUniform1i(imhID, height);
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffTexture);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(0,     0);
		glTexCoord2i(0, 1); glVertex2i(0,     1);
		glTexCoord2i(1, 1); glVertex2i(width, 1);
		glTexCoord2i(1, 0); glVertex2i(width, 0);
	glEnd();
	
	///////// CALCULATE FITNESS /////////
	float sum = 0.0;
	glReadPixels(0, 0, width, 1, GL_RED, GL_FLOAT, columnAvgs);
	for (int i = 0; i < width; ++i)
		sum += columnAvgs[i];
	
	return uint64(sum * width * height * 2550);
}


void Renderer::renderImageGPU(Point2i** v, Scalar* c, int tris, Mat& out) {
	// glScalef won't work - tested
	// no need to flip matrix either

	glDisable(GL_TEXTURE_2D);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glUseProgram(p[0]);
	
	glBegin(GL_TRIANGLES);
	for(int j = 0; j < tris; j++) {
		glColor4f(c[j][0], c[j][1], c[j][2], c[j][3]);

		glVertex2i(v[j][0].x, v[j][0].y);
		glVertex2i(v[j][1].x, v[j][1].y);
		glVertex2i(v[j][2].x, v[j][2].y);
	}
	glEnd();
	
	glReadPixels(0, 0, out.cols, out.rows, GL_BGR, GL_UNSIGNED_BYTE, out.data);
}


uint64 Renderer::renderCPU(Point2i** v, Scalar* c, int tris) {
	Mat out = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));
	Mat overlay = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < tris; i++) {
		Point p[] = {
			Point(v[i][0].x, v[i][0].y),
			Point(v[i][1].x, v[i][1].y),
			Point(v[i][2].x, v[i][2].y),
		};
		Scalar cf = Scalar(c[i][0] * 255.0, c[i][1] * 255.0, c[i][2] * 255.0);
		fillConvexPoly(overlay, p, 3, cf);

		float alpha = c[i][3];// * 0.1;
		addWeighted(overlay, alpha, out, 1.0 - alpha, 0, out);
	}
	
	absdiff(out, *original, overlay);
	overlay.convertTo(overlay, CV_16UC3); // should be made optional in some way
	overlay = overlay.mul(overlay);
	Scalar s = sum(overlay);
	
	return s[0] + s[1] + s[2];
}


void Renderer::renderImageCPU(Point2i** v, Scalar* c, int tris, Mat& out) {
	Mat overlay = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));

	for(int i = 0; i < tris; i++) {
		Point p[] = {
			Point(v[i][0].x, v[i][0].y),
			Point(v[i][1].x, v[i][1].y),
			Point(v[i][2].x, v[i][2].y),
		};
		Scalar cf = Scalar(c[i][0] * 255.0, c[i][1] * 255.0, c[i][2] * 255.0);
		fillConvexPoly(overlay, p, 3, cf);

		float alpha = c[i][3];// * 0.1;
		addWeighted(overlay, alpha, out, 1.0 - alpha, 0, out);
	}
}


void Renderer::printShaderInfoLog(const char* title, GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n%s\n", title, infoLog);
        free(infoLog);
    }
}


void Renderer::printProgramInfoLog(const char* title, GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n%s\n", title, infoLog);
        free(infoLog);
    }
}


char* Renderer::textFileRead(const char* fn) {
	FILE *fp;
	char *content = NULL;

	int f,count;
	f = open(fn, O_RDONLY);

	count = lseek(f, 0, SEEK_END);

	close(f);

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			
			fclose(fp);
		}
	}
	return content;
}
