#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <time.h> 
#include <GL/glut.h>

#define PI 3.1415927
#define TEXTURE_COUNT 18
#define SPREADER_LOWER_LIMIT 0.0
#define SPREADER_UPPER_LIMIT 0.5

#define CNT_NONE 0
#define CNT_SC 1
#define CNT_CT 2

#define NONE 0
#define STRADLE_CARRIER 1
#define CONTAINER_TRUCK 2

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
#define TX_FLOOR 13
#define TX_WATER 14
#define TX_SKY 15
#define TX_CONT_STACKF 16
#define TX_CONT_STACKB 17

#define CAMERA_RAD 3.5
using namespace std;

struct BitMapFile
{
	int sizeX;
	int sizeY;
	unsigned char* data;
};

GLfloat rotX = 0.0f;
GLfloat rotY = 3.1f;

GLfloat posCam[3];
GLfloat posCenter[3];

GLfloat posSc[] = { -9.0f, 0.0f, -10.0f };
GLfloat posCt[] = { -12.0f, 0.0f, -9.0f };
GLfloat posCn[] = { -9.0f, 0.0f, 3.0f };

GLfloat spHeight = 0.0;

int attachedTo = CNT_NONE;
int active = STRADLE_CARRIER;
int winId, winW, winH;

bool showWireframe = false;
bool showAxes = false;
bool showGrid = false;

static unsigned int texture[TEXTURE_COUNT];

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
	image[TX_FLOOR] = getbmp("textures/floor.bmp");
	image[TX_WATER] = getbmp("textures/water.bmp");
	image[TX_SKY] = getbmp("textures/sky.bmp");
	image[TX_CONT_STACKF] = getbmp("textures/container_stack_f.bmp");
	image[TX_CONT_STACKB] = getbmp("textures/container_stack_b.bmp");

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

void drawContainer(GLfloat width, GLfloat height, GLfloat length) {
	int tx_Container[] = { TX_CONT_SIDE, TX_CONT_SIDE, TX_CONT_FRONT, TX_CONT_BACK, TX_CONT_SIDE, TX_CONT_SIDE };
	drawCube(0.0, 0.065, 0.0, width, height, length, tx_Container);
}

void drawStraddleCarrier(GLfloat spreaderHeight, bool attached) {
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

	// CONTAINER
	if (attached) {
		glPushMatrix();
		glTranslatef(0.095, 0.406 - spreaderHeight, 0.06);
		drawContainer(0.17, 0.164, 0.4);
		glPopMatrix();
	}

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

void drawTruck(bool attached) {
	int tx_Body[] = { TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY, TX_TRUCK_BODY };
	int tx_Wheel[] = { TX_TYRE, TX_WHEEL1, TX_WHEEL1 };

	glPushMatrix();
	glTranslatef(0.0, 0.18, 0.0);
	// TRAILER - BASE
	drawCube(0.05, 0.18, 0.9, 0.65, 0.05, 2.1, tx_Body);

	// CONTAINER
	if (attached) {
		glPushMatrix();
		glTranslatef(-0.05, 0.18, 0.95);
		drawContainer(0.85, 0.82, 2.0);
		glPopMatrix();
	}

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

void drawEnv() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[TX_FLOOR]);
	glBegin(GL_QUADS);

	GLfloat tmpY = -30;
	for (int i = 0; i < 9; i++) {
		GLfloat tmpX = -30;

		for (int j = 0; j < 9; j++) {
			glTexCoord2f(0.0, 0.0); glVertex3f(tmpX, 0.0, tmpY);
			glTexCoord2f(1.0, 0.0); glVertex3f(tmpX, 0.0, tmpY + 10);
			glTexCoord2f(1.0, 1.0); glVertex3f(tmpX + 10, 0.0, tmpY + 10);
			glTexCoord2f(0.0, 1.0); glVertex3f(tmpX + 10, 0.0, tmpY);
			tmpX += 10;
		}
		tmpY += 10;
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[TX_WATER]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-30.0, 0.0, -30.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-60.0, 0.0, -30.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-60.0, 0.0, 60.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-30.0, 0.0, 60.0);

	glTexCoord2f(1.0, 0.0); glVertex3f(-60.0, 0.0, -30.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(60.0, 0.0, -30.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(60.0, 0.0, -60.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-60.0, 0.0, -60.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();

	// Camera
	if (active == STRADLE_CARRIER) {
		posCenter[0] = posSc[0] + 0.9;
		posCenter[1] = 1.0;
		posCenter[2] = posSc[2] + 1.6;
	}
	else if (active == CONTAINER_TRUCK) {
		posCenter[0] = posCt[0] + 0.375;
		posCenter[1] = 1.0;
		posCenter[2] = posCt[2] + 2.3;
	}

	if (active == NONE) {
		gluLookAt(0.0, 2.0 + (rotX * 5), 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

		glTranslatef(posCam[0], 0.0, posCam[2]);
		glRotatef(rotY * 30, 0.0f, 1.0f, 0.0f);
	}
	else {
		GLfloat camX = posCenter[0] + (CAMERA_RAD * sin(rotY));
		GLfloat camY = 2.0 + rotX;
		GLfloat camZ = posCenter[2] + (CAMERA_RAD * cos(rotY));

		gluLookAt(camX, camY, camZ, posCenter[0], posCenter[1], posCenter[2], 0.0, 1.0, 0.0);
	}

	// Draw environment
	drawEnv();

	// Draw axes
	if (showAxes) {
		drawAxes();
	}

	// Draw grid
	if (showGrid) {
		drawGrid();
	}

	glPushMatrix();

	// Draw Container
	if (attachedTo == CNT_NONE) {
		glPushMatrix();
		glTranslatef(posCn[0], posCn[1], posCn[2]);
		drawContainer(0.85, 0.82, 2.0);
		glPopMatrix();
	}
	else if (attachedTo == CNT_SC) {
		posCn[0] = posSc[0] + (0.095 * 5);
		posCn[2] = posSc[2] + (0.06 * 5);
	}

	// Draw Truck
	glPushMatrix();
	glTranslatef(posCt[0], 0.0, posCt[2]);
	drawTruck(attachedTo == CNT_CT);
	glPopMatrix();

	// Draw Straddle Carrier
	glPushMatrix();
	glTranslatef(posSc[0], 0.0, posSc[2]);
	glScalef(5.0, 5.0, 5.0);
	drawStraddleCarrier(spHeight, attachedTo == CNT_SC);
	glPopMatrix();

	glPopMatrix();

	glPopMatrix();
	glutSwapBuffers();
}

void keyboardSpecial(int key, int x, int y) {
	if (key == GLUT_KEY_F1) {
		showAxes = !showAxes;
	}
	else if (key == GLUT_KEY_F2) {
		showGrid = !showGrid;
	}
	else if (key == GLUT_KEY_F3) {
		if (showWireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		showWireframe = !showWireframe;
	}
	else if (active == NONE) {
		if (key == GLUT_KEY_UP) {
			posCam[2] += 1.0;
		}
		else if (key == GLUT_KEY_DOWN) {
			posCam[2] -= 1.0;
		}
		else if (key == GLUT_KEY_LEFT) {
			posCam[0] += 1.0;
		}
		else if (key == GLUT_KEY_RIGHT) {
			posCam[0] -= 1.0;
		}
	}
	else if (active == STRADLE_CARRIER) {
		if (key == GLUT_KEY_UP) {
			posSc[2] += 0.1;
		}
		else if (key == GLUT_KEY_DOWN) {
			posSc[2] -= 0.1;
		}
		else if (key == GLUT_KEY_LEFT) {
			posSc[0] += 0.1;
		}
		else if (key == GLUT_KEY_RIGHT) {
			posSc[0] -= 0.1;
		}
		else if (key == GLUT_KEY_PAGE_DOWN) {
			if (SPREADER_UPPER_LIMIT > spHeight) spHeight += 0.01;
		}
		else if (key == GLUT_KEY_PAGE_UP) {
			if (SPREADER_LOWER_LIMIT < spHeight) spHeight -= 0.01;
		}
	}
	else if (active == CONTAINER_TRUCK) {
		if (key == GLUT_KEY_UP) {
			posCt[2] += 0.2;
		}
		else if (key == GLUT_KEY_DOWN) {
			posCt[2] -= 0.2;
		}
		else if (key == GLUT_KEY_LEFT) {
			posCt[0] += 0.2;
		}
		else if (key == GLUT_KEY_RIGHT) {
			posCt[0] -= 0.2;
		}
	}
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		glutDestroyWindow(winId);
		exit(0);
	}
	else if (key == '0') {
		active = NONE;
	}
	else if (key == '1') {
		active = STRADLE_CARRIER;
		rotX = 0.0;
		rotY = 3.1;
	}
	else if (key == '2') {
		active = CONTAINER_TRUCK;
		rotX = 0.0;
		rotY = 3.1;
	}
	else if (active == STRADLE_CARRIER && key == ' ') {
		if (attachedTo == CNT_SC) {
			GLfloat diffX = posCt[0] - posSc[0];
			GLfloat diffZ = posCt[2] - posSc[2];

			if ((diffX > 0.2 && diffX < 0.8) && (diffZ > -0.8 && diffZ < -0.2)) {
				attachedTo = CNT_CT;
			}
			else {
				attachedTo = CNT_NONE;
				posCn[1] = 0;
			}
		}
		else {
			GLfloat diffX = posCn[0] - posSc[0];
			GLfloat diffZ = posCn[2] - posSc[2];

			if (attachedTo == CNT_NONE) {
				if ((diffX > 0.31 && diffX < 0.7) && (diffZ > -0.1 && diffZ < 0.7) && spHeight >= 0.5) {
					attachedTo = CNT_SC;
				}
			}
			else if (attachedTo == CNT_CT) {
				if ((diffX > 0.2 && diffX < 0.7) && (diffZ > -0.1 && diffZ < 0.7) && spHeight >= 0.41) {
					attachedTo = CNT_SC;
				}
			}
		}
	}

	glutPostRedisplay();
}

void mouse(int x, int y) {
	int midX = winW / 2;
	int midY = winH / 2;

	if (x != midX || y != midY) {
		glutWarpPointer(midX, midY);

		rotY += (GLfloat)((midX - x)) / 1000;
		rotX += (GLfloat)((midY - y)) / 500;

		if (rotX < 0.0) rotX = 0.0;
		if (rotX > 3.0) rotX = 3.0;
		glutPostRedisplay();
	}
}

void timer(int x) {
	glutPostRedisplay();
	glutTimerFunc(60, timer, 1);
}

void init() {
	glClearColor(0.0, 0.364, 0.741, 1.0);
	glGenTextures(TEXTURE_COUNT, texture);
	loadExternalTextures();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void changeSize(GLsizei w, GLsizei h) {
	winW = w;
	winH = h;

	glViewport(0, 0, w, h);
	GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(120.0, aspect_ratio, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv) {
	srand(time(NULL));

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(150, 150);

	winId = glutCreateWindow("Straddle Carrier");

	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutPassiveMotionFunc(mouse);
	glutSetCursor(GLUT_CURSOR_NONE);

	glutTimerFunc(60.0, timer, 1);
	init();
	glutMainLoop();

	return 0;
}