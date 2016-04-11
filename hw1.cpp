//Ryan Frapwell Homework 1
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
        #include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 5000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//check variblei
int showbubbles = 0;
int cycle = 5;
//Structures

struct Vec {
        float x, y, z;
};

struct Shape {
        float width, height;
        float radius;
        Vec center;
};

struct Particle {
        Shape s;
        Vec velocity;
};

struct Game {
        Shape box[5];
        Particle particle[MAX_PARTICLES];
        int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void physics(Game *game);

//-----------------------------------------------------------------------------
//Setup timers
const double physicsRate = 1.0 / 30.0;
const double oobillion = 1.0 / 1e9;
struct timespec timeStart, timeCurrent;
struct timespec timePause;
double physicsCountdown=0.0;
double timeSpan=0.0;
unsigned int upause=0;
double timeDiff(struct timespec *start, struct timespec *end) {
	return (double)(end->tv_sec - start->tv_sec ) +
			(double)(end->tv_nsec - start->tv_nsec) * oobillion;
}
void timeCopy(struct timespec *dest, struct timespec *source) {
	memcpy(dest, source, sizeof(struct timespec));
}
//-----------------------------------------------------------------------------

int main(void)
{
        int done=0;
        srand(time(NULL));
        initXWindows();
        init_opengl();
        //declare game object
        Game game;
        game.n=0;
	
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);

        //declare a box shape
	for(int i = 0; i<5; i++)
	{
        	game.box[i].width = 100;
        	game.box[i].height = 15;
       		
		game.box[i].center.x = 120 + (60*i);
       		game.box[i].center.y = 500 - (100*i);
	}

	game.circle.radius = .5;
        game.circle.center.x = 60 + 9*65;
        game.circle.center.y = 450- 8*60;


        //start animation
        while(!done) {
                while(XPending(dpy)) {
                        XEvent e;
                        XNextEvent(dpy, &e);
                        check_mouse(&e, &game);
                        done = check_keys(&e, &game);
		}

		clock_gettime(CLOCK_REALTIME, &timeCurrent);
		timeSpan = timeDiff(&timeStart, &timeCurrent);
		timeCopy(&timeStart, &timeCurrent);
		physicsCountdown += timeSpan;
		
		while(physicsCountdown >= physicsRate) {
			physics(&game);
			physicsCountdown -= physicsRate;
		}
                movement(&game);
                render(&game);
                glXSwapBuffers(dpy, win);
        }
        cleanupXWindows();
        return 0;
}

void set_title(void)
{
        //Set the window title bar.
        XMapWindow(dpy, win);
        XStoreName(dpy, win, "335 Lab1   LMB for particle");
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
        XSetWindowAttributes swa;
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

void init_opengl(void)
{
        //OpenGL initialization
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        //Initialize matrices
        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        //Set 2D mode (no perspective)
        glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
        //Set the screen background color
        glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y) {
        if (game->n >= MAX_PARTICLES)
                return;
	if(showbubbles == 1){
        	Particle *p = &game->particle[game->n];
        	p->s.center.x = x;
        	p->s.center.y = y;
        	p->velocity.y = 3.0;
		int loop = 0;
		if((cycle%5) == 0){
        		p->velocity.x = 0;
		}
		else if((cycle%5) == 1){
        		p->velocity.x = -1.0;
		}
		else if((cycle%5) == 2){
        		p->velocity.x = 1.0;
		}
		else if((cycle%5) == 3){
        		p->velocity.x = .5;
		}
		else if((cycle%5) == 4){
        		p->velocity.x = -.5;
		}
		loop++;
		cycle++;
        	game->n++;
	} else if(showbubbles == 0){
        	Particle *p = &game->particle[game->n];
        	p->s.center.x = x;
        	p->s.center.y = y;
        	p->velocity.y = -4.0;
        	p->velocity.x =  1.0;
        	game->n++;
	}
}

void physics(Game *game)
{
	if (showbubbles){
		int loop = 0;
		while(loop<3)
		{
			makeParticle(game, 120, 560);
			loop++;
		}
	}
}

void check_mouse(XEvent *e, Game *game)
{
        static int savex = 0;
        static int savey = 0;
        static int n = 0;

        if (e->type == ButtonRelease) {
                return;
        }
        if (e->type == ButtonPress) {
                if (e->xbutton.button==1) {
                        //Left button was pressed
                        int y = WINDOW_HEIGHT - e->xbutton.y;
                        makeParticle(game, e->xbutton.x, y);
                        return;
                }
                if (e->xbutton.button==3) {
                        //Right button was pressed
                        return;
                }
        }
        //Did the mouse move?
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
                savex = e->xbutton.x;
                savey = e->xbutton.y;
                if (++n < 10)
                        return;


        }
}

int check_keys(XEvent *e, Game *game)
{
        //Was there input from the keyboard?
        int key = XLookupKeysym(&e->xkey, 0);
        if (e->type == KeyPress) {
                if (key == XK_Escape) {
                        return 1;
                }
	} else {
	    return 0;
	}
	switch(key){
	    case XK_b:
		showbubbles ^=1;
        	std::cout << "hit b" << std::endl;
	}
		
        return 0;
}
void movement(Game *game)
{
        Particle *p;

        if (game->n <= 0)
                return;

        for (int i=0; i<game->n; i++) {
                p = &game->particle[i];
                p->s.center.x += p->velocity.x;
                p->s.center.y += p->velocity.y;

                //gravity
                p->velocity.y -= 0.2;

                //check for collision with shapes...
                Shape *s[5];
		for(int z=0; z < 5; z++){
                	s[z] = &game->box[z];
		}

		for(int x=0; x < 5; x++){

                	if (p->s.center.y >= s[x]->center.y - (s[x]->height) &&
                    	p->s.center.y <= s[x]->center.y + (s[x]->height) &&
                    	p->s.center.x >= s[x]->center.x - (s[x]->width) &&
                    	p->s.center.x <= s[x]->center.x + (s[x]->width)) {

                        	p->velocity.y = 0;
				if (p->velocity.x == 0){
				    p->velocity.x = 1;
				}
				if (p->velocity.x == -.5){
				    p->velocity.x = -1;
				}
				if (p->velocity.x == .5){
				    p->velocity.x = 1;
				}
                	}	
		}
                //check for off-screen
                if (p->s.center.y < 0.0) {
                        std::cout << "off screen" << std::endl;
                        game->particle[i] = game->particle[game->n-1];
                        game->n--;
                }
        }
}

void render(Game *game)
{
        float w, h;
        glClear(GL_COLOR_BUFFER_BIT);
        //Draw shapes...

        //draw box1
        Shape *s1[5];
	
	for(int x=0; x < 5; x++){
	        glColor3ub(90,140,90);
	        s1[x] = &game->box[x];
 	       	glPushMatrix();
	        glTranslatef(s1[x]->center.x, s1[x]->center.y, s1[x]->center.z);
	        w = s1[x]->width;
	        h = s1[x]->height;
	        glBegin(GL_QUADS);
	       	        glVertex2i(-w,-h);
	                glVertex2i(-w, h);
	                glVertex2i( w, h);
	                glVertex2i( w,-h);
	        glEnd();
	        glPopMatrix();        glColor3ub(90,140,90);
        Shape *c = &game->circle;
        glPushMatrix();
        glTranslatef(c->center.x, c->center.y, c->center.z);
        w = c->width;
        h = c->height;
        glBegin(GL_LINE_LOOP);
        for(int q = 0; q <= 200; q++) {
                double ang = 2*3.14* q / 200;
                double x = 150 * cos(ang);
                double y = 150 * sin(ang);
                glVertex2d(x,y);
        }


        glEnd();
        glPopMatrix();

	}

        //draw all particles here
        glPushMatrix();
        glColor3ub(150,160,220);
        for (int i=0; i<game->n; i++) {
                Vec *c = &game->particle[i].s.center;
                w = 2;
                h = 2;
                glBegin(GL_QUADS);
                        glVertex2i(c->x-w, c->y-h);
                        glVertex2i(c->x-w, c->y+h);
                        glVertex2i(c->x+w, c->y+h);
                        glVertex2i(c->x+w, c->y-h);
                glEnd();
                glPopMatrix();
        }
}

