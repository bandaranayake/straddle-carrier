#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <GL/glut.h>

#define PI 3.1415927
#define TEXTURE_COUNT 13
#define SPREADER_LOWER_LIMIT 0.0
#define SPREADER_UPPER_LIMIT 0.5

#define TX_METAL_RED 0
#define TX_METAL_GRAY 1
#define TX_GLASS_CABIN 2
#define TX_GLASS_TRUCK 3
#define TX_WHEEL1 4
#define TX_WHEEL2 5
#define TX_TYRE 6
#define TX_TRUCK_FRONT1 7
#define TX_TRUCK_FRONT2 8
#define TX_TRUCK_BODY 9
#define TX_CONT_FRONT 10
#define TX_CONT_BACK 11
#define TX_CONT_SIDE 12

using namespace std;

struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char* data;
};

GLfloat moveX = 0.0f;
GLfloat moveY = 0.0f;
GLfloat moveZ = 0.0f;

GLfloat rotX = 0.0f;
GLfloat rotY = 0.0f;
GLfloat rotZ = 0.0f;

GLfloat camY = 0.0f;
GLfloat camX = 0.0f;
GLfloat camZ = 0.0f;

GLfloat spHeight = 0.0;

bool showWireframe = false;
bool showAxes = true;
bool showGrid = true;

static unsigned int texture[TEXTURE_COUNT];

GLUquadricObj* qobj;

BitMapFile* getbmp(string filename)
{
	int offset, headerSize;

	BitMapFile* bmpRGB = new BitMapFile;
	BitMapFile* bmpRGBA = new BitMapFile;

	ifstream infile(filename.c_str(), ios::binary);

	infile.seekg(10);
	infile.read((char*)&offset, 4);

	infile.read((char*)&headerSize, 4);

	infile.seekg(18);
	infile.read((char*)&bmpRGB->sizeX, 4);
	infile.read((char*)&bmpRGB->sizeY, 4);

	int padding = (3 * bmpRGB->sizeX) % 4 ? 4 - (3 * bmpRGB->sizeX) % 4 : 0;
	int sizeScanline = 3 * bmpRGB->sizeX + padding;
	int sizeStorage = sizeScanline * bmpRGB->sizeY;
	bmpRGB->data = new unsigned char[sizeStorage];

	infile.seekg(offset);
	infile.read((char*)bmpRGB->data, sizeStorage);

	int startScanline, endScanlineImageData, temp;
	for (int y = 0; y < bmpRGB->sizeY; y++)
	{
		startScanline = y * sizeScanline;
		endScanlineImageData = startScanline + 3 * bmpRGB->sizeX;
		for (int x = startScanline; x < endScanlineImageData; x += 3)
		{
			temp = bmpRGB->data[x];
			bmpRGB->data[x] = bmpRGB->data[x + 2];
			bmpRGB->data[x + 2] = temp;
		}
	}

	bmpRGBA->sizeX = bmpRGB->sizeX;
	bmpRGBA->sizeY = bmpRGB->sizeY;
	bmpRGBA->data = new unsigned char[4 * bmpRGB->sizeX * bmpRGB->sizeY];

	for (int j = 0; j < 4 * bmpRGB->sizeY * bmpRGB->sizeX; j += 4)
	{
		bmpRGBA->data[j] = bmpRGB->data[(j / 4) * 3];
		bmpRGBA->data[j + 1] = bmpRGB->data[(j / 4) * 3 + 1];
		bmpRGBA->data[j + 2] = bmpRGB->data[(j / 4) * 3 + 2];
		bmpRGBA->data[j + 3] = 0xFF;
	}
	return bmpRGBA;
}

void loadExternalTextures()
{
	BitMapFile* image[TEXTURE_COUNT];
	image[TX_METAL_RED] = getbmp("textures/metal_red.bmp");
	image[TX_METAL_GRAY] = getbmp("textures/metal_gray.bmp");
	image[TX_GLASS_CABIN] = getbmp("textures/cabin_glass.bmp");
	image[TX_WHEEL1] = getbmp("textures/wheel1.bmp");
	image[TX_WHEEL2] = getbmp("textures/wheel2.bmp");
	image[TX_TYRE] = getbmp("textures/tyre.bmp");
	image[TX_TRUCK_FRONT1] = getbmp("textures/truck_front_top.bmp");
	image[TX_TRUCK_FRONT2] = getbmp("textures/truck_front_bottom.bmp");
	image[TX_TRUCK_BODY] = getbmp("textures/truck_body.bmp");
	image[TX_GLASS_TRUCK] = getbmp("textures/truck_glass.bmp");
	image[TX_CONT_FRONT] = getbmp("textures/container_front.bmp");
	image[TX_CONT_BACK] = getbmp("textures/container_back.bmp");
	image[TX_CONT_SIDE] = getbmp("textures/container_side.bmp");

	for (int i = 0; i < TEXTURE_COUNT; i++) {
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[i]->sizeX, image[i]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[i]->data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
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

void drawCube(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l, int tx[]) {
	glEnable(GL_TEXTURE_2D);

	// TOP
	glBindTexture(GL_TEXTURE_2D, texture[tx[0]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glEnd();

	// BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[tx[1]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glEnd();

	// FRONT
	glBindTexture(GL_TEXTURE_2D, texture[tx[2]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z + l);
	glEnd();

	// BACK
	glBindTexture(GL_TEXTURE_2D, texture[tx[3]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z);
	glEnd();

	// LEFT
	glBindTexture(GL_TEXTURE_2D, texture[tx[4]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glEnd();

	// RIGHT
	glBindTexture(GL_TEXTURE_2D, texture[tx[5]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawCylinder(GLfloat radius, GLfloat height, int tx[]) {
	GLfloat y = 0.0, z = 0.0;
	GLfloat txX = 0.0, txY = 0.0;
	GLfloat xcos = 0.0, ysin = 0.0;
	GLfloat angle_stepsize = 0.1;

	glEnable(GL_TEXTURE_2D);

	// TUBE
	glBindTexture(GL_TEXTURE_2D, texture[tx[0]]);

	glBegin(GL_QUAD_STRIP);
	for (GLfloat angle = 2 * PI; angle >= 0.0; angle = angle - angle_stepsize) {
		y = radius * sin(angle);
		z = radius * cos(angle);
		glTexCoord2f(1.0, 0.0);  glVertex3f(height, y, z);
		glTexCoord2f(1.0, 1.0);  glVertex3f(0.0, y, z);
	}
	glVertex3f(height, 0.0, radius);
	glVertex3f(0.0, 0.0, radius);
	glEnd();

	// BASE
	glBindTexture(GL_TEXTURE_2D, texture[tx[1]]);

	glBegin(GL_POLYGON);
	for (GLfloat angle = 0.0; angle < 2 * PI; angle = angle + angle_stepsize) {
		ysin = sin(angle);
		xcos = cos(angle);
		y = radius * ysin;
		z = radius * xcos;
		txX = xcos * 0.5 + 0.5;
		txY = ysin * 0.5 + 0.5;

		glTexCoord2f(txX, txY); glVertex3f(0.0, y, z);
	}
	glVertex3f(0.0, 0.0, radius);
	glEnd();

	// TOP
	glBindTexture(GL_TEXTURE_2D, texture[tx[2]]);

	glBegin(GL_POLYGON);
	for (GLfloat angle = 2 * PI - angle_stepsize; angle >= 0.0; angle = angle - angle_stepsize) {
		ysin = sin(angle);
		xcos = cos(angle);
		y = radius * ysin;
		z = radius * xcos;
		txX = xcos * 0.5 + 0.5;
		txY = ysin * 0.5 + 0.5;

		glTexCoord2f(txX, txY); glVertex3f(height, y, z);
	}
	glVertex3f(height, 0.0, radius);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawCabin() {
	glEnable(GL_TEXTURE_2D);

	// SIDE 1 - Body
	glBindTexture(GL_TEXTURE_2D, texture[TX_METAL_RED]);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.12, 0.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.12, 0.12, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.12, 0.12, 0.12);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.12, 0.04, 0.12);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.12, 0.0, 0.09);
	glEnd();

	// SIDE 2 - Body
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.09);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.04, 0.12);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.0, 0.12, 0.12);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.0, 0.12, 0.0);
	glEnd();

	glBegin(GL_QUADS);
	// TOP
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.12, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.12, 0.12);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.12, 0.12, 0.12);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.12, 0.12, 0.0);

	// BACK
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.12, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.12, 0.12, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.12, 0.0, 0.0);

	// FRONT-TOP
	glTexCoord2f(0.0, 0.0);	glVertex3f(0.0, 0.04, 0.12);
	glTexCoord2f(1.0, 0.0);	glVertex3f(0.12, 0.04, 0.12);
	glTexCoord2f(1.0, 1.0);	glVertex3f(0.12, 0.12, 0.12);
	glTexCoord2f(0.0, 1.0);	glVertex3f(0.0, 0.12, 0.12);

	// FRONT-BOTTOM
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.04, 0.12);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.0, 0.0, 0.09);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.12, 0.0, 0.09);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.12, 0.04, 0.12);

	// BOTTOM
	glTexCoord2f(0.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.12, 0.0, 0.0);
	glTexCoord2f(1.0, 1.0);	glVertex3f(0.12, 0.0, 0.09);
	glTexCoord2f(0.0, 1.0);	glVertex3f(0.0, 0.0, 0.09);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[TX_GLASS_CABIN]);
	// FRONT-TOP - Window
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);	glVertex3f(0.01, 0.05, 0.1201);
	glTexCoord2f(1.0, 0.0);	glVertex3f(0.11, 0.05, 0.1201);
	glTexCoord2f(1.0, 1.0);	glVertex3f(0.11, 0.11, 0.1201);
	glTexCoord2f(0.0, 1.0);	glVertex3f(0.01, 0.11, 0.1201);

	// FRONT-BOTTOM - Window
	glTexCoord2f(0.0, 0.0); glVertex3f(0.01, 0.03, 0.1201);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.01, 0.005, 0.0901);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.11, 0.005, 0.0901);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.11, 0.03, 0.1201);
	glEnd();

	// SIDE 1 - Window
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.0001, 0.015, 0.015);
	glTexCoord2f(0.0, 0.9); glVertex3f(-0.0001, 0.015, 0.09);
	glTexCoord2f(0.4, 1.0); glVertex3f(-0.0001, 0.04, 0.105);
	glTexCoord2f(1.0, 1.0); glVertex3f(-0.0001, 0.105, 0.105);
	glTexCoord2f(1.0, 0.0); glVertex3f(-0.0001, 0.105, 0.015);
	glEnd();

	// SIDE 1 - Window
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.1201, 0.015, 0.015);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.1201, 0.105, 0.015);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.1201, 0.105, 0.105);
	glTexCoord2f(0.4, 1.0); glVertex3f(0.1201, 0.04, 0.105);
	glTexCoord2f(0.0, 0.9); glVertex3f(0.1201, 0.015, 0.09);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawStraddleCarrier(GLfloat spreaderHeight) {
	int tx_Body[] = { TX_METAL_RED, TX_METAL_RED, TX_METAL_RED, TX_METAL_RED, TX_METAL_RED, TX_METAL_RED };
	int tx_Wheel[] = { TX_TYRE, TX_WHEEL2, TX_WHEEL2 };
	int tx_Chain[] = { TX_METAL_GRAY, TX_METAL_GRAY, TX_METAL_GRAY };

	glPushMatrix();
	glTranslatef(0.0, 0.065, 0.0);

	// SIDE 1 - WHEELS
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.03);
	drawCylinder(0.065, 0.05, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.50);
	drawCylinder(0.065, 0.05, tx_Wheel);
	glPopMatrix();
	// SIDE 1 - WHEEL MOUNTS
	drawCube(0.0, 0.065, 0.0, 0.05, 0.04, 0.05, tx_Body);
	drawCube(0.0, 0.065, 0.47, 0.05, 0.04, 0.05, tx_Body);
	// SIDE 1 - BOTTOM
	drawCube(0.0, 0.105, 0.0, 0.05, 0.05, 0.52, tx_Body);
	// SIDE 1 - SIDES
	drawCube(0.0, 0.155, 0.06, 0.05, 0.52, 0.035, tx_Body);
	drawCube(0.0, 0.155, 0.425, 0.05, 0.52, 0.035, tx_Body);

	// SIDE 2 - WHEELS
	glPushMatrix();
	glTranslatef(0.31, 0.0, 0.03);
	drawCylinder(0.065, 0.05, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.31, 0.0, 0.5);
	drawCylinder(0.065, 0.05, tx_Wheel);
	glPopMatrix();
	// SIDE 2 - WHEEL MOUNTS
	drawCube(0.31, 0.065, 0.0, 0.05, 0.04, 0.05, tx_Body);
	drawCube(0.31, 0.065, 0.47, 0.05, 0.04, 0.05, tx_Body);
	// SIDE 2 - BOTTOM
	drawCube(0.31, 0.105, 0.0, 0.05, 0.05, 0.52, tx_Body);
	// SIDE 2 - SIDES
	drawCube(0.31, 0.155, 0.06, 0.05, 0.52, 0.035, tx_Body);
	drawCube(0.31, 0.155, 0.425, 0.05, 0.52, 0.035, tx_Body);

	// TOP
	drawCube(0.0, 0.675, 0.06, 0.36, 0.04, 0.4, tx_Body);

	// CABIN
	glPushMatrix();
	glTranslatef(0.0, 0.575, 0.46);
	drawCabin();
	glPopMatrix();

	// SPREADER
	drawCube(0.11, 0.635 - spreaderHeight, 0.11, 0.14, 0.02, 0.3, tx_Body);

	// SPREADER - CHAINS
	glPushMatrix();
	glTranslatef(0.12, 0.637 - spreaderHeight, 0.12);
	glRotatef(90, 0, 0, 1);
	drawCylinder(0.003, 0.04 + spreaderHeight, tx_Chain);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.24, 0.637 - spreaderHeight, 0.12);
	glRotatef(90, 0, 0, 1);
	drawCylinder(0.003, 0.04 + spreaderHeight, tx_Chain);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.24, 0.637 - spreaderHeight, 0.4);
	glRotatef(90, 0, 0, 1);
	drawCylinder(0.003, 0.04 + spreaderHeight, tx_Chain);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.12, 0.637 - spreaderHeight, 0.4);
	glRotatef(90, 0, 0, 1);
	drawCylinder(0.003, 0.04 + spreaderHeight, tx_Chain);
	glPopMatrix();

	glPopMatrix();
}

void drawTruck() {
	int tx_Body[] = { TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY };
	int tx_Wheel[] = { TX_TYRE, TX_WHEEL1, TX_WHEEL1 };

	glPushMatrix();
	glTranslatef(0.0, 0.18, 0.0);
	// TRAILER - BASE
	drawCube(0.05, 0.18, 0.9, 0.65, 0.05, 2.1, tx_Body);

	// TRAILER - WHEELS BACK (SIDE 1)
	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.2);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 1.58);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	// TRAILER - WHEELS FRONT (SIDE 1)
	glPushMatrix();
	glTranslatef(0.0, 0.0, 2.3);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 0.0, 2.68);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	// TRAILER - WHEELS BACK (SIDE 2)
	glPushMatrix();
	glTranslatef(0.655, 0.0, 1.2);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.655, 0.0, 1.58);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	// TRAILER - WHEELS FRONT (SIDE 2)
	glPushMatrix();
	glTranslatef(0.655, 0.0, 2.3);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.655, 0.0, 2.68);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	// TRUCK - WHEELS SIDE 1
	glPushMatrix();
	glTranslatef(0.0, 0.0, 3.35);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	// TRUCK - WHEELS SIDE 2
	glPushMatrix();
	glTranslatef(0.655, 0.0, 3.35);
	drawCylinder(0.18, 0.1, tx_Wheel);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	// BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[TX_TRUCK_BODY]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.05, 0.0, 3.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.7, 0.0, 3.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.7, 0.0, 3.7);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.05, 0.0, 3.7);
	glEnd();

	// FRONT-TOP
	glBindTexture(GL_TEXTURE_2D, texture[TX_TRUCK_FRONT1]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.05, 0.4, 3.7);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.7, 0.4, 3.7);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.7, 0.75, 3.55);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.05, 0.75, 3.55);
	glEnd();

	// FRONT-BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[TX_TRUCK_FRONT2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(0.05, 0.0, 3.7);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.7, 0.0, 3.7);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.7, 0.4, 3.7);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.05, 0.4, 3.7);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[TX_TRUCK_BODY]);
	glBegin(GL_QUADS);
	// BACK
	glTexCoord2f(0.0, 0.0); glVertex3f(0.05, 0.0, 3.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.05, 0.75, 3.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.7, 0.75, 3.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.7, 0.0, 3.0);

	// TOP
	glTexCoord2f(0.0, 0.0); glVertex3f(0.05, 0.75, 3.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(0.05, 0.75, 3.55);
	glTexCoord2f(1.0, 1.0); glVertex3f(0.7, 0.75, 3.55);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.7, 0.75, 3.0);
	glEnd();

	// SIDE 1
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.8); glVertex3f(0.05, 0.0, 3.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.05, 0.0, 3.7);
	glTexCoord2f(0.4, 1.0); glVertex3f(0.05, 0.4, 3.7);
	glTexCoord2f(0.75, 0.95); glVertex3f(0.05, 0.75, 3.55);
	glTexCoord2f(0.75, 0.8); glVertex3f(0.05, 0.75, 3.0);
	glEnd();

	// SIDE 2
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.8); glVertex3f(0.7, 0.0, 3.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.7, 0.75, 3.0);
	glTexCoord2f(0.4, 1.0); glVertex3f(0.7, 0.75, 3.55);
	glTexCoord2f(0.75, 0.95); glVertex3f(0.7, 0.4, 3.7);
	glTexCoord2f(0.75, 0.8); glVertex3f(0.7, 0.0, 3.7);
	glEnd();

	// SIDE 1 - Window
	glBindTexture(GL_TEXTURE_2D, texture[TX_GLASS_TRUCK]);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.8); glVertex3f(0.049, 0.4, 3.1);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.049, 0.4, 3.65);
	glTexCoord2f(0.4, 1.0); glVertex3f(0.049, 0.4, 3.65);
	glTexCoord2f(0.75, 0.95); glVertex3f(0.049, 0.7, 3.5);
	glTexCoord2f(0.75, 0.8); glVertex3f(0.049, 0.7, 3.1);
	glEnd();

	// SIDE 2 - Window
	glBindTexture(GL_TEXTURE_2D, texture[TX_GLASS_TRUCK]);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.8); glVertex3f(0.701, 0.4, 3.1);
	glTexCoord2f(0.75, 0.8); glVertex3f(0.701, 0.7, 3.1);
	glTexCoord2f(0.75, 0.95); glVertex3f(0.701, 0.7, 3.5);
	glTexCoord2f(0.4, 1.0); glVertex3f(0.701, 0.4, 3.65);
	glTexCoord2f(0.0, 1.0); glVertex3f(0.701, 0.4, 3.65);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	gluLookAt(0.0 + camX, 2.0 + camY, 5.0 + camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

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
		camY = -10;
		rotY = 145;
		moveY = 0;
		moveX = 7;
		moveZ = 10;
	}
	else if (key == 'p') {
		cout << camY << "|" << rotY << "|" << moveY << "|" << moveX << "|" << moveZ;
	}
	glutPostRedisplay();
}

void init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glGenTextures(TEXTURE_COUNT, texture);
	loadExternalTextures();

	qobj = gluNewQuadric();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
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