

#include<cmath>

#include<cstdio>
#include <assimp/Importer.hpp>
#include <string>

#include "shaderDemo.h"
#include "offImport/off_io.h"
#include "offImport\common.h"

// include GLEW to access OpenGL 3.3 functions
//#include <GL/glew.h>
#include "glew.h"

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "VSMathlib.h"
#include "VSShaderlib.h"

VSMathLib *vsml;
VSShaderLib shader;

// Camera Position
float camX, camY, camZ;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = -43.0f, beta = 48.0f;
float r = 5.25f;

// Frame counting and FPS computation
long myTime,timebase = 0,frame = 0;
char s[32];

GLuint vao;

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if(h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	vsml->loadIdentity(VSMathLib::PROJECTION);
	vsml->perspective(53.13f, ratio, 0.1f, 1000.0f);
}

// ------------------------------------------------------------
//
// Render stuff
//
graphicData gd;
unsigned int cnt = 0;
void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	vsml->loadIdentity(VSMathLib::VIEW);
	vsml->loadIdentity(VSMathLib::MODEL);
	// set the camera using a function similar to gluLookAt
	vsml->lookAt(camX, camY, camZ, 0,0,0, 0,1,0);
	// use our shader
	glUseProgram(shader.getProgramIndex());
	// send matrices to uniform buffer
	vsml->matricesToGL();
	// render VAO
	glBindVertexArray(vao);
	//glDrawElements(GL_TRIANGLES, gd.nTris, GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, faceCount);
	 //swap buffers
	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch(key) {

		case 27:
			glutLeaveMainLoop();
			break;

		case 'c': 
			printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
			break;

	}

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}

// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX =  - xx + startX;
	deltaY =    yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux *   						       sin(betaAux * 3.14f / 180.0f);

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}

void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

//  uncomment this if not using an idle func
//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//

GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/helloWorld.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/helloWorld.frag");

	// set semantics for the shader variables
	shader.setProgramOutput(0,"outputF");
	shader.setVertexAttribName(VSShaderLib::VERTEX_COORD_ATTRIB, "position");

	shader.prepareProgram();

	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
	
	return(shader.isProgramValid());
}

void importOffModel()
{
	off_io io;
	LoadData ld;
	std::filebuf fb;
	if (fb.open("models\\teapot.off", std::ios::in))
	{
		std::istream m1(&fb);
		io.Load(m1, &ld);
	}
	
	gd.nVertices=0;
	gd.nTris=0;
	int count = 0;
	gd.pVertices = new float[ld.verts.size() * 3];
	for (int i =0; i<ld.verts.size();i++) {
		gd.pVertices[count] = (ld.verts[i])[0]; count++;
		gd.pVertices[count] = (ld.verts[i])[1]; count++;
		gd.pVertices[count] = (ld.verts[i])[2]; count++;
		gd.nVertices++;
	}
	count = 0;
	gd.pIndices = new unsigned int[ld.tris.size() * 3];
	for (int i = 0; i<ld.tris.size(); i++) {
		gd.pIndices[count] = (ld.tris[i])[0]; count++;
		gd.pIndices[count] = (ld.tris[i])[1]; count++;
		gd.pIndices[count] = (ld.tris[i])[2]; count++;
		gd.nTris++;
	}
	std::cout<< gd.nTris;
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void initOpenGL()
{
	importOffModel();
	// set the camera position based on its spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r *   						     sin(beta * 3.14f / 180.0f);

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// create the VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create buffers for our vertex data
	GLuint buffers[2];
	glGenBuffers(2, buffers);

	//vertex coordinates buffer
	glBindBuffer(
		GL_ARRAY_BUFFER,		//  target to which the buffer object is bound
		buffers[0]				//  name of a buffer object
	);
	glBufferData(
		GL_ARRAY_BUFFER, 		// target to which the buffer object is bound
		sizeof(float)*verticeCount * 3,
		vertices,
//	sizeof(float)*gd.nVertices *3, 		// size in bytes of the buffer object's new data store
	//	gd.pVertices, 				// pointer to data, or NULL if no data is to be copied
		GL_STATIC_DRAW);		// usage pattern of the data store, here (STATIC) modified once and used many times.
	glEnableVertexAttribArray(VSShaderLib::VERTEX_COORD_ATTRIB);
	glVertexAttribPointer(
			VSShaderLib::VERTEX_COORD_ATTRIB,		// attribute 0. No particular reason for 0, but must match the layout in the shader.
			4,										// size
			GL_FLOAT,								// type
			GL_FALSE,								// normalized?
			0,										// stride
			(void*)0								// array buffer offset
	);

	//index buffer
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * gd.nTris, gd.pIndices, GL_STATIC_DRAW);

	// unbind the VAO
	glBindVertexArray(0);
}

void initVSL() {
	vsml = VSMathLib::getInstance();
	// tell VSL the uniform block name
	vsml->setUniformBlockName("Matrices");
	// set semantics for the matrix variable
	vsml->setUniformName(VSMathLib::PROJ_VIEW_MODEL, "pvm");
}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char **argv) {

//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA|GLUT_MULTISAMPLE);

	glutInitContextVersion (3, 3);
	glutInitContextProfile (GLUT_CORE_PROFILE );
	glutInitContextFlags(GLUT_DEBUG);

	glutInitWindowPosition(100,100);
	glutInitWindowSize(512,512);
	glutCreateWindow("Simple Shader Demo");

//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);

	glutMouseWheelFunc ( mouseWheel ) ;

//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	/*printf ("Vendor: %s\n", glGetString (GL_VENDOR));
	printf ("Renderer: %s\n", glGetString (GL_RENDERER));
	printf ("Version: %s\n", glGetString (GL_VERSION));
	printf ("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));*/

	if (!setupShaders())
		return(1);

	initOpenGL();

	initVSL();

	//  GLUT main loop
	glutMainLoop();

	return(0);

}

