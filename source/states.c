#include "states.h"

#include "system/keyboard.h"
#include "utils/timer.h"
#include "scene.h"
#include "label.h"

extern Scene scene;


void st_idle(){
	
}

static void trans_title_falling(){

	float* y = &scene.cam->mat_world[13];
	
	if (*y > -19){
		*y -= 0.25;
	}else{
		scene.state = st_falling;
	}


}


static void trans_falling_title(){

	float* y = &scene.cam->mat_world[13];
	
	if (*y < 30){
		*y += 0.25;
	}else{
		scene.state = st_title;
	}


}


void st_title(){

	if (keyboard.keyHit('\r')){
		scene.state = trans_title_falling;
	}

}


void st_falling(){


	static Timer timer = {1, 0, 0}; // initial 1 second
	static Timer wall_clock = {1, 0, 0};
	static int score = 0;
	int right_left = 0;


	Board* board = (Board*)scene.objects->data[0];
	Label* lbl_time = (Label*)scene.objects->data[3];
	Label* lbl_score = (Label*)scene.objects->data[4];

	timerTick(&wall_clock);
	sprintf(lbl_time->text, "time:\n%04d", wall_clock.count);
	sprintf(lbl_score->text, "score:\n%04d", score);

	right_left = 
		keyboard.keyHit(keyboard.RIGHT) -
		keyboard.keyHit(keyboard.LEFT);
	
	if (keyboard.keyHit(keyboard.UP)){
		if (boardPieceCanRotate(board)){
			boardPieceRotate(board);
		}
	}


	if ( boardPieceCanMove(board, 0, right_left) ){
		boardPieceMove(board, 0, right_left);
	}

	
	if (keyboard.keyHit(keyboard.DOWN)){
		boardPieceDrop(board);
		wavePosSet(scene.wave, &board->piece);
		scene.wave->time = 0;
		timer.tick = 0; // next timerTick(&timer) will return 1
	}



	if (timerTick(&timer)){


		if ( boardPieceCanMove(board, 1, 0) ){

			boardPieceMove(board, 1, 0);

		}else{
			int complete_rows = boardCompleteRows(board);
			if (complete_rows){
				score += complete_rows * complete_rows;
				if (timer.time > 0.1){
					timer.time = 1 + (0.1f - 1) / 150 * score;
				}
				scene.state = st_clearing;
				return;
			}
			if (! boardPieceNew(board) ){
				timerInit(&timer, 1);
				timerInit(&wall_clock, wall_clock.time);
				score = 0;
				boardClear(board);
				boardPieceNew(board);
				scene.state = trans_falling_title;
			}
		}


	}

}


void st_clearing(){

	static Timer timer = {0.11, 0, 0};

	Board* board = (Board*)scene.objects->data[0];

	if (timerTick(&timer)){
		if (timer.count % 2){
			boardHiddenRowsShow(board);
		}else{
			boardCompleteRowsHide(board);
		}
	}

	if (timer.count > 10){
		boardClearComplete(board);
		if ( boardPieceNew(board) ){
			timerInit(&timer, timer.time);
			scene.state = st_falling;
		}
	}

}
