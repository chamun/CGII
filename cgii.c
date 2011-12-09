/* Autores: 
 * Bruney Narciso da Silva
 * Rodrigo Chamun
 */
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "draw.h"

#define abs(x)       ((x) >= 0 ? (x) : -(x))
#define min(x,y)     ((x) > (y) ? (y) : (x))
#define max(x,y)     ((x) < (y) ? (y) : (x))
#define toRadians(x) ((x) * 3.14 / 180)

#define WINDOW_SIZE 700

#define SCORE_BOARD_WIDTH  150
#define SCORE_BOARD_HEIGHT  35
#define MAP_SIZE           (WINDOW_SIZE / 4) 

#define PLAYER_HEIGHT  1.8f
#define MAX_OBJ_HEIGHT 10.0f

#define MAXX 0
#define MAXY 1
#define MAXZ 2
#define MINX 3
#define MINY 4
#define MINZ 5

/* This order is *VERY* important and shouldn't be changed */
#define W 0
#define S 1
#define A 2
#define D 3
#define P 4

#define COMPASS_POINT_N 5
#define USEFUL_KEYS     4
#define VELOCITY        50 

/* Structs */
struct point { float x, y; };
struct object { 
	float x, y;
	float w, h;
	float scale;
	float translate;
	int visible;
};

/* Globals */
int DYNAMIC_OBJECTS_NUMBER;
int FIXED_OBJECTS_NUMBER  ;
int WORLD_SIDE = 100;

struct point compass[COMPASS_POINT_N] = {           /* Player coordinates  */
                                          {0 , -1}, /* W - Front vector    */
                                          {0 ,  1}, /* S - Back vector     */
                                          {-1,  0}, /* A - Left vector     */
                                          {1 ,  0}, /* D - Right vector    */
                                          {0 ,  0}  /* P - Player position */
                                        } ; 
int keys[USEFUL_KEYS]; /* A list with all the pressed keys */
int px, py;            /* Keeps previous mouse coordinates between mouse actions */
int init_x, init_y;    /* Avatar initial position */

/* Game control */
int score   = 0; /* the game score         */
int gametime    = 0; /* playing time           */
int playing = 1; /* if the game is running */
int game    = 0; /* game number            */
int map     = 0; /* map on/off             */

/* Objectes coordinates */
struct object *coins ; /* points */
struct object *fixeds; /* fixed  */

/* Models */
objObj *obj;          /* points */
objObj **fixeds_objs; /* fixeds */

/* Prototypes */

/* Remove a key to the current keys list */
void key_list_remove(int key);
/* Add a key to the current keys list */
void key_list_add(int key);
/* Reset the game */
void reset_game(void);

/* Vector minus operation, stores the result in *result */
void vecMinus (struct point, struct point, struct point *result);
/* Vector plus operation, stores the result in *result */
void vecPlus (struct point, struct point, struct point *result);
/* Multiplies a vector by alpha */
void vecTimes (struct point *, float alpha); 

/* Draw all the objects that are considered points */
void drawCoins(void);
/* Draws all the fixed objects */
void drawFixeds(void);
/* Draw the floor and the walls */
void drawEnv(void);
/* Draw a circle of radius 3 w/ center at (x,y) */
void drawCircle(float x, float y);
/* Draw a circle of side 5 w/ center at (x, y) */
void drawSquare(float x, float y);
/* 
 * Draws the triangle that represents the player on the mini 
 * map given the map scale
 */
void drawCompassTriangle(float scale);
/* Draws the mini map w/ center at (x, y) */
void drawMiniMap(int x, int y);
/* Draws the score boart w/ center at (x, y) */
void drawScoreBoard(int x, int y);
/* Draws the "You win" board */
void drawEnd(void);
/* Main Draw function */
void draw(void);

/* Keyboard callback, when a key is released */
void keyboardUp(unsigned char key, int x, int y); 
/* Keyboard callback, when a key is pressed */
void keyboard(unsigned char key, int x, int y);
/* Mouse callback, when mouse is moved */
void mouseMoved(int x, int y);
/* Mouse callback, when mouse is pressed */
void mousePressed(int button, int state, int x, int y);

/* Moves the compass */
void translate_compass(float dx, float dy);
/* Rotates the compass */
void rotate_compass(float angle);

/* Checks collision with the points objects */
void collided_points(void);
/* Checks collision with the fixed objects */
int collided_fixed(float x, float y);
/* Checks whether (x, y) is inside obj */
int collision(float x, float y, struct object *obj);
/* Checks whethet the player is inside the walls */
int collided_walls(void);

/* Sets up illumination */
void illumination(void);
/* Moves the player in a given direction (W,A,S,D) */
void move(int side);
/* Get the min and max points of an obj object */
void getCoords(objObj *obj, float *coords);
/* Sets up our object struct with given coordinates and position */
void fill_object(struct object *obj, float x, float y, float coords[6]);
/* Load config file */
void initObjs();
/* Place all point objects randomly on the map */
void generatePoints(void);
/* Ticker */
void timer(int t);
/* Movement ticker */
void move_time (int value);

/* Functions */
int 
main(int argc, char *argv[])
{
	/* Glut Configs */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);

	glutInitWindowPosition(0,0);
	glutInitWindowSize    (WINDOW_SIZE, WINDOW_SIZE);
	glutCreateWindow      ("Take the Cohen");

	/* OpenGL Configs */
	glClearColor   (0.0f, 0.0f, 0.0f, 1.0f); /* black */
	glShadeModel   (GL_SMOOTH);
	glBlendFunc    (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_NORMALIZE); /* Avoids weird illumination */

	/* Callbacks */
	glutDisplayFunc    (draw); 
	glutKeyboardFunc   (keyboard); 
	glutKeyboardUpFunc (keyboardUp);
	glutMouseFunc      (mousePressed); 
	glutMotionFunc     (mouseMoved);
	glutTimerFunc      (VELOCITY, move_time, 0);
	glutTimerFunc      (1000, timer, game);

	/* Globals initialization */
	memset(keys, 0, sizeof(keys));
	srand(time(NULL));
	initObjs();
	generatePoints();

	/* Start doin' things! */
	glutMainLoop ();

	return 0;
}

/* Key list management */
void
key_list_remove(int key) 
{
	keys[key] = 0;
}

void
key_list_add(int key)
{
	keys[key] = 1;
	switch (key) {
		case 'W':
			key_list_remove(S);
		break;
		case 'S':
			key_list_remove(W);
		break;
		case 'A':
			key_list_remove(D);
		break;
		case 'D':
			key_list_remove(S);
		break;
	}
}

void
reset_game(void)
{
	gametime = 0;
	glutTimerFunc(1000, timer, ++game);
	score = 0;
	playing = 1;

	int i;
	for (i = 0; i < DYNAMIC_OBJECTS_NUMBER; i++)
		coins[i].visible = 1;

	translate_compass(init_x - compass[P].x, init_y - compass[P].y);
}

/* Vector operations */
void
vecMinus (struct point a, struct point b, struct point *result)
{
	result->x = a.x - b.x;
	result->y = a.y - b.y;
}

void
vecPlus (struct point a, struct point b, struct point *result)
{
	result->x = a.x + b.x;
	result->y = a.y + b.y;
}

void
vecTimes (struct point *p, float alpha) {
		p->x *= alpha;
		p->y *= alpha;
}

/* Drawing Functions */
void
drawCoins(void)
{
	int i;

	for (i = 0; i < DYNAMIC_OBJECTS_NUMBER; i++) {
		if (coins[i].visible) {
			glPushMatrix();
				glTranslatef(coins[i].x, coins[i].translate, coins[i].y);
				drawObj(*obj);
			glPopMatrix();
		}
	}
}
void
drawFixeds()
{
	int i;

	for (i = 0; i < FIXED_OBJECTS_NUMBER; i++) {
			glPushMatrix();
				glTranslatef(fixeds[i].x, fixeds[i].translate, fixeds[i].y);
				glScalef(fixeds[i].scale, fixeds[i].scale, fixeds[i].scale);
				drawObj(*(fixeds_objs[i]));
			glPopMatrix();
	}
}
void 
drawEnv(void)
{
	int i;

	glBegin(GL_LINES);
		for (i = - WORLD_SIDE; i < WORLD_SIDE; i++) {
			if (i % 2 == 0)
				glColor3f(1.0f, 1.0f, 1.0f);
			else
				glColor3f(0.5f, 0.5f, 0.5f);
			glVertex3f(i,           0,  WORLD_SIDE);
			glVertex3f(i,           0, -WORLD_SIDE);
			glVertex3f( WORLD_SIDE, 0,           i);
			glVertex3f(-WORLD_SIDE, 0,           i);
		}
	glEnd();

	glBegin(GL_QUADS);
		glColor3f(1.0f, 0.0f, 0.0f);
		glNormal3f(0,0,1);
		glVertex3f(-WORLD_SIDE, WORLD_SIDE, -WORLD_SIDE);
		glVertex3f( WORLD_SIDE, WORLD_SIDE, -WORLD_SIDE);
		glVertex3f( WORLD_SIDE,          0, -WORLD_SIDE);
		glVertex3f(-WORLD_SIDE,          0, -WORLD_SIDE);
		glColor3f(0.3f, 1.0f, 0.3f);
		glNormal3f(1,0,0);
		glVertex3f(-WORLD_SIDE, WORLD_SIDE, -WORLD_SIDE);
		glVertex3f(-WORLD_SIDE, WORLD_SIDE,  WORLD_SIDE);
		glVertex3f(-WORLD_SIDE,          0,  WORLD_SIDE);
		glVertex3f(-WORLD_SIDE,          0, -WORLD_SIDE);
		glColor3f(0.3f, 0.3f, 1.0f);
		glNormal3f(0,0,-1);
		glVertex3f(-WORLD_SIDE, WORLD_SIDE, WORLD_SIDE);
		glVertex3f(-WORLD_SIDE,          0, WORLD_SIDE);
		glVertex3f( WORLD_SIDE,          0, WORLD_SIDE);
		glVertex3f( WORLD_SIDE, WORLD_SIDE, WORLD_SIDE);
		glColor3f(1.0f, 0.3f, 1.0f);
		glNormal3f(-1,0,0);
		glVertex3f(WORLD_SIDE, WORLD_SIDE,  WORLD_SIDE);
		glVertex3f(WORLD_SIDE, WORLD_SIDE, -WORLD_SIDE);
		glVertex3f(WORLD_SIDE,          0, -WORLD_SIDE);
		glVertex3f(WORLD_SIDE,          0,  WORLD_SIDE);
	glEnd();
}
void 
drawCircle(float x, float y)
{
	int i, np;
	float px, py, nx, ny, c, s;
	float angle;

	np = 20;
	px = 3;
	py = 0;
	angle = toRadians(360 / (float) np);
	c = cos(angle);
	s = sin(angle);

	glPushMatrix();
		glTranslatef(x, y, 0);
		glColor3f(0, 0, 0);
		glBegin(GL_LINE_LOOP);
			for (i = 0; i < np; i++) {
				nx = px * c - py * s;
				ny = px * s + py * c;

				glVertex2f(nx, ny);

				px = nx;
				py = ny;
			}
		glEnd();
	glPopMatrix();
}
void
drawSquare(float x, float y)
{
	float side = 5.0f;

	glPushMatrix();
		glTranslatef(x, y, 0);
		glColor3f(0, 0, 0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(  side / 2,  side / 2);
			glVertex2f(  side / 2, -side / 2);
			glVertex2f( -side / 2, -side / 2);
			glVertex2f( -side / 2,  side / 2);
		glEnd();
	glPopMatrix();

}
void
drawCompassTriangle(float scale)
{
	int i;
	struct point p, aux;
	float size = 3 / scale;

	vecMinus(compass[S], compass[P], &aux);

	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glPushMatrix();
		glBegin(GL_TRIANGLES);
			for (i = 0; i < COMPASS_POINT_N; i++) {
				if (i == S) 
					continue;
				vecMinus(compass[i], compass[P], &p);
				vecTimes(&p, size);
				if( i == A || i == D)  /* makes the triangle less flattened */
					vecPlus(p, aux, &p);
				vecPlus(compass[P], p, &p);
				glVertex2f(p.x * scale, p.y * scale);
			}
		glEnd();
	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void
drawTransparentBoard(int x, int y, int width, int height)
{
	glPushMatrix();
		glTranslatef(x, y, 0);
		/* Board */
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glBegin(GL_QUADS);
			glVertex2f(0    , 0     );
			glVertex2f(width, 0     );
			glVertex2f(width, height);
			glVertex2f(0    , height);
		glEnd();
		glDisable(GL_BLEND);
		glColor3f(1.0f, 1.0f, 1.0f);
		/* outline */
		glBegin(GL_LINE_LOOP);
			glVertex2f(0    , 0     );
			glVertex2f(width, 0     );
			glVertex2f(width, height);
			glVertex2f(0    , height);
		glEnd();
	glPopMatrix();
}

void
drawMiniMap(int x, int y)
{
	int i;
	float scale = min((MAP_SIZE / ((float) WORLD_SIDE * 2)), 1);
	float side  = WORLD_SIDE * scale;

	glPushMatrix();
		glTranslatef(x, y, 0);

		drawTransparentBoard(-side, -side, side * 2, side * 2);
				/* Points */
		for (i = 0; i < DYNAMIC_OBJECTS_NUMBER; i++) 
			if (coins[i].visible)
				drawCircle(coins[i].x * scale, coins[i].y * scale);
		/* Fixed */
		for (i = 0; i < FIXED_OBJECTS_NUMBER; i++) 
				drawSquare(fixeds[i].x * scale, fixeds[i].y * scale);

		/* PLayer */
		glColor3f(0.0f, 0.0f, 0.0f);
		drawCompassTriangle(scale);

	glPopMatrix();
}
void
drawScoreBoard(int x, int y)
{
	char numberbuf[10];

	glPushMatrix();
		glTranslatef(x, y, 0);

		drawTransparentBoard(0, 0, SCORE_BOARD_WIDTH, SCORE_BOARD_HEIGHT);
		
		/* Text */
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(1, 15);
		glutBitmapString(GLUT_BITMAP_9_BY_15, "Time:  ");
		sprintf(numberbuf, "%9d", gametime);
		glutBitmapString(GLUT_BITMAP_9_BY_15, numberbuf);
		glRasterPos2f(1, 30);
		glutBitmapString(GLUT_BITMAP_9_BY_15, "Score: ");
		sprintf(numberbuf, "%9d", score);
		glutBitmapString(GLUT_BITMAP_9_BY_15, numberbuf);
	glPopMatrix();
}
void 
drawEnd(void)
{
	glPushMatrix();
		glTranslatef(WINDOW_SIZE/2 - 50, WINDOW_SIZE/2 - 50, 0);

		drawTransparentBoard(0, 0, SCORE_BOARD_WIDTH, 40);	

		/* Text */
		glColor3f(0.0f, 0.0f, 0.0f);
		glRasterPos2f(30, 20);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, "You won!");
		glRasterPos2f(1, 35);
		glutBitmapString(GLUT_BITMAP_HELVETICA_12, "Press 'r' to restart");
		drawScoreBoard(0, 40);
	glPopMatrix();
}

void
draw(void)
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* 3D */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glMatrixMode(GL_PROJECTION);    
	glLoadIdentity();
	gluPerspective( 60, 1, 0.05, WORLD_SIDE * 2 * 2.4142);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	illumination();

	gluLookAt(compass[P].x, PLAYER_HEIGHT, compass[P].y,
	          compass[W].x, PLAYER_HEIGHT, compass[W].y, 
              0, 1, 0);

	drawEnv();
	drawCoins();
	drawFixeds(); 

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	/* 2D */

	/* Isso aqui só precisa ser chamado uma vez, e quando a janela muda de
	 * tamanho */
	glViewport(0, 0, WINDOW_SIZE, WINDOW_SIZE); 
	glMatrixMode(GL_PROJECTION);    
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_SIZE, WINDOW_SIZE, 0);

	glMatrixMode(GL_MODELVIEW);    
	glLoadIdentity();
	if(playing) {
		drawScoreBoard(20,20);
		if(map)
			drawMiniMap(3.5f * MAP_SIZE, 3.5f * MAP_SIZE);
	} else 
		drawEnd();

	glutSwapBuffers();
}


/* I/O */

void
keyboardUp(unsigned char key, int x, int y) {
	switch(key) {
		case 'w':
				key_list_remove(W);
			break;
		case 's':
				key_list_remove(S);
			break;
		case 'a':
				key_list_remove(A);
			break;
		case 'd':
				key_list_remove(D);
			break;
	}
}
void
keyboard(unsigned char key, int x, int y) 
{
	switch(key) {
		case 'w':
				key_list_add(W);
			break;
		case 's':
				key_list_add(S);
			break;
		case 'a':
				key_list_add(A);
			break;
		case 'd':
				key_list_add(D);
			break;
		case 'r':
				reset_game();
			break;
		case 'm':
				map = !map;
			break;
	}
}

void
mouseMoved(int x, int y)
{
	int dx, dy;

	dx = px - x;
	dy = py - y;

	rotate_compass(dx);

	px = x;
	py = y;

	glutPostRedisplay();
}

void
mousePressed(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		px = x;
		py = y;
	}
}

/* Compass */
void
translate_compass(float dx, float dy) {
	int i;

	for (i = 0; i < COMPASS_POINT_N; i++) {
		compass[i].x += dx;
		compass[i].y += dy;
	}
}

void
rotate_compass(float angle)
{
	int i;
	float nx, ny, x, y, c, s;

	if(!playing)
		return;

	/* Conversão de graus pra radianos */
	angle = toRadians(angle); 
	c = cos(angle);
	s = sin(angle);


	for (i = 0; i < P; i++) {
		x = compass[i].x - compass[P].x; 
		y = compass[i].y - compass[P].y;

		nx = x * c - y * s;
		ny = x * s + y * c;

		compass[i].x = nx + compass[P].x;
		compass[i].y = ny + compass[P].y;
	}

}

/* Collision */
int 
collided_walls(void)
{
	if (!(   compass[P].x > -WORLD_SIDE && compass[P].x < WORLD_SIDE
	      && compass[P].y > -WORLD_SIDE && compass[P].y < WORLD_SIDE))
		  return 1;

	return 0;
}

int
collision(float x, float y, struct object *obj)
{
	int minx = obj->x - obj->w/2;
	int miny = obj->y - obj->h/2;

	return    x >= minx && x <= minx + obj->w
	       && y >= miny && y <= miny + obj->h;
}

int
collided_fixed(float x, float y)
{
		int i;
		for (i = 0; i < FIXED_OBJECTS_NUMBER; i++) 
			if(collision(x, y, &fixeds[i]))
					return 1;
		return 0;
}

void
collided_points(void)
{
	int i;

	for (i = 0; i < DYNAMIC_OBJECTS_NUMBER; i++) {
		if (coins[i].visible) {
			if (collision(compass[P].x, compass[P].y, &coins[i])) {
				score++;
				coins[i].visible = 0;
			}
		}
	}
	if(score == DYNAMIC_OBJECTS_NUMBER)
		playing = 0;
}

/* Others */

void 
illumination(void)
{
	/* Iluminação */
	GLfloat ambient    [4] = {0.2, 0.2, 0.2, 1.0};
	GLfloat diffuse    [4] = {0.7, 0.7, 0.7, 1.0};
	GLfloat specular   [4] = {0.8, 0.8, 0.8, 1.0};
	GLfloat position   [4] = {  0,   0,  -1, 1.0};
	GLfloat specularity[4] = {1.0, 1.0, 1.0, 1.0};

	glMaterialfv(GL_FRONT, GL_SPECULAR, specularity);
	glMateriali(GL_FRONT, GL_SHININESS, 60);

	glLightfv(GL_LIGHT0,  GL_AMBIENT,  ambient);
	glLightfv(GL_LIGHT0,  GL_DIFFUSE,  diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
}

void
move(int side)
{
	float dx, dy;

	if (!playing) 
		return;

	dx = compass[side].x - compass[P].x;
	dy = compass[side].y - compass[P].y;

	translate_compass(dx , dy);

	if (collided_walls() || collided_fixed(compass[P].x, compass[P].y))
		translate_compass(-dx, -dy);
}

void
move_time (int value)
{
	int i;

	for (i = 0; i < USEFUL_KEYS; i++) 
		if (keys[i])
			move(i);

	collided_points();

	glutPostRedisplay();
	glutTimerFunc(VELOCITY, move_time, 1);
}

void
getCoords(objObj *obj, float *coords)
{
	int i, j, ind;
	float x, y, z;

	for (i = 0; i < obj->nbFace; i++) {
		for (j = 0; j < obj->faceList[i].nbVertex; j++) {
			ind = obj->faceList[i].vertexIndexList[j];

			x = obj->vertexList[ind].coords[0];
			y = obj->vertexList[ind].coords[1];
			z = obj->vertexList[ind].coords[2];

			coords[MAXX] = max (coords[MAXX], x);
			coords[MINX] = min (coords[MINX], x);
			coords[MAXY] = max (coords[MAXY], y);
			coords[MINY] = min (coords[MINY], y);
			coords[MAXZ] = max (coords[MAXZ], z);
			coords[MINZ] = min (coords[MINZ], z);

		}
	}

}

void
fill_object(struct object *obj, float x, float y, float coords[6])
{
	float w, h;
	float scale, trans;

	w = abs(coords[MAXX] - coords[MINX]);
	h = abs(coords[MAXZ] - coords[MINZ]);
	scale = min (MAX_OBJ_HEIGHT / abs(coords[MAXY] - coords[MINY]), 1);
	trans = -coords[MINY];

	obj->x = x;
	obj->y = y;
	obj->w = w * scale;
	obj->h = h * scale;
	obj->translate = trans;
	obj->scale = scale;
	obj->visible = 1;
}

void
initObjs()
{
	char texto[128];
	int x, y, z, cont;

	scanf("%s", texto);
	scanf("%d,%d,%d", &x, &y, &z);
	translate_compass(x, z);
	init_x = x;
	init_y = z;
	WORLD_SIDE = max(x, WORLD_SIDE);
	WORLD_SIDE = max(z, WORLD_SIDE);

	scanf("%s", texto);
	if (strcmp(texto, "Fixos") == 0) {
		scanf("%d", &cont);
		FIXED_OBJECTS_NUMBER = cont;
		fixeds      = malloc(sizeof(struct object) * cont);
		fixeds_objs = malloc(sizeof(objObj) * cont)		  ;

		cont--;
		while (cont > -1) {
			fixeds_objs[cont] = (objObj *) malloc(sizeof(objObj));

			scanf("%s", texto);
			openObj(texto, fixeds_objs[cont]);

			float coords[6] = { -INFINITY, -INFINITY, -INFINITY, INFINITY, INFINITY, INFINITY };
			getCoords(fixeds_objs[cont], coords);

			scanf("%d,%d,%d", &x, &y, &z);

			fill_object(&fixeds[cont], x, z, coords);
			WORLD_SIDE = max(x, WORLD_SIDE);
			WORLD_SIDE = max(z, WORLD_SIDE);

			cont--;
		}
	}

	scanf("%s", texto);
	if (strcmp(texto, "Premios") == 0) {
		scanf("%d", &cont);
		DYNAMIC_OBJECTS_NUMBER = cont;

		scanf("%s", texto);
		obj = (objObj *) malloc(sizeof (objObj));
		openObj(texto, obj);
	}

	WORLD_SIDE += 10;
}

void
generatePoints(void)
{
	int i;
	float x, y;
	float coords[6] = { -INFINITY, -INFINITY, -INFINITY, INFINITY, INFINITY, INFINITY };
	getCoords(obj, coords);

	coins = (struct object *) malloc(sizeof(struct object) * DYNAMIC_OBJECTS_NUMBER);

	for (i = 0; i < DYNAMIC_OBJECTS_NUMBER; i++) {
		do {
			x = 2 * (rand() % WORLD_SIDE) - WORLD_SIDE;
			y = 2 * (rand() % WORLD_SIDE) - WORLD_SIDE;
		} while(collided_fixed(x, y));
		fill_object(&coins[i], x, y, coords);
	}
}

void
timer(int t)
{
	if(playing && t == game) {
		gametime++;
		glutTimerFunc(1000, timer, game);
	}
}
