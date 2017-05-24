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

#define RENDER_GPU

Renderer::Renderer(int width, int height) {
#ifdef RENDER_GPU
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
#endif
}


void Renderer::prepareOpenGL(Mat& original) {
	glewInit();
	
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);

	glPixelStorei(GL_PACK_ALIGNMENT, (original.step & 3) ? 1 : 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, original.step / original.elemSize());
	
	glViewport(0, 0, original.cols, original.rows);
	
	glClearColor(0, 0, 0, 0);

	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	/////////////////////////
	
	/*Mat temp;
	cv::flip(original, temp, 0);
	glGenTextures(1, &originalTexture);
	glBindTexture(GL_TEXTURE_2D, originalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, original.cols, original.rows,
				0, GL_BGR, GL_UNSIGNED_BYTE, original.ptr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/
	
	/////////////////////////
	
	framebufferName = 0;
	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, original.cols, original.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	
	createShaders();
}


void Renderer::createShaders() {
	assert(GLEW_ARB_fragment_shader);
	
	const char* fragmentShaderCode = textFileRead("diff.frag");
	fragShader1 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader1, 1, &fragmentShaderCode, nullptr);
	glCompileShader(fragShader1);
	
	printShaderInfoLog(fragShader1);
	
	const char* fragmentShaderCode2 = textFileRead("sum.frag");
	fragShader2 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader2, 1, &fragmentShaderCode2, nullptr);
	glCompileShader(fragShader2);
	
	printShaderInfoLog(fragShader2);
	
	p1 = glCreateProgram();
	p2 = glCreateProgram();

	glAttachShader(p1, fragShader1);
	glAttachShader(p2, fragShader2);

	glLinkProgram(p1);
	glLinkProgram(p2);
	
	printProgramInfoLog(p1);
	printProgramInfoLog(p2);
}



void Renderer::render(Point2f** v, Scalar* c, int tris, Mat& out) {

#ifdef RENDER_GPU
	renderGPU(v, c, tris, out);
#else
	renderCPU(v, c, tris, out);
#endif
}


void Renderer::renderGPU(Point2f** v, Scalar* c, int tris, Mat& out) {
	// glScalef won't work - tested
	// no need to flip matrix either

	glDisable(GL_TEXTURE_2D);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBegin(GL_TRIANGLES);
	for(int j = 0; j < tris; j++) {
		glColor4f(c[j][0], c[j][1], c[j][2], c[j][3]);

		glVertex2f(v[j][0].x, v[j][0].y);
		glVertex2f(v[j][1].x, v[j][1].y);
		glVertex2f(v[j][2].x, v[j][2].y);
	}
	glEnd();
	
	///////// SECOND PASS /////////
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	GLuint texID = glGetUniformLocation(p1, "tex");
	glUniform1i(texID, 0);
	glUseProgram(p1);	
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebufferName);
	//glBindTexture(GL_TEXTURE_2D, originalTexture);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2f(-1.0, -1.0);
		glTexCoord2i(0, 1); glVertex2f(-1.0,  1.0);
		glTexCoord2i(1, 1); glVertex2f( 1.0,  1.0);
		glTexCoord2i(1, 0); glVertex2f( 1.0, -1.0);
	glEnd();
	
	// Copy OpenGL buffer data
	glReadPixels(0, 0, out.cols, out.rows, GL_BGR, GL_UNSIGNED_BYTE, out.data);
}


void Renderer::renderCPU(Point2f** v, Scalar* c, int tris, Mat& out) {
	int wh = out.rows / 2;
	int hh = out.cols / 2;

	Mat empty = Mat(out.rows, out.cols, CV_8UC3, Scalar(0, 0, 0));
	Mat overlay = Mat(out.rows, out.cols, CV_8UC3, Scalar(0, 0, 0));

	empty.copyTo(out);
	empty.copyTo(overlay);

	for(int i = 0; i < tris; i++) {
		Point p[] = {
			Point(v[i][0].x * wh + wh, v[i][0].y * hh + hh),
			Point(v[i][1].x * wh + wh, v[i][1].y * hh + hh),
			Point(v[i][2].x * wh + wh, v[i][2].y * hh + hh),
		};
		Scalar cf = Scalar(c[i][0] * 255.0, c[i][1] * 255.0, c[i][2] * 255.0);
		fillConvexPoly(overlay, p, 3, cf);

		addWeighted(overlay, c[i][3], out, 1.0 - c[i][3], 0, out);
	}
}


void Renderer::printShaderInfoLog(GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}


void Renderer::printProgramInfoLog(GLuint obj) {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0) {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
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