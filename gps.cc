/*   Shows the mandelbrot speed or time from NAVILOCK gps receiver,
 *   provided the scripts gpsspeed.sh and gpsspeed.py as well as
 *   gpstime.sh and gpstime.py are running correctly.
 *   Copyright (C) 2017  Jost Neigenfind
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "led-matrix.h"

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int s[12][7][4] = {
{
  {0,1,1,0},
  {1,0,0,1},
  {1,0,0,1},
  {1,0,0,1},
  {1,0,0,1},
  {1,0,0,1},
  {0,1,1,0}
},
{
  {0,0,1,0},
  {0,1,1,0},
  {0,0,1,0},
  {0,0,1,0},
  {0,0,1,0},
  {0,0,1,0},
  {0,1,1,1}
},
{
  {0,1,1,0},
  {1,0,0,1},
  {0,0,0,1},
  {0,1,1,1},
  {1,0,0,0},
  {1,0,0,0},
  {1,1,1,1}
},
{
  {0,1,1,0},
  {1,0,0,1},
  {0,0,0,1},
  {0,1,1,1},
  {0,0,0,1},
  {1,0,0,1},
  {0,1,1,0}
},
{
  {0,0,1,0},
  {0,1,1,0},
  {0,1,1,0},
  {1,0,1,0},
  {1,1,1,1},
  {0,0,1,0},
  {0,0,1,0}
},
{
  {1,1,1,1},
  {1,0,0,0},
  {1,0,0,0},
  {0,1,1,1},
  {0,0,0,1},
  {1,0,0,1},
  {0,1,1,0}
},
{
  {0,0,1,0},
  {0,1,0,0},
  {1,0,0,0},
  {1,1,1,0},
  {1,0,0,1},
  {1,0,0,1},
  {0,1,1,0}
},
{
  {1,1,1,1},
  {0,0,0,1},
  {0,0,0,1},
  {0,0,1,0},
  {0,0,1,0},
  {0,0,1,0},
  {0,0,1,0}
},
{
  {0,1,1,0},
  {1,0,0,1},
  {1,0,0,1},
  {0,1,1,0},
  {1,0,0,1},
  {1,0,0,1},
  {0,1,1,0}
},
{
  {0,1,1,0},
  {1,0,0,1},
  {1,0,0,1},
  {0,1,1,1},
  {0,0,0,1},
  {0,0,1,0},
  {0,1,0,0}
},
{
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,1,1,0},
  {0,1,0,0}
},
{
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
}};

int width = 32;
int height = 32;

Canvas *canvas;

static void draw_number(int index, int x, int w, int y, int h, int red, int green, int blue){
  for (int j = 0; j < w; j++){
    for (int i = 0; i < h; i++) {
      if (s[index][i][j] > 0)
        canvas->SetPixel(x + j, y + i, red, green, blue);
    }
  }
}

static void draw_chars(char *chars, int len){
  int index = 0;
  for (int i = 0; i < len; i++){
    if (chars[i] == '0')
      index =  0;
    if (chars[i] == '1')
      index =  1;
    if (chars[i] == '2')
      index =  2;
    if (chars[i] == '3')
      index =  3;
    if (chars[i] == '4')
      index =  4;
    if (chars[i] == '5')
      index =  5;
    if (chars[i] == '6')
      index =  6;
    if (chars[i] == '7')
      index =  7;
    if (chars[i] == '8')
      index =  8;
    if (chars[i] == '9')
      index =  9;
    if (chars[i] == '.')
      index = 10;
    if (chars[i] == ' ')
      index = 11;
    draw_number(index, 3+i*5, 4, 2, 7, 255, 255, 255);
  }
}

static void DrawOnCanvas(){
  FILE *f;

  char chars[256];
  int size = 0;
  while(true){
    f = fopen("/home/pi/rpi-rgb-led-matrix-examples/speed.txt", "r");
    fseek(f, 0L, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size > 0){
      fscanf(f, "%s", chars);

      double value;
      sscanf(chars, "%lf", &value); 
      sprintf(chars, "%5.2f", value);

      //canvas->Fill(0,0,0);
      draw_chars(chars, 5);
    }
    fclose(f);

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
