#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <time.h>
#include <GL/glut.h>

#define PI 3.1415927
#define TEXTURE_COUNT 38
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
#define TX_CONT1 10
#define TX_CONT2 11
#define TX_CONT3 12
#define TX_FLOOR1 13
#define SKYBOX_TOP 14
#define SKYBOX_DOWN 15
#define TX_CONT4 16
#define TX_CONT5 17
#define TX_WALL1 18
#define TX_WALL2 19
#define TX_WALL3 20
#define TX_ROOF 21
#define TX_CEILING 22
#define TX_FLOOR2 23 
#define TX_CONT6 24
#define TX_CONT7 25
#define TX_CONT8 26
#define TX_CONT9 27
#define TX_CONT10 28
#define TX_CONT11 29
#define TX_CONT12 30
#define TX_CONT13 31
#define TX_CONT14 32
#define TX_CONT15 33
#define SKYBOX_FRONT 34
#define SKYBOX_BACK 35
#define SKYBOX_LEFT 36
#define SKYBOX_RIGHT 37

#define CAMERA_RAD 3.5

#define CO_CN1 0
#define CO_CN2 1

using namespace std;

GLfloat rotX = 0.0f, rotY = 3.13f;

GLfloat posCam[3];
GLfloat posCenter[3];

GLfloat posSc[] = { -0.9f, 0.0f, -14.0f };
GLfloat posCt[] = { -0.7f, 0.0f, 48.0f };
GLfloat posCn[] = { -0.45f, 0.0f, 3.0f };
GLfloat posTrucks[3][2];
bool dirTrucks[3];

GLfloat spHeight = 0.0;

// Container Type, Face, (x, y) of Corner
static GLfloat containers_tex[2][6][8] = {
	{ // Main Container
		{0.858, 0.650246, 0.142, 0.650246, 0.142, 0.35468, 0.858, 0.35468},
		{0.858, 0.650246, 0.858, 0.35468, 0.142, 0.35468, 0.142, 0.650246},
		{0.0, 0.35468, 0.0, 0.650246, 0.142, 0.650246, 0.142, 0.35468},
		{0.858, 0.650246, 1.0, 0.650246, 1.0, 0.35468, 0.858, 0.35468},
		{0.858, 1.0, 0.142, 1.0, 0.142, 0.650246, 0.858, 0.650246},
		{0.858, 0.0, 0.858, 0.35468, 0.142, 0.35468, 0.142, 0.0}
	},
	{ // Other Containers
		{0.774, 0.668657, 0.22, 0.668657, 0.22, 0.334328, 0.774, 0.334328},
		{0.774, 0.668657, 0.774, 0.334328, 0.22, 0.334328,  0.22, 0.668657},
		{0.0, 0.668657, 0.0, 0.334328, 0.22, 0.334328, 0.22, 0.668657},
		{1.0, 0.668657, 0.774, 0.668657, 0.774, 0.334328, 1.0, 0.334328},
		{0.774, 1.0, 0.22, 1.0, 0.22, 0.668657, 0.774, 0.668657},
		{0.774, 0.0, 0.774, 0.334328, 0.22, 0.334328, 0.22, 0.0}
	}
};

int attachedTo = CNT_NONE;
int active = STRADLE_CARRIER;
int winId, winW, winH;

bool showWireframe = false;
bool showAxes = false;
bool showGrid = false;

static unsigned int texture[TEXTURE_COUNT];
static unsigned int container_textures[14] = { TX_CONT2, TX_CONT3, TX_CONT4, TX_CONT5, TX_CONT6, TX_CONT7, TX_CONT8,
								TX_CONT9, TX_CONT10, TX_CONT11, TX_CONT12, TX_CONT13, TX_CONT14, TX_CONT15 };

unsigned int container_stack1[44];
unsigned int container_stack2[100];

struct BitMapFile {
	int sizeX;
	int sizeY;
	unsigned char* data;
};

BitMapFile* getbmp(string filename) {
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

void loadExternalTextures() {
	glGenTextures(TEXTURE_COUNT, texture);

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
	image[TX_FLOOR1] = getbmp("textures/floor1.bmp");
	image[TX_FLOOR2] = getbmp("textures/floor2.bmp");
	image[TX_WALL1] = getbmp("textures/wall1.bmp");
	image[TX_WALL2] = getbmp("textures/wall2.bmp");
	image[TX_WALL3] = getbmp("textures/wall3.bmp");
	image[TX_ROOF] = getbmp("textures/roof.bmp");
	image[TX_CEILING] = getbmp("textures/ceiling.bmp");
	image[TX_CONT1] = getbmp("textures/container1.bmp");
	image[TX_CONT2] = getbmp("textures/container2.bmp");
	image[TX_CONT3] = getbmp("textures/container3.bmp");
	image[TX_CONT4] = getbmp("textures/container4.bmp");
	image[TX_CONT5] = getbmp("textures/container5.bmp");
	image[TX_CONT6] = getbmp("textures/container6.bmp");
	image[TX_CONT7] = getbmp("textures/container7.bmp");
	image[TX_CONT8] = getbmp("textures/container8.bmp");
	image[TX_CONT9] = getbmp("textures/container9.bmp");
	image[TX_CONT10] = getbmp("textures/container10.bmp");
	image[TX_CONT11] = getbmp("textures/container11.bmp");
	image[TX_CONT12] = getbmp("textures/container12.bmp");
	image[TX_CONT13] = getbmp("textures/container13.bmp");
	image[TX_CONT14] = getbmp("textures/container14.bmp");
	image[TX_CONT15] = getbmp("textures/container15.bmp");
	image[SKYBOX_TOP] = getbmp("textures/skybox_top.bmp");
	image[SKYBOX_DOWN] = getbmp("textures/skybox_down.bmp");
	image[SKYBOX_FRONT] = getbmp("textures/skybox_front.bmp");
	image[SKYBOX_BACK] = getbmp("textures/skybox_back.bmp");
	image[SKYBOX_LEFT] = getbmp("textures/skybox_left.bmp");
	image[SKYBOX_RIGHT] = getbmp("textures/skybox_right.bmp");

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

void drawCube(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l, int texture_id[]) {
	glEnable(GL_TEXTURE_2D);

	// TOP
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[0]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glEnd();

	// BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[1]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glEnd();

	// FRONT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[2]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z + l);
	glEnd();

	// BACK
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[3]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z);
	glEnd();

	// LEFT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[4]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glEnd();

	// RIGHT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[5]]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawCylinder(GLfloat radius, GLfloat height, int texture_id[]) {
	GLfloat y = 0.0, z = 0.0;
	GLfloat txX = 0.0, txY = 0.0;
	GLfloat xcos = 0.0, ysin = 0.0;
	GLfloat angle_stepsize = 0.1;

	glEnable(GL_TEXTURE_2D);

	// TUBE
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[0]]);

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
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[1]]);

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
	glBindTexture(GL_TEXTURE_2D, texture[texture_id[2]]);

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

void drawContainer(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l, int texture_id, int container_id) {
	glEnable(GL_TEXTURE_2D);

	// TOP
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][0][0], containers_tex[container_id][0][1]); glVertex3f(x, y + h, z);
	glTexCoord2f(containers_tex[container_id][0][2], containers_tex[container_id][0][3]); glVertex3f(x, y + h, z + l);
	glTexCoord2f(containers_tex[container_id][0][4], containers_tex[container_id][0][5]); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(containers_tex[container_id][0][6], containers_tex[container_id][0][7]); glVertex3f(x + w, y + h, z);
	glEnd();

	// BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][1][0], containers_tex[container_id][1][1]); glVertex3f(x, y, z);
	glTexCoord2f(containers_tex[container_id][1][2], containers_tex[container_id][1][3]); glVertex3f(x + w, y, z);
	glTexCoord2f(containers_tex[container_id][1][4], containers_tex[container_id][1][5]); glVertex3f(x + w, y, z + l);
	glTexCoord2f(containers_tex[container_id][1][6], containers_tex[container_id][1][7]); glVertex3f(x, y, z + l);
	glEnd();

	// FRONT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][2][0], containers_tex[container_id][2][1]); glVertex3f(x, y, z + l);
	glTexCoord2f(containers_tex[container_id][2][2], containers_tex[container_id][2][3]); glVertex3f(x + w, y, z + l);
	glTexCoord2f(containers_tex[container_id][2][4], containers_tex[container_id][2][5]); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(containers_tex[container_id][2][6], containers_tex[container_id][2][7]); glVertex3f(x, y + h, z + l);
	glEnd();

	// BACK
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][3][0], containers_tex[container_id][3][1]); glVertex3f(x, y, z);
	glTexCoord2f(containers_tex[container_id][3][2], containers_tex[container_id][3][3]); glVertex3f(x, y + h, z);
	glTexCoord2f(containers_tex[container_id][3][4], containers_tex[container_id][3][5]); glVertex3f(x + w, y + h, z);
	glTexCoord2f(containers_tex[container_id][3][6], containers_tex[container_id][3][7]); glVertex3f(x + w, y, z);
	glEnd();

	// LEFT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][4][0], containers_tex[container_id][4][1]); glVertex3f(x, y, z);
	glTexCoord2f(containers_tex[container_id][4][2], containers_tex[container_id][4][3]); glVertex3f(x, y, z + l);
	glTexCoord2f(containers_tex[container_id][4][4], containers_tex[container_id][4][5]); glVertex3f(x, y + h, z + l);
	glTexCoord2f(containers_tex[container_id][4][6], containers_tex[container_id][4][7]); glVertex3f(x, y + h, z);
	glEnd();

	// RIGHT
	glBindTexture(GL_TEXTURE_2D, texture[texture_id]);
	glBegin(GL_QUADS);
	glTexCoord2f(containers_tex[container_id][5][0], containers_tex[container_id][5][1]); glVertex3f(x + w, y, z);
	glTexCoord2f(containers_tex[container_id][5][2], containers_tex[container_id][5][3]); glVertex3f(x + w, y + h, z);
	glTexCoord2f(containers_tex[container_id][5][4], containers_tex[container_id][5][5]); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(containers_tex[container_id][5][6], containers_tex[container_id][5][7]); glVertex3f(x + w, y, z + l);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawWall(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l, int tx) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[tx]);
	glBegin(GL_QUADS);

	// TOP
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);

	// FRONT
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z + l);

	// BACK
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z);

	// LEFT
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);

	// RIGHT
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawWarehouse(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l) {
	int tx_wallF[] = { TX_WALL3, TX_WALL3, TX_WALL2, TX_WALL3, TX_WALL3, TX_WALL3 };
	int tx_wallB[] = { TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL2, TX_WALL3, TX_WALL3 };
	int tx_wallR[] = { TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL2, TX_WALL3 };
	int tx_wallL[] = { TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL3, TX_WALL2 };

	drawCube(x + 0.3, y, z + l - 0.3, w - 0.6, h, 0.3, tx_wallB); // Back Wall
	drawCube(x, y, z, 0.3, h, l, tx_wallL); // Left Wall
	drawCube(x + w - 0.3, y, z, 0.3, h, l, tx_wallR); // Right Wall

	// Front Wall
	GLfloat tw = (w - 0.6) / 15;
	GLfloat th1 = h * 0.25;
	GLfloat th2 = h * 0.75;

	for (int i = 0; i < 15; i++) {
		if (i % 2 == 0) {
			drawCube(x + (i * tw) + 0.3, y, z, tw, h, 0.3, tx_wallF);
		}
		else {
			drawCube(x + (i * tw) + 0.3, y + th2, z, tw, th1, 0.3, tx_wallF);
		}
	}

	glEnable(GL_TEXTURE_2D);

	// Left Top Wall
	glBindTexture(GL_TEXTURE_2D, texture[TX_WALL2]);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + 0.3, y + h, z);
	glTexCoord2f(0.5, 0.0); glVertex3f(x + 0.3, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 1.0); glVertex3f(x + 0.3, y + h, z + l);

	glTexCoord2f(0.0, 0.0); glVertex3f(x + w - 0.3, y + h, z + l);
	glTexCoord2f(0.5, 0.0); glVertex3f(x + w - 0.3, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w - 0.3, y + h, z);
	glEnd();

	// Right Top Wall
	glBindTexture(GL_TEXTURE_2D, texture[TX_WALL3]);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(0.5, 0.0); glVertex3f(x + w, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z + l);

	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.5, 0.0); glVertex3f(x, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[TX_ROOF]);
	glBegin(GL_QUADS);
	// Left Roof
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z);
	glTexCoord2f(0.0, 10.0); glVertex3f(x, y + h + 2.0, z + (l / 2));
	glTexCoord2f(10.0, 10.0); glVertex3f(x + w, y + h + 2.0, z + (l / 2));
	glTexCoord2f(10.0, 0.0); glVertex3f(x + w, y + h, z);

	// Right Roof
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(10.0, 0.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(10.0, 10.0); glVertex3f(x + w, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 10.0); glVertex3f(x, y + h + 2.0, z + (l / 2));
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[TX_CEILING]);
	glBegin(GL_QUADS);
	// Left Ceiling
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z);
	glTexCoord2f(10.0, 0.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(10.0, 10.0); glVertex3f(x + w, y + h + 2.0, z + (l / 2));
	glTexCoord2f(0.0, 10.0); glVertex3f(x, y + h + 2.0, z + (l / 2));

	// Right Ceiling
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 10.0); glVertex3f(x, y + h + 2.0, z + (l / 2));
	glTexCoord2f(10.0, 10.0); glVertex3f(x + w, y + h + 2.0, z + (l / 2));
	glTexCoord2f(10.0, 0.0); glVertex3f(x + w, y + h, z + l);
	glEnd();

	// Floor
	glBindTexture(GL_TEXTURE_2D, texture[TX_FLOOR2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y + 0.001, z);
	glTexCoord2f(0.0, 20.0); glVertex3f(x, y + 0.001, z + l);
	glTexCoord2f(20.0, 20.0); glVertex3f(x + w, y + 0.001, z + l);
	glTexCoord2f(20.0, 0.0); glVertex3f(x + w, y + 0.001, z);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawSkybox(GLfloat x, GLfloat y, GLfloat z, GLfloat w, GLfloat h, GLfloat l) {
	x = x - w / 2;
	y = y - h / 2;
	z = z - l / 2;

	glEnable(GL_TEXTURE_2D);

	// TOP
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_TOP]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y + h, z + l);
	glEnd();

	// BOTTOM
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_DOWN]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z);
	glEnd();

	// FRONT
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_FRONT]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z + l);
	glEnd();

	// BACK
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_BACK]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z);
	glEnd();

	// LEFT
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_LEFT]);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, y + h, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x, y + h, z + l);
	glTexCoord2f(0.0, 0.0); glVertex3f(x, y, z + l);
	glEnd();

	// RIGHT
	glBindTexture(GL_TEXTURE_2D, texture[SKYBOX_RIGHT]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(x + w, y, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x + w, y, z + l);
	glTexCoord2f(1.0, 1.0); glVertex3f(x + w, y + h, z + l);
	glTexCoord2f(0.0, 1.0); glVertex3f(x + w, y + h, z);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void drawPost(GLfloat x, GLfloat y, GLfloat z) {
	int tx_Post[] = { TX_METAL_GRAY, TX_METAL_GRAY, TX_METAL_GRAY };
	int tx_PostTop[] = { TX_GLASS_CABIN, TX_WALL2, TX_GLASS_CABIN, TX_GLASS_CABIN, TX_GLASS_CABIN, TX_GLASS_CABIN };

	glPushMatrix();
	glTranslatef(x, y, z);

	glPushMatrix();
	glRotatef(90.0, 0.0, 0.0, 1.0);
	drawCylinder(0.2, 6.0, tx_Post);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0, 5.8, 0.0);
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	glRotatef(15.0, 0.0, 0.0, 1.0);
	drawCylinder(0.1, 2.0, tx_Post);
	glPopMatrix();

	glTranslatef(-0.6, 0.0, 1.85);
	drawCube(0.0, 6.25, 0.0, 1.2, 0.1, 1.2, tx_PostTop);
	glPopMatrix();
}

void drawEnv() {
	// Floor
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[TX_FLOOR1]);
	glBegin(GL_QUADS);

	GLfloat tmpZ = -30;
	GLfloat tmpX = -30;
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			glTexCoord2f(0.0, 0.0); glVertex3f(tmpX, 0.0, tmpZ);
			glTexCoord2f(1.0, 0.0); glVertex3f(tmpX, 0.0, tmpZ + 10);
			glTexCoord2f(1.0, 1.0); glVertex3f(tmpX + 10, 0.0, tmpZ + 10);
			glTexCoord2f(0.0, 1.0); glVertex3f(tmpX + 10, 0.0, tmpZ);
			tmpX += 10;
		}
		tmpZ += 10;
		tmpX = -30;
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//Walls
	tmpZ = -30;
	for (int i = 0; i < 9; i++) {
		drawWall(-30, 0.0, tmpZ, 0.5, 0.6, 10.0, TX_WALL1);
		tmpZ += 10;
	}

	tmpX = -30;
	for (int i = 0; i < 9; i++) {
		drawWall(tmpX, 0.0, -30, 10.0, 0.6, 0.5, TX_WALL1);
		tmpX += 10;
	}

	drawWall(-30, 0.0, 59.8, 90.0, 3.2, 0.2, TX_WALL1);

	// Warehouses
	drawWarehouse(-20.0, 0.0, 44.5, 30.0, 3.0, 15.0);
	drawWarehouse(20.0, 0.0, 44.5, 30.0, 3.0, 15.0);

	// Container Stacks
	for (int i = 0; i < 44; i++) {
		drawContainer(58.3, 0.0, 57.88 - (i * 2.0), 1.7, 1.64, 2.0, container_stack1[i], CO_CN2);
		drawContainer(58.3, 1.64, 57.88 - (i * 2.0), 1.7, 1.64, 2.0, container_stack1[43 - i], CO_CN2);
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 25; j++) {
			int t = (i * 25) + j;
			drawContainer(40.0 - (16 * i), 0.0, 28 - (j * 2.0), 1.7, 1.64, 2.0, container_stack2[t], CO_CN2);
			drawContainer(40.0 - (16 * i), 1.64, 28 - (j * 2.0), 1.7, 1.64, 2.0, container_stack2[99 - t], CO_CN2);

			drawContainer(38.3 - (16 * i), 0.0, 28 - (j * 2.0), 1.7, 1.64, 2.0, container_stack2[t], CO_CN2);
			drawContainer(38.3 - (16 * i), 1.64, 28 - (j * 2.0), 1.7, 1.64, 2.0, container_stack2[99 - t], CO_CN2);
		}
	}

	// Trucks
	for (int i = 2; i < 8; i++) {
		if (i != 5) {
			glPushMatrix();
			glTranslatef((i * 3.92) - 20.3, 0.0, 48.0);
			glRotatef(180, 0, 1, 0);
			drawTruck();
			glPopMatrix();
		}
	}

	for (int i = 1; i < 7; i++) {
		if (i != 3) {
			glPushMatrix();
			glTranslatef((i * 3.92) + 19.65, 0.0, 48.0);
			glRotatef(180, 0, 1, 0);
			drawTruck();
			glPopMatrix();
		}
	}

	// Light Post
	for (int i = 0; i < 5; i++) {
		drawPost((i * 20.0) - 25.0, 0.0, -29.0);
	}

	// Skybox
	drawSkybox(0.0, 0.0, 0.0, 120.0, 80.0, 120.0);
}

void initLighting() {
	GLfloat L_Ambient[] = { 1.0, 1.0, 0.15, 1.0 };
	GLfloat L_Diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat L_Specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat L_SpotDirection[] = { 0.0, -1.0, 1.0 };

	GLfloat L0_Postion[] = { -25.0, 6.0, -26.0, 1.0 };
	GLfloat L1_Postion[] = { -5.0, 6.0, -26.0, 1.0 };
	GLfloat L2_Postion[] = { 15.0, 6.0, -26.0, 1.0 };
	GLfloat L3_Postion[] = { 35.0, 6.0, -26.0, 1.0 };
	GLfloat L4_Postion[] = { 55.0, 6.0, -26.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, L_Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, L_Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, L_Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, L0_Postion);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, L_SpotDirection);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60);

	glLightfv(GL_LIGHT1, GL_AMBIENT, L_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, L_Diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, L_Specular);
	glLightfv(GL_LIGHT1, GL_POSITION, L1_Postion);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, L_SpotDirection);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60);

	glLightfv(GL_LIGHT2, GL_AMBIENT, L_Ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, L_Diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, L_Specular);
	glLightfv(GL_LIGHT2, GL_POSITION, L2_Postion);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, L_SpotDirection);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 60);

	glLightfv(GL_LIGHT3, GL_AMBIENT, L_Ambient);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, L_Diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, L_Specular);
	glLightfv(GL_LIGHT3, GL_POSITION, L3_Postion);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, L_SpotDirection);
	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 60);

	glLightfv(GL_LIGHT4, GL_AMBIENT, L_Ambient);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, L_Diffuse);
	glLightfv(GL_LIGHT4, GL_SPECULAR, L_Specular);
	glLightfv(GL_LIGHT4, GL_POSITION, L4_Postion);
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, L_SpotDirection);
	glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 60);
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
		posCenter[0] = posCt[0] - 0.375;
		posCenter[1] = 1.0;
		posCenter[2] = posCt[2] - 2.3;
	}

	if (active == NONE) {
		gluLookAt(0.0, 2.0 + (rotX * 5), 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

		glTranslatef(posCam[0], 0.0, posCam[2]);
		glRotatef(rotY * 30, 0.0f, 1.0f, 0.0f);
	}
	else {
		GLfloat camX = posCenter[0] + (CAMERA_RAD * sin(rotY));
		GLfloat camY = 2.0 + rotX;
		GLfloat camZ;

		if (active == STRADLE_CARRIER) {
			camZ = posCenter[2] + (CAMERA_RAD * cos(rotY));
		}
		else {
			camZ = posCenter[2] - (CAMERA_RAD * cos(rotY));
		}

		gluLookAt(camX, camY, camZ, posCenter[0], posCenter[1], posCenter[2], 0.0, 1.0, 0.0);
	}

	initLighting();

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

	// Draw Container
	if (attachedTo == CNT_SC) {
		posCn[0] = posSc[0] + 1.325;
		posCn[1] = posSc[1] - (spHeight * 5) + 2.68;
		posCn[2] = posSc[2] + 2.3;
	}
	else if (attachedTo == CNT_CT) {
		posCn[0] = posCt[0] + 0.05;
		posCn[1] = posCt[1] + 0.41;
		posCn[2] = posCt[2] - 0.95;
	}

	glPushMatrix();
	glTranslatef(posCn[0], posCn[1], posCn[2]);
	glRotatef(180, 0, 1, 0);
	drawContainer(0.0, 0.0, 0.0, 0.85, 0.82, 2.0, TX_CONT1, CO_CN1);
	glPopMatrix();

	// Draw Truck
	glPushMatrix();
	glTranslatef(posCt[0], 0.0, posCt[2]);
	glRotatef(180, 0, 1, 0);
	drawTruck();
	glPopMatrix();

	// Draw Straddle Carrier
	glPushMatrix();
	glTranslatef(posSc[0], posSc[1], posSc[2]);
	glScalef(5.0, 5.0, 5.0);
	drawStraddleCarrier(spHeight);
	glPopMatrix();

	// Trucks
	for (int i = 0; i < 3; i++) {
		if (dirTrucks[i]) {
			glPushMatrix();
			glTranslatef(posTrucks[i][0], 0.0, posTrucks[i][1]);
			glRotatef(180, 0, 1, 0);
			drawTruck();
			glPopMatrix();
		}
		else {
			glPushMatrix();
			glTranslatef(posTrucks[i][0] - 0.65, 0.0, posTrucks[i][1]);
			drawTruck();
			glPopMatrix();

			// Container
			drawContainer(posTrucks[i][0] - 0.7, 0.41, posTrucks[i][1] + 0.95, 0.85, 0.82, 2.0, TX_CONT1, CO_CN1);
		}
	}

	glPopMatrix();
	glutSwapBuffers();
}

void handleKeyPress(unsigned char key) {
	if (active == NONE) {
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
	}
	else if (active == CONTAINER_TRUCK) {
		if (key == GLUT_KEY_UP) {
			posCt[2] -= 0.2;
		}
		else if (key == GLUT_KEY_DOWN) {
			posCt[2] += 0.2;
		}
		else if (key == GLUT_KEY_LEFT) {
			posCt[0] -= 0.2;
		}
		else if (key == GLUT_KEY_RIGHT) {
			posCt[0] += 0.2;
		}
	}
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
	else if (key == GLUT_KEY_PAGE_DOWN) {
		if (active == STRADLE_CARRIER && SPREADER_UPPER_LIMIT > spHeight) {
			spHeight += 0.01;
		}
	}
	else if (key == GLUT_KEY_PAGE_UP) {
		if (active == STRADLE_CARRIER && SPREADER_LOWER_LIMIT < spHeight) {
			spHeight -= 0.01;
		}
	}
	else {
		handleKeyPress(key);
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
		rotY = 3.13;
	}
	else if (key == '2') {
		active = CONTAINER_TRUCK;
		rotX = 0.0;
		rotY = 3.13;
	}
	else if (key == 'w') {
		handleKeyPress(GLUT_KEY_UP);
	}
	else if (key == 'a') {
		handleKeyPress(GLUT_KEY_LEFT);
	}
	else if (key == 's') {
		handleKeyPress(GLUT_KEY_DOWN);
	}
	else if (key == 'd') {
		handleKeyPress(GLUT_KEY_RIGHT);
	}
	else if (active == STRADLE_CARRIER && key == ' ') {
		if (attachedTo == CNT_SC) {
			GLfloat diffX = posCt[0] - posSc[0];
			GLfloat diffZ = posCt[2] - posSc[2];

			if ((diffX > 1.1 && diffX < 1.5) && (diffZ > 3.1 && diffZ < 3.4)) {
				attachedTo = CNT_CT;
			}
			else {
				attachedTo = CNT_NONE;
				posCn[1] = 0;
			}
		}
		else {
			GLfloat diffX = posCn[0] - posSc[0];
			GLfloat diffY = (posSc[1] - (spHeight * 5) + 2.68) - posCn[1];
			GLfloat diffZ = posCn[2] - posSc[2];

			if ((diffX > 1.2 && diffX < 1.5) && (diffZ > 1.9 && diffZ < 2.7) && (diffY > 0 && diffY < 0.15)) {
				attachedTo = CNT_SC;
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

		if (active == STRADLE_CARRIER) {
			rotY += (GLfloat)((midX - x)) / 1000;
		}
		else {
			rotY -= (GLfloat)((midX - x)) / 1000;
		}

		rotX += (GLfloat)((midY - y)) / 500;

		if (rotX < 0.0) rotX = 0.0;
		if (rotX > 3.0) rotX = 3.0;

		glutPostRedisplay();
	}
}

void timer(int x) {
	for (int i = 0; i < 3; i++) {
		if (dirTrucks[i]) {
			posTrucks[i][1] -= 0.2;

			if (posTrucks[i][1] < -20) {
				dirTrucks[i] = false;
			}
		}
		else {
			posTrucks[i][1] += 0.2;

			if (posTrucks[i][1] > 45) {
				dirTrucks[i] = true;
			}
		}
	}

	glutPostRedisplay();
	glutTimerFunc(60, timer, 1);
}

void init() {
	GLfloat globalAmbient[] = { 0.6, 0.6, 0.6, 0.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	loadExternalTextures();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_LIGHTING);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	glEnable(GL_LIGHT3);
	glEnable(GL_LIGHT4);

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
}

void changeSize(GLsizei w, GLsizei h) {
	winW = w;
	winH = h;

	glViewport(0, 0, w, h);
	GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0, aspect_ratio, 1.0, 150.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char** argv) {
	srand(time(NULL));

	// Container Stacks
	for (int i = 0; i < 44; i++) {
		container_stack1[i] = container_textures[rand() % 14];
	}

	for (int i = 0; i < 100; i++) {
		container_stack2[i] = container_textures[rand() % 14];
	}

	// Trucks
	for (int i = 0; i < 3; i++) {
		dirTrucks[i] = (rand() % 2 == 0);

		if (dirTrucks[i]) {
			posTrucks[i][1] = rand() % 30;
		}
		else {
			posTrucks[i][1] = -(rand() % 20);
		}
	}
	posTrucks[0][0] = -16.38;
	posTrucks[1][0] = 31.41;
	posTrucks[2][0] = 47.09;

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(150, 150);

	winId = glutCreateWindow("Straddle Carrier");
	glutFullScreen();

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