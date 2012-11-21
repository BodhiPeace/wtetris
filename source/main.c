#include "system/events.h"
#include "system/display.h"
#include "system/keyboard.h"
#include "system/mouse.h"
#include "utils/file.h"
#include "utils/gl.h"
#include "utils/math.h"
#include "utils/camera.h"
#include "utils/timer.h"
#include "utils/rawdimp.h"
#include "utils/contain.h"
#include "utils/timer.h"
#include "fpscont.h"
#include "board.h"
#include "wave.h"
#include "states.h"
#include "renderer.h"
#include "scene.h"
#include "label.h"
#include <stdio.h>


extern struct Scene scene;

int main(int argc, char* argv[]){


	display.create(640, 480);
	gl_init();
	init_keyboard();
	init_mouse();


	// Load geometry data
	//
	GLuint bo_vertex, bo_index;
	glGenBuffers(1, &bo_vertex);
	glGenBuffers(1, &bo_index);

	GLuint* count_nodes = rawdimp("scene.rawd", bo_vertex, bo_index);
	Geom* nodes = (Geom*)(count_nodes + 1);
	Geom* goem_border = nodes;
	Geom* geom_digits = nodes + 1;



	// Create and load shader programs
	//
	char* src_vs = file.loadText("base.vs");
	char* src_fs = file.loadText("base.fs");

	GLuint vs = gl.createShader(GL_VERTEX_SHADER, src_vs);
	GLuint fs = gl.createShader(GL_FRAGMENT_SHADER, src_fs);

	GLuint program = gl.createProgram(vs, fs);

	glUseProgram(program);



	// Get handle to shader vars
	//
	GLint loc_world = glGetUniformLocation(program, "world");
	GLint loc_view = glGetUniformLocation(program, "view");
	GLint loc_proj = glGetUniformLocation(program, "proj");
	


	// Camera setup - projecton and initial position
	Camera cam;
	cam.loc_view = loc_view;
	cam.loc_proj = loc_proj;
	matIdent(cam.mat_world);
	toggleProjection(&cam);

	float* p = &cam.mat_world[12];
	p[0] = 12; p[1] = 30; p[2] = 40;



	// Hide mouse, and set to center
	init_mouse();
	mouse.hide();
	mouse.setPos(display.getWidth()/2, display.getHeight()/2);

	// Persistent OpenGL state
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT, GL_LINE);

	

	
	// Core struct init
	//
	scene.cam = &cam;
	
	Array objects;
	arrayInit(&objects, 5);
	scene.objects = &objects;
	

	// Visible objects ===================================
	
	Board board;
	boardInit(&board, goem_border, 20, 12);

	arrayAppend(&objects, &board);

	
	Label title;
	labelInit(&title, -12.5, 30, geom_digits);
	char text_title[] = "wtetris";
	labelSet(&title, text_title);
	labelScaleSet(&title, 5);

	arrayAppend(&objects, &title);

	Label info;
	labelInit(&info, -3.5, 20, geom_digits);
	char text_info[] = "start:enter\nquit:q";
	labelSet(&info, text_info);
	labelScaleSet(&info, 2);

	arrayAppend(&objects, &info);

	Label time;
	labelInit(&time, -16.5, -18.0, geom_digits);
	char score_text[] = "time:\n2501";
	labelSet(&time, score_text);
	labelScaleSet(&time, 2);
	
	arrayAppend(&objects, &time);

	Label score;
	labelInit(&score, 26, -18.0, geom_digits);
	char text[] = "score:\n0000";
	labelSet(&score, text);
	labelScaleSet(&score, 2);

	arrayAppend(&objects, &score);

	// ===================================================


	Wave wave;
	wave.amp = 2;
	wave.wlen = 2;
	wave.time = 1000;
	waveLocsSet(&wave, program);
	wavePosSet(&wave, &board.piece);

	scene.wave = &wave;


	Renderer renderer;
	renderer.loc_world = loc_world;

	// Initial game state
	//
	scene.state = st_title;
	//Unit32 time_delta = 0;

	
	// LOOP -----------------------------------------------------------------------
	
	while (1){

		events.process();

		if (keyboard.keyHit('q')){
			break;
		}

		if (keyboard.keyHit('r')){
			toggleProjection(&cam);
		}

		//fpscont(&cam);
		//scene.state(&board);
		scene.state();
		
		wave.time += 0.5;
		waveToProg(&wave);

		cameraWorldToView(&cam);
		glUniformMatrix4fv(loc_view, 1, 0, cam.mat_view);

		render(&renderer, &scene);

		display.update();


		SDL_Delay(16);
	}

	return 0;

}
