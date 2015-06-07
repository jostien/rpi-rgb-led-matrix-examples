/* pong, Copyright (c) 2003 Jeremy English <jenglish@myself.com>
 * A pong screen saver
 *
 * Modified by Brian Sawicki <sawicki@imsa.edu> to fix a small bug.
 * Before this fix after a certain point the paddles would be too
 * small for the program to effectively hit the ball.  The score would
 * then skyrocket as the paddles missed most every time. Added a max
 * so that once a paddle gets 10 the entire game restarts.  Special
 * thanks to Scott Zager for some help on this.
 *
 * Modified by Trevor Blackwell <tlb@tlb.org> to use analogtv.[ch] display.
 * Also added gradual acceleration of the ball, shrinking of paddles, and
 * scorekeeping.
 *
 * Modified by Gereon Steffens <gereon@steffens.org> to add -clock and -noise
 * options. See http://www.burovormkrijgers.nl (ugly flash site,
 * navigate to Portfolio/Browse/Misc/Pong Clock) for the hardware implementation
 * that gave me the idea. In clock mode, the score reflects the current time, and
 * the paddles simply stop moving when it's time for the other side to score. This
 * means that the display is only updated a few seconds *after* the minute actually
 * changes, but I think this fuzzyness fits well with the display, and since we're
 * not displaying seconds, who cares. While I was at it, I added a -noise option
 * to control the noisyness of the display.
 *
 * Modified by Jost Neigenfind <jostie@gmx.de> to adapt code to raspberrypi led
 * matrix.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 */

/*
 * TLB sez: I haven't actually seen a pong game since I was about 9. Can
 * someone who has one make this look more realistic? Issues:
 *
 *  - the font for scores is wrong. For example '0' was square.
 *  - was there some kind of screen display when someone won?
 *  - did the ball move smoothly, or was the X or Y position quantized?
 *
 * It could also use better player logic: moving the paddle even when the ball
 * is going away, and making mistakes instead of just not keeping up with the
 * speeding ball.
 *
 * There is some info at http://www.mameworld.net/discrete/Atari/Atari.htm#Pong
 *
 * It says that the original Pong game did not have a microprocessor, or even a
 * custom integrated circuit. It was all discrete logic.
 *
 */

#include "led-matrix.h"

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int s[10][5][3] = {
{
  {1,1,1},
  {1,0,1},
  {1,0,1},
  {1,0,1},
  {1,1,1}
},
{
  {0,0,1},
  {0,0,1},
  {0,0,1},
  {0,0,1},
  {0,0,1}
},
{
  {1,1,1},
  {0,0,1},
  {1,1,1},
  {1,0,0},
  {1,1,1}
},
{
  {1,1,1},
  {0,0,1},
  {1,1,1},
  {0,0,1},
  {1,1,1}
},
{
  {1,0,1},
  {1,0,1},
  {1,1,1},
  {0,0,1},
  {0,0,1}
},
{
  {1,1,1},
  {1,0,0},
  {1,1,1},
  {0,0,1},
  {1,1,1}
},
{
  {1,1,1},
  {1,0,0},
  {1,1,1},
  {1,0,1},
  {1,1,1}
},
{
  {1,1,1},
  {0,0,1},
  {0,0,1},
  {0,0,1},
  {0,0,1}
},
{
  {1,1,1},
  {1,0,1},
  {1,1,1},
  {1,0,1},
  {1,1,1}
},
{
  {1,1,1},
  {1,0,1},
  {1,1,1},
  {0,0,1},
  {0,0,1}
}};

typedef struct _paddle {
  int x;
  int y;
  int w;
  int h;
  int wait;
  int lock;
  int score;
} Paddle;

typedef struct _ball {
  int x;
  int y;
  int w;
  int h;
} Ball;

struct state {
  Paddle l_paddle;
  Paddle r_paddle;
  Ball ball;
  int bx,by;
  int m_unit;
  int paddle_rate;
  double noise;
};

int width = 32;
int height = 32;

Canvas *canvas;

static void hit_top_bottom(struct state *st) {
  if ((st->ball.y <= 0) || (st->ball.y+st->ball.h >= height))
    st->by=-st->by;
}

static void reset_score(struct state * st){
  st->r_paddle.score = 0;
  st->l_paddle.score = 0;
}

static void new_game(struct state *st){
  /* Starts a Whole New Game*/
  st->ball.x = width/2;
  st->ball.y = height/2;

  st->bx = st->m_unit;
  st->l_paddle.wait = 1;
  st->l_paddle.lock = 0;
  st->r_paddle.wait = 0;
  st->r_paddle.lock = 0;
  if (rand() % 2 == 0){
    st->bx = -st->m_unit;
    st->l_paddle.wait = 0;
    st->r_paddle.wait = 1;
  }
  st->by = st->m_unit;
  if (rand() % 2 == 0)
    st->by = -st->m_unit;

  st->ball.y += (rand() % (height/6))-(height/3);

  st->paddle_rate = st->m_unit-1;
  reset_score(st);

  st->l_paddle.h = height/4;
  st->r_paddle.h = height/4;
}

static void start_game(struct state *st) {
  /*Init the ball*/
  st->ball.x = width/2;
  st->ball.y = height/2;

  st->bx = st->m_unit;
  st->l_paddle.wait = 1;
  st->l_paddle.lock = 0;
  st->r_paddle.wait = 0;
  st->r_paddle.lock = 0;
  if (rand() % 2 == 0){
    st->bx = -st->m_unit;
    st->l_paddle.wait = 0;
    st->r_paddle.wait = 1;
  }
  st->by = st->m_unit;
  if (rand() % 2 == 0)
    st->by = -st->m_unit;

  st->ball.y += (rand() % (height/6))-(height/3);

  st->paddle_rate = st->m_unit-1;

  if (st->l_paddle.h > 10)
    st->l_paddle.h= st->l_paddle.h*19/20;
  if (st->r_paddle.h > 10)
    st->r_paddle.h= st->r_paddle.h*19/20;
}

static void hit_paddle(struct state *st){
  if (st->ball.x + st->ball.w >= st->r_paddle.x && st->bx > 0){ //we are traveling to the right
    if (st->ball.y + st->ball.h > st->r_paddle.y && st->ball.y < st->r_paddle.y + st->r_paddle.h){
      st->bx=-st->bx;
      st->l_paddle.wait = 0;
      st->r_paddle.wait = 1;
      st->r_paddle.lock = 0;
      st->l_paddle.lock = 0;
    } else {
      st->l_paddle.score++;
      if (st->l_paddle.score >=10)
	new_game(st);
      else 
	start_game(st);
    }
  }

  if (st->ball.x <= st->l_paddle.x + st->l_paddle.w && st->bx < 0 ){ //we are traveling to the left
    if (st->ball.y + st->ball.h > st->l_paddle.y && st->ball.y < st->l_paddle.y + st->l_paddle.h){
      st->bx=-st->bx;
      st->l_paddle.wait = 1;
      st->r_paddle.wait = 0;
      st->r_paddle.lock = 0;
      st->l_paddle.lock = 0;
    } else {
      st->r_paddle.score++;
      if (st->r_paddle.score >= 10)
	new_game(st);
      else
	start_game(st);
    }
  }
}

static void *pong_init (){
  struct state *st = (struct state *) calloc (1, sizeof(*st));

  // Init the paddles
  st->l_paddle.x = 0;
  st->l_paddle.y = 15;
  st->l_paddle.w = 2;
  st->l_paddle.h = 8;
  st->l_paddle.wait = 1;
  st->l_paddle.lock = 0;
  st->r_paddle = st->l_paddle;
  st->r_paddle.x = 30;
  st->r_paddle.wait = 0;

  // Init the ball
  st->ball.x = 15;
  st->ball.y = 15;
  st->ball.w = 2;
  st->ball.h = 2;

  st->m_unit = 2; // must be greater than 1 sine otherwise there is no paddle movement

  reset_score(st);

  start_game(st);

  return st;
}

static void p_logic(struct state *st, Paddle *p){
  int targ;

  if (st->bx > 0){
    targ = st->ball.y + st->by*(st->r_paddle.x-st->ball.x)/st->bx;
  } else if (st->bx < 0){
    targ = st->ball.y - st->by*(st->ball.x - st->l_paddle.x - st->l_paddle.w)/st->bx;
  } else {
    targ = st->ball.y;
  }
  if (targ > height)
    targ = height;
  if (targ < 0)
    targ = 0;

  if (targ < p->y && !p->lock){
    p->y -= st->paddle_rate;
  } else if (targ > (p->y + p->h) && !p->lock){
    p->y += st->paddle_rate;
  } else {
    int move = targ - (p->y + p->h/2);
    if (move > st->paddle_rate)
      move = st->paddle_rate;
    if (move < -st->paddle_rate)
      move = -st->paddle_rate;
    p->y += move;
    p->lock = 1;
  }
}

static void p_hit_top_bottom(Paddle *p){
  if(p->y <= 0){
    p->y = 0;
  }
  if((p->y + p->h) >= height){
    p->y = height - p->h;
  }
}

static void draw_solid(int x, int w, int y, int h, int red, int green, int blue){
  for (int j = 0; j < w; j++){
    for (int i = 0; i < h; i++) {
      canvas->SetPixel(x + j, y + i, red, green, blue);
    }
  }
}

static void draw_area(int index, int x, int w, int y, int h, int red, int green, int blue){
  for (int j = 0; j < w; j++){
    for (int i = 0; i < h; i++) {
      if (s[index][i][j] > 0)
	canvas->SetPixel(x + j, y + i, red, green, blue);
    }
  }
}

static void paint_paddle(struct state *st, Paddle *p){
  draw_solid(p->x, p->w, p->y, p->h, 255, 255, 255);
}

static void erase_ball(struct state *st){
  draw_solid(st->ball.x, st->ball.w, st->ball.y, st->ball.h, 0, 0, 0);
}

static void paint_ball(struct state *st){
  draw_solid(st->ball.x, st->ball.w, st->ball.y, st->ball.h, 255, 255, 255);
}

static void paint_net(struct state *st){
  int x,y;

  x = (0 + 32)/2;
  for (y = 0; y < 32; y+=6) {
    draw_solid(x-1, 2, y, 3, 255, 255, 255);
    //draw_solid(x-1, 2, y+3, 3, 255, 255, 255);
  }
}

static void paint_score(struct state *st){
  draw_area(st->l_paddle.score, 9, 3, 1, 5, 255, 255, 255);
  draw_area(st->r_paddle.score, 20, 3, 1 ,5, 255, 255, 255);
}

static void pong_draw (state *st){
  erase_ball(st);

  st->ball.x += st->bx;
  st->ball.y += st->by;

  if ((random()%100)==0){
    if (st->bx>0)
      st->bx++;
    else
      st->bx--;
  }

  if (!st->r_paddle.wait){
    p_logic(st, &st->r_paddle);
  }
  if (!st->l_paddle.wait){
    p_logic(st, &st->l_paddle);
  }

  p_hit_top_bottom(&st->r_paddle);
  p_hit_top_bottom(&st->l_paddle);

  hit_top_bottom(st);
  hit_paddle(st);

  paint_score(st);
  paint_net(st);

  paint_paddle(st, &st->r_paddle);
  paint_paddle(st, &st->l_paddle);

  paint_ball(st);
}

static void DrawOnCanvas(){
  state *st = (state *)pong_init();
  while(true){
    canvas->Fill(0,0,0);
    pong_draw(st);
    usleep(0.25*1000000);
  }
}

int main(int argc, char *argv[]) {
  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;

  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  int rows = 32;   // A 32x32 display. Use 16 when this is a 16x32 display.
  int chain = 1;   // Number of boards chained together.
  canvas = new RGBMatrix(&io, rows, chain);

  DrawOnCanvas();    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
