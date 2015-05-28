#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "ppm.h"
#include <stdio.h>
#include <unistd.h> //for sleep function

//800, 600
//#define WINDOW_WIDTH  1920
//#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_DUCKS 2

#define rnd() (float)rand() / (float)RAND_MAX

extern "C"
{
#include "fonts.h"
}


#define USE_SOUND
#ifdef USE_SOUND
#include <FMOD/fmod.h>
#include <FMOD/wincompat.h>
#include "fmod.h"
#endif //USE_SOUND

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;
//Structures
struct Vec {
	float x, y, z;
};
struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Game {
Shape box[10];
bool menutest;
~Game()
    {
    }
 Game()
    {
    floor = WINDOW_HEIGHT / 5.0;
    menutest = true;
for(int i=5; i<8; i++)
        {
        box[i].width = 140;
        box[i].height = 40;
        }
        
        box[5].center.x = 400;
        box[5].center.y = 500;
        box[5].center.z = 0;

        box[6].center.x = 400;
        box[6].center.y = 400;
        box[6].center.z = 0;

        box[7].center.x = 400;
        box[7].center.y = 300;
        box[7].center.z = 0;


    }
 
};
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
int check_keys(XEvent *e, Game *game);
void render(Game *game);
void check_resize(XEvent *e);

int main(void)
{

initXWindows();
	init_opengl();
	Game game;
	cleanupXWindows();
	cleanup_fonts();
	#ifdef USE_SOUND
	fmod_cleanup();
	#endif //USE_SOUND
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Duck Hunt");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;

	XSetWindowAttributes swa;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	//XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
			InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void reshape_window(int width, int height)
{
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	set_title();
}

unsigned char *buildAlphaData(Ppmimage *img) {
	// add 4th component to RGB stream...
	int a,b,c;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	//newdata = (unsigned char *)malloc(img->width * img->height * 4);
	newdata = new unsigned char[img->width * img->height * 4];
	ptr = newdata;
	for (int i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0);
		b = *(data+1);
		c = *(data+2);
		*(ptr+0) = a;
		*(ptr+1) = b;
		*(ptr+2) = c;
		*(ptr+3) = (a|b|c);
		ptr += 4;
		data += 3;
	}
	return newdata;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

	//added for background
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//clear the screen
	glClearColor(1.0, 1.0, 1.0, 1.0);
	}
	
	void check_resize(XEvent *e)
{
	if(e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if(xce.width != WINDOW_WIDTH || xce.height != WINDOW_HEIGHT)
	{
		reshape_window(xce.width, xce.height);
	}
}
int check_keys(XEvent *e, Game *game)
{
if (e->type == KeyPress) {
        int key = XLookupKeysym(&e->xkey, 0);
        if (key == XK_Escape) {
            return 1;
        }
         if(key == XK_1)
        {
            game->menutest = false;
            }
         if(key == XK_2)
        {
            game->menutest = false;
            }
      }
    return 0;
}      

/The Display of the game 
void render(Game *game)
{
	float w, h, x, y;
	
	//Drawing Shapes
	glColor3ub(255, 255, 255);
	glBegin(GL_LINES);
	glVertex2f(0.0, game->floor);
	glVertex2f(WINDOW_WIDTH, game->floor);
	glEnd();

	Rect r;
	r.bot = WINDOW_HEIGHT - 550;
	r.left = WINDOW_WIDTH - 715;
	r.center = 0;

//-------------------------------------------------------------------
//Drawing Boxes
	Shape *s;

	if (game->menutest == true) {
       const char* text[3] = {"One Duck Hunt - Press 1", "Two Duct Hunt - Press 2", "      Exit - Press ESC"}; // the Text need fixing to look better.
		for(int i=5; i<8; i++) {
			glColor3ub(90, 140, 90);
			s = &game->box[i];
			glPushMatrix();
			glTranslatef(s->center.x, s->center.y, s->center.z);
			w = s->width;
			h = s->height;
			r.bot = s->height - 50;
			r.left = s->width - 250;
			glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
			glEnd();
			if (i == 5)
				ggprint16(&r, 35, 0x00ffffff, text[0]);
			if (i == 6)
				ggprint16(&r, 35, 0x00ffffff, text[1]);
			if (i == 7)
				ggprint16(&r, 35, 0x00ffffff, text[2]);
			r.bot = s->height - 120;
			r.left = s->width - 170;
			if (i == 5)
				ggprint16(&r, 35, 0x00ffffff, "");
			if (i == 6)
				ggprint16(&r, 35, 0x00ffffff, "");
			if (i == 7)
				ggprint16(&r, 35, 0x00ffffff, "");
			glPopMatrix();
		}
	}

