#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GLES2/gl2.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>

// Initialize framebuffer device
int initFramebuffer(fb_var_screeninfo *vinfo)
{
	int fbfd = open("/dev/fb0", O_RDWR);
	if (fbfd == -1)
	{
		perror("Error opening framebuffer device");
		return -1;
	}

	if (ioctl(fbfd, FBIOGET_VSCREENINFO, vinfo))
	{
		perror("Error reading variable information");
		close(fbfd);
		return -1;
	}

	return fbfd;
}

// Shader sources
const GLchar *vertexSource =
	"attribute vec4 position;    \n"
	"void main()                 \n"
	"{                           \n"
	"  gl_Position = position;   \n"
	"}                           \n";

const GLchar *fragmentSource =
	"precision mediump float;                   \n"
	"void main()                                \n"
	"{                                          \n"
	"  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n" // Red color
	"}                                          \n";

GLuint program;

void initShaders()
{
	// Compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Link vertex and fragment shader into a shader program
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glBindAttribLocation(program, 0, "position");
	glLinkProgram(program);

	// Use the program
	glUseProgram(program);
}

// Initialize OpenGL
void initOpenGL()
{
	// Initialize shaders
	initShaders();

	// Set the viewport
	glViewport(0, 0, 640, 480);

	// Set the clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

// Render a frame
void renderFrame()
{
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw a triangle
	GLfloat vertices[] = {
		0.0f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f};

	// Load the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

// Main rendering loop
void renderLoop(EGLDisplay display, EGLSurface surface)
{
	while (true)
	{
		// Rendering operations go here
		renderFrame();
		eglSwapBuffers(display, surface); // Swap the front and back buffer
	}
}

int main()
{
	fb_var_screeninfo vinfo;

	// Initialize the framebuffer device
	int fbfd = initFramebuffer(&vinfo);
	if (fbfd < 0)
	{
		exit(EXIT_FAILURE);
	}

	Display *xDisplay = XOpenDisplay(NULL);
	if (xDisplay == NULL)
	{
		std::cerr << "Failed to open X display\n";
		return -1;
	}

	Window root = DefaultRootWindow(xDisplay);

	// Set up X11 window attributes
	XSetWindowAttributes swa;
	swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask;

	// Create an X11 window
	Window win = XCreateWindow(
		xDisplay, root,
		0, 0, 640, 480, 0,
		CopyFromParent, InputOutput,
		CopyFromParent, CWEventMask,
		&swa);

	XSetWindowAttributes xattr;
	xattr.override_redirect = False;
	XChangeWindowAttributes(xDisplay, win, CWOverrideRedirect, &xattr);

	XWMHints hints;
	hints.input = True;
	hints.flags = InputHint;
	XSetWMHints(xDisplay, win, &hints);

	XMapWindow(xDisplay, win);
	XStoreName(xDisplay, win, "EGL X11 Window");

	// Initialize EGL
	EGLDisplay display = eglGetDisplay((EGLNativeDisplayType)xDisplay);
	eglInitialize(display, NULL, NULL);

	// Specify the desired configuration attributes
	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_NONE};

	// Choose the first configuration that matches
	EGLConfig config;
	EGLint numConfigs;
	eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

	// Create an EGL rendering context
	EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);

	EGLSurface surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)fbfd, NULL);
	eglMakeCurrent(display, surface, surface, context);

	// Initialize OpenGL
	initOpenGL();

	// Enter main rendering loop
	renderLoop(display, surface);

	// Cleanup
	eglDestroySurface(display, surface);
	eglDestroyContext(display, context);
	eglTerminate(display);
	XDestroyWindow(xDisplay, win);
	XCloseDisplay(xDisplay);
	close(fbfd);

	return 0;
}
