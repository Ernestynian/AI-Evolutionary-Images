#include <cstdio>

#include <stdio.h>
#include <stdlib.h>

#include "renderer.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>


typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool(*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;


// source: https://sidvind.com/wiki/Opengl/windowless
Renderer::Renderer(int width, int height) {
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

	this->width  = width * 0.5;
	this->height = height * 0.5;
	
	prepareOpenGL();
}


void Renderer::prepareOpenGL() {
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);

	glViewport(0, 0, (int)width * 2, (int)height * 2);

	glClearColor(0, 0, 0, 0);

	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
}


void Renderer::render(Point** v, Scalar* c, int tris, Mat& out) {
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	for(int j = 0; j < tris; j++) {
		glColor4f(c[j][0] / 255.0,
				  c[j][1] / 255.0,
				  c[j][2] / 255.0,
				  0.5);

		glVertex3f(v[j][0].x / width  - 1.0,
				   v[j][0].y / height - 1.0, 0);
		glVertex3f(v[j][1].x / width  - 1.0,
				   v[j][1].y / height - 1.0, 0);
		glVertex3f(v[j][2].x / width  - 1.0,
				   v[j][2].y / height - 1.0, 0);
	}
	glEnd();

	glFlush();

	// Copy OpenGL buffer data
	glReadPixels(0, 0, out.cols, out.rows, GL_BGR, GL_UNSIGNED_BYTE, out.data);
	flip(out, out, 0);
}
