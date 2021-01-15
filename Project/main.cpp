#include <math.h>
#include <iostream>
#include <GL/glut.h>  

#define PI 3.1415927

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

void drawCube(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l) {
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
	glColor3f(1.0, 0.5, 1.0);
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

void drawCabin(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l) {
	// SIDE 1
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0, 1.0, 0.5);
	glVertex3f(x + l, y, z);
	glVertex3f(x + l, y + h, z);
	glVertex3f(x + l, y + h, z + l);
	glVertex3f(x + l, y + (h / 3), z + l);
	glVertex3f(x + l, y, z + (l * 3 / 4));
	glEnd();

	// SIDE 2
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.5, 1.0, 1.0);
	glVertex3f(x, y, z);
	glVertex3f(x, y, z + (l * 3 / 4));
	glVertex3f(x, y + (h / 3), z + l);
	glVertex3f(x, y + h, z + l);
	glVertex3f(x, y + h, z);
	glEnd();

	glBegin(GL_QUADS);
	// TOP
	glColor3f(1.0, 0.5, 0.5);
	glVertex3f(x, y + h, z);
	glVertex3f(x, y + h, z + l);
	glVertex3f(x + w, y + h, z + l);
	glVertex3f(x + w, y + h, z);

	// BACK
	glColor3f(1.0f, 0.5f, 1.0f);
	glVertex3f(x, y, z);
	glVertex3f(x, y + h, z);
	glVertex3f(x + w, y + h, z);
	glVertex3f(x + w, y, z);

	// FRONT-TOP
	glColor3f(0.5, 1.0, 0.5);
	glVertex3f(x, y + (h / 3), z + l);
	glVertex3f(x + w, y + (h / 3), z + l);
	glVertex3f(x + w, y + h, z + l);
	glVertex3f(x, y + h, z + l);

	// FRONT-BOTTOM
	glColor3f(0.5, 0.5, 1.0);
	glVertex3f(x, y + (h / 3), z + l);
	glVertex3f(x, y, z + (l * 3 / 4));
	glVertex3f(x + w, y, z + (l * 3 / 4));
	glVertex3f(x + w, y + (h / 3), z + l);

	// BOTTOM
	glColor3f(1.0, 0.5, 0.5);
	glVertex3f(x, y, z);
	glVertex3f(x + w, y, z);
	glVertex3f(x + w, y, z + (l * 3 / 4));
	glVertex3f(x, y, z + (l * 3 / 4));
	glEnd();
}

void drawCylinder(GLfloat radius, GLfloat height) {
	GLfloat y = 0.0;
	GLfloat z = 0.0;
	GLfloat angle = 0.0;
	GLfloat angle_stepsize = 0.1;

	glColor3f(1.0, 0.5, 0.5);
	// TUBE
	glBegin(GL_QUAD_STRIP);
	angle = 2 * PI;
	while (angle >= 0.0) {
		y = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(height, y, z);
		glVertex3f(0.0, y, z);
		angle = angle - angle_stepsize;
	}
	glVertex3f(height, 0.0, radius);
	glVertex3f(0.0, 0.0, radius);
	glEnd();

	// BACK
	glColor3f(1.0, 1.0, 0.5);
	glBegin(GL_POLYGON);
	angle = 0.0;
	while (angle < 2 * PI) {
		y = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(0.0, y, z);
		angle = angle + angle_stepsize;
	}
	glVertex3f(0.0, 0.0, radius);
	glEnd();

	// FRONT
	glColor3f(1.0, 1.0, 0.5);
	glBegin(GL_POLYGON);
	angle = 2 * PI - angle_stepsize;
	while (angle >= 0.0) {
		y = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(height, y, z);
		angle = angle - angle_stepsize;
	}
	glVertex3f(height, 0.0, radius);
	glEnd();
}

void drawStraddleCarrier() {
	glPushMatrix();
	// SIDE 1
	// WHEEL
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.3);
	drawCylinder(0.65, 0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 5.0);
	drawCylinder(0.65, 0.5);
	glPopMatrix();
	// WHEEL MOUNT
	drawCube(0.0, 0.65, 0.0, 0.5, 0.4, 0.5);
	drawCube(0.0, 0.65, 4.7, 0.5, 0.4, 0.5);
	// BOTTOM
	drawCube(0.0, 1.05, 0.0, 0.5, 0.5, 5.2);
	// SIDE
	drawCube(0.0, 1.55, 0.6, 0.5, 5.2, 0.35);
	drawCube(0.0, 1.55, 4.25, 0.5, 5.2, 0.35);

	// SIDE 2
	// WHEEL
	glPushMatrix();
	glTranslatef(3.1, 0.0, 0.3);
	drawCylinder(0.65, 0.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.1, 0.0, 5.0);
	drawCylinder(0.65, 0.5);
	glPopMatrix();
	// WHEEL MOUNT
	drawCube(3.1, 0.65, 0.0, 0.5, 0.4, 0.5);
	drawCube(3.1, 0.65, 4.7, 0.5, 0.4, 0.5);
	// BOTTOM
	drawCube(3.1, 1.05, 0.0, 0.5, 0.5, 5.2);
	// SIDE
	drawCube(3.1, 1.55, 0.6, 0.5, 5.2, 0.35);
	drawCube(3.1, 1.55, 4.25, 0.5, 5.2, 0.35);

	// TOP
	drawCube(0.0, 6.75, 0.6, 3.6, 0.4, 4.0);

	// CABIN
	drawCabin(0.0, 5.75, 4.6, 1.2, 1.2, 1.2);

	// SPREADER
	drawCube(1.0, 6.35, 1.1, 1.4, 0.2, 3.0);
	glPopMatrix();
}

void drawTruck() {
	glPushMatrix();
	// TRAILER
	// BASE
	drawCube(0.05, 0.18, 0.4, 0.65, 0.05, 2.5);

	// SIDE 1
	// WHEELS BACK
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.7);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.08);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// WHEELS FRONT
	glPushMatrix();
	glTranslatef(0.0, 0.0, 2.3);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 2.68);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// SIDE 2
	// WHEELS BACK
	glPushMatrix();
	glTranslatef(0.655, 0.0, 0.7);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.655, 0.0, 1.08);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// WHEELS FRONT
	glPushMatrix();
	glTranslatef(0.655, 0.0, 2.3);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.655, 0.0, 2.68);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// TRUCK
	// WHEELS SIDE 1
	glPushMatrix();
	glTranslatef(0.0, 0.0, 3.55);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// WHEELS SIDE 2
	glPushMatrix();
	glTranslatef(0.655, 0.0, 3.55);
	drawCylinder(0.18, 0.1);
	glPopMatrix();

	// SIDE 1
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.5, 1.0, 0.5);
	glVertex3f(0.05, 0.0, 3.0);
	glVertex3f(0.05, 0.0, 3.9);
	glVertex3f(0.05, 0.4, 3.9);
	glVertex3f(0.05, 0.75, 3.75);
	glVertex3f(0.05, 0.75, 3.0);
	glEnd();

	// SIDE 2
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0.5, 0.5, 1.0);
	glVertex3f(0.7, 0.0, 3.0);
	glVertex3f(0.7, 0.75, 3.0);
	glVertex3f(0.7, 0.75, 3.75);
	glVertex3f(0.7, 0.4, 3.9);
	glVertex3f(0.7, 0.0, 3.9);
	glEnd();

	glBegin(GL_QUADS);
	// BACK
	glColor3f(1.0, 0.5, 1.0);
	glVertex3f(0.05, 0.0, 3.0);
	glVertex3f(0.05, 0.75, 3.0);
	glVertex3f(0.7, 0.75, 3.0);
	glVertex3f(0.7, 0.0, 3.0);

	// FRONT-TOP
	glColor3f(0.5, 1.0, 1.0);
	glVertex3f(0.05, 0.4, 3.9);
	glVertex3f(0.7, 0.4, 3.9);
	glVertex3f(0.7, 0.75, 3.75);
	glVertex3f(0.05, 0.75, 3.75);

	// FRONT-BOTTOM
	glColor3f(1.0, 0.5, 0.5);
	glVertex3f(0.05, 0.0, 3.9);
	glVertex3f(0.7, 0.0, 3.9);
	glVertex3f(0.7, 0.4, 3.9);
	glVertex3f(0.05, 0.4, 3.9);

	// TOP
	glColor3f(1.0, 1.0, 0.5);
	glVertex3f(0.05, 0.75, 3.0);
	glVertex3f(0.05, 0.75, 3.75);
	glVertex3f(0.7, 0.75, 3.75);
	glVertex3f(0.7, 0.75, 3.0);

	// BOTTOM
	glColor3f(0.5, 1.0, 1.0);
	glVertex3f(0.05, 0.0, 3.0);
	glVertex3f(0.7, 0.0, 3.0);
	glVertex3f(0.7, 0.0, 3.9);
	glVertex3f(0.05, 0.0, 3.9);
	glEnd();

	glPopMatrix();
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

	drawTruck();

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
		camY = -10;
		rotY = 145;
		moveY = 0;
		moveX = 7;
		moveZ = 10;
	}
	else if (key == 'p') {
		std::cout << camY << "|" << rotY << "|" << moveY << "|" << moveX << "|" << moveZ;
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