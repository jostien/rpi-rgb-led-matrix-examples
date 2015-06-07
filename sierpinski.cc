/*    Shows sierpinski triangle.
 *    Copyright (C) 2014  Jost Neigenfind
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

static float* getMiddle(float* p1, float* p2){
  float* ret = new float[2];

  ret[0] = (p1[0] + p2[0])/2;
  ret[1] = (p1[1] + p2[1])/2;

  return ret;
}

// Makes sierpinski triangle.
static void MakeSierpinski(Canvas *canvas, int n){
  float* A = new float[2];
  A[0] = 0; A[1] = -1.7321;
  float* B = new float[2];
  B[0] = -1; B[1] = 0;
  float* C = new float[2];
  C[0] =  1; C[1] = 0;

  float* P = new float[2];
  P[0] = 0; P[1] = -1.7321;

  int** array = new int*[32];
  for(int i = 0; i < 32; i++)
    array[i] = new int[32];
  for (int i = 0; i < 32; i++){
    for (int j = 0; j < 32; j++){
      array[i][j] = 0;
    }
  }

  canvas->Fill(0, 0, 0);
  int* color = new int[3];
  for (int i = 0; i < n; i++){
    int r = rand() % 3;
    if (r == 0){
      P = getMiddle(A, P);
      color[0] = 255; color[1] = 0; color[2] = 0; 
    }
    if (r == 1){
      P = getMiddle(B, P);
      color[0] = 0; color[1] = 255; color[2] = 0;
    }
    if (r == 2){
      P = getMiddle(C, P);
      color[0] = 0; color[1] = 0; color[2] = 255;
    }

    int x = int(P[0]*16) + 15;
    int y = int(P[1]*15) + 28;

    if (x >= 0 && x <= 32 && y >= 0 && y <= 32){
      if (array[x][y] == 0){
        canvas->SetPixel(x, y, color[0], color[1], color[2]);
        usleep(1*1000000);
        array[x][y] = 1;
      }
    }
  }
}

static void DrawOnCanvas(Canvas *canvas) {
  srand (time(NULL));
  while(true){
    MakeSierpinski(canvas, 200000);
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
  Canvas *canvas = new RGBMatrix(&io, rows, chain);

  DrawOnCanvas(canvas);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
