#include <math.h>
#include <iostream>
#include <GL/glut.h>  

GLfloat moveX = 0.0f;
GLfloat moveY = 0.0f;
GLfloat moveZ = 0.0f;

GLfloat rotX = 0.0f;
GLfloat rotY = 0.0f;
GLfloat rotZ = 0.0f;

GLfloat camY = 0.0f;
GLfloat camX = 0.0f;
GLfloat camZ = 0.0f;

bool showWireframe = false;
bool showAxes = true;
bool showGrid = true;

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void drawAxes() {
	glBegin(GL_LINES);

	glLineWidth(1.5);

	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(-25.0, 0.0, 0.0);
	glVertex3f(25.0, 0.0, 0.0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -25.0, 0.0);
	glVertex3f(0.0, 25.0, 0.0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, -25);
	glVertex3f(0.0, 0.0, 25);

	glEnd();
}

void drawGrid() {
	GLfloat ext = 20.0f;
	GLfloat step = 1.0f;
	GLfloat yGrid = 0.0f;
	GLint line;

	glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_LINES);
	for (line = -ext; line <= ext; line += step) {
		glVertex3f(line, yGrid, ext);
		glVertex3f(line, yGrid, -ext);

		glVertex3f(ext, yGrid, line);
		glVertex3f(-ext, yGrid, line);
	}
	glEnd();
}

void drawCube(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat l, GLfloat h) {
	glBegin(GL_QUADS);
	// TOP
	glColor3f(1.0, 1.0, 0.5);
	glVertex3f(x, y + h, z);
	glVertex3f(x, y + h, z + l);
	glVertex3f(x + w, y + h, z + l);
	glVertex3f(x + w, y + h, z);

	// BOTTOM
	glColor3f(0.5, 1.0, 1.0);
	glVertex3f(x, y, z);
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y, z + l);
	glVertex3f(x, y, z + l);

	// FRONT
	glColor3f(1.0, 0.5, 0.5);
	glVertex3f(x, y, z + l);
	glVertex3f(x + w, y, z + l);
	glVertex3f(x + w, y + h, z + l);
	glVertex3f(x, y + h, z + l);

	// BACK
	glColor3f(1.0f, 0.5f, 1.0f);
	glVertex3f(x, y, z);
	glVertex3f(x, y + h, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x + w, y, z);

	// LEFT
	glColor3f(0.5, 1.0, 0.5);
	glVertex3f(x, y, z);
	glVertex3f(x, y, z + l);
	glVertex3f(x, y + h, z + l);
	glVertex3f(x, y + h, z);

	// RIGHT
	glColor3f(0.5, 0.5, 1.0);
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x + w, y + h, z + l);
	glVertex3f(x + w, y, z + l);
	glEnd();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	gluLookAt(10.0 + camX, 10.0 + camY, 10.0 + camZ, 0, 0, 0, 0, 1.0, 0);

	glTranslatef(moveX, moveY, moveZ);
	glRotatef(rotX, 1.0f, 0.0f, 0.0f);
	glRotatef(rotY, 0.0f, 1.0f, 0.0f);
	glRotatef(rotZ, 0.0f, 0.0f, 1.0f);

	if (showAxes) {
		drawAxes();
	}

	if (showGrid) {
		drawGrid();
	}

	glPopMatrix();
	glutSwapBuffers();
}

void keyboardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_UP) {
		moveZ += 1;
	}
	else if (key == GLUT_KEY_DOWN) {
		moveZ -= 1;
	}
	else if (key == GLUT_KEY_LEFT) {
		moveX += 1;
	}
	else if (key == GLUT_KEY_RIGHT) {
		moveX -= 1;
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'w') {
		camY += 0.5;
	}
	else if (key == 's') {
		camY -= 0.5;
	}
	else if (key == 'a') {
		rotY += 5.0;
	}
	else if (key == 'd') {
		rotY -= 5.0;
	}
	else if (key == '2') {
		moveY += 1;
	}
	else if (key == '8') {
		moveY -= 1;
	}
	else if (key == 'c') {
		if (showWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		showWireframe = !showWireframe;
	}
	else if (key == 'x') {
		showAxes = !showAxes;
	}
	else if (key == 'g') {
		showGrid = !showGrid;
	}
	else if (key == 'r') {
		camY = 0;
		rotY = 0;
		moveY = 0;
		moveX = 0;
		moveZ = 0;
	}

	glutPostRedisplay();
}

void changeSize(GLsizei w, GLsizei h) {
	glViewport(0, 0, w, h);
	GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120.0, aspect_ratio, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(150, 150);

	glutCreateWindow("Straddle Carrier");

	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);

	init();
	glutMainLoop();

	return 0;
}