/*   Shows the mandelbrot set, zooms several times into it at its border,
 *   takes a random element of the set, shows the corresponding julia set,
 *   zooms serval times into it and starts from the beginning. 
 *   Copyright (C) 2014  Jost Neigenfind
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

// Makes histogram of number of iterations. It should be
// possible to improve this method with respect to memory.
static int GetHistogram(int** array, int array_size, int max_iter, int* histogram){
  for (int i = 0; i < max_iter; i++)
    histogram[i] = 0;
  
  for (int i = 0; i < array_size; i++)
    for (int j = 0; j < array_size; j++)
      if (array[i][j] != 0)
        histogram[array[i][j]]++;

  int total = 0;;
  for (int i = 0; i < max_iter; i++)
    total = total + histogram[i];
  
  return total;
}

// Makes hues for number of iterations from points of mandelbrot/julia set and histogram. 
static void GetHue(int** array, int array_size, int max_iter, int* histogram, int total, float* hue){
  for (int i = 0; i < max_iter; i++)
    hue[i] = 0;

  for (int i = 0; i < array_size; i++){
    for (int j = 0; j < array_size; j++){
      int iter = array[i][j];
      if (hue[iter] == 0){
        for (int k = 0; k < iter; k++){
          hue[iter] = hue[iter] + float(histogram[k])/float(total);
        }
      }
    }
  }
}

// Gets color given number of iterations and list of hues.
static int GetColor(int iter, float* hue){
  int max = 0xffffff;
  return int(hue[iter]*float(max));
}

// Gets ratio between inside and outside of set given array.
static float GetRatio(int** array, int x, int y, int w){
  float in = 0;
  float out = 0;
  for (int i = y; i < y + w; i++){
    for (int j = x; j < x + w; j++){
      if (array[i][j] == 0)
        in++;
      else
        out++;
    }
  }

  if (out == 0)
    return 0;

  return in/out;
}

// Gets new and smaller area of current area of set.
static void GetNewArea(int** array, int array_size, float* area, float a, float b){
  float start_x = area[0];
  float start_y = area[1];
  float width = area[2];
  float max_iter = area[3];

  int rx = 0;
  int ry = 0;
  int w = array_size/2;
  float ratio = 0;
  int c = 0;
  while (ratio < a || ratio > b){
    rx = rand() % (array_size - w);
    ry = rand() % (array_size - w);

    ratio = GetRatio(array, rx, ry, w);
    c++;

    // if there is no area which satisfies
    // the constraints take the middle
    if (c > 10000){
      rx = array_size/4;
      ry = array_size/4;
      break;
    }
  }

  area[0] = start_x + rx*(width/float(array_size));
  area[1] = start_y + ry*(width/float(array_size));
  area[2] = width/2;
  area[3] = 3*float(max_iter);
  area[4] = float(rx);
  area[5] = float(ry);
  area[6] = float(w);
}

// Gets points in given interval.
static float* GetInterval(float x, float step_width, int n){
  float* ret = new float[n];
  for (int i = 0; i < n; i++){
    ret[i] = x + i*step_width;
  }

  return ret;
}

// Gets element of set corresponing to array since we want connected julia sets
static float* GetElement(int** array, int array_size, float* area){
  float* ret = new float[2];

  int i = 0;
  int j = 0;
  while (array[i][j] != 0){
    i = rand() % array_size;
    j = rand() % array_size;
  }
  ret[0] = area[0] + j*area[2]/array_size;
  ret[1] = area[1] + i*area[2]/array_size;

  return ret;
}

// Makes mandelbrot set.
static void MakeBrot(int** array, int array_size, float* area){
  float start_x = area[0];
  float start_y = area[1];
  float width = area[2];
  float max_iter = area[3];

  float zx, zy, cX, cY, tmp;
  float* x = GetInterval(start_x, width/float(array_size), array_size);
  float* y = GetInterval(start_y, width/float(array_size), array_size);
  for (int i = 0; i < array_size; i++) {
    for (int j = 0; j < array_size; j++) {
      zx = zy = 0;
      cX = x[j];
      cY = y[i];
      int iter = int(max_iter);
      while (zx * zx + zy * zy < 4 && iter > 0) {
        tmp = zx * zx - zy * zy + cX;
        zy = 2.0 * zx * zy + cY;
        zx = tmp;
        iter--;
      }
      if (iter > 0)
        array[i][j] = iter;
      else
        array[i][j] = 0;
    }
  }
}

// Makes julia set.
static void MakeJulia(int** array, int array_size, float* area, float cX, float cY, float max_iter){
  float start_x = area[0]; //-2.0
  float start_y = area[1]; //-2.0
  float width = area[2];   // 4.0

  float zx, zy, tmp;
  float* x = GetInterval(start_x, width/float(array_size), array_size);
  float* y = GetInterval(start_y, width/float(array_size), array_size);
  for (int i = 0; i < array_size; i++) {
    for (int j = 0; j < array_size; j++) {
      zx = x[j];
      zy = y[i];

      int iter = int(max_iter);
      while (zx * zx + zy * zy < 4 && iter > 0) {
        tmp = zx * zx - zy * zy + cX;
        zy = 2.0 * zx * zy + cY;
        zx = tmp;
        iter--;
      }
      if (iter > 0)
        array[i][j] = iter;
      else
        array[i][j] = 0;
    }
  }
}

// Does everything.
static void DoIt(Canvas *canvas){
  int array_size = 32;
  int max_iter = 570;

  // it should be possible to optimize histogram and
  // hue using an hash map s.t. less memory is needed
  int* histogram = new int[1024*1024*10];
  float* hue = new float[1024*1024*10];
  // two dimensional array for display
  int** array = new int*[array_size];
  for(int i = 0; i < array_size; i++)
    array[i] = new int[array_size];
  // array for containing data
  float* area = new float[7];

  // start values for showing mandelbrot set
  area[0] = -2.25;
  area[1] = -1.50;
  area[2] =  3.00;
  area[3] =  max_iter;

  // make mandelbrot set and zoom into it for five times
  int n = 5;
  for (int k = 0; k < n; k++){
    // make mandelbrot set
    MakeBrot(array, array_size, area);

    // get histogram
    int total = GetHistogram(array, array_size, int(area[3]), histogram);
    // and get hue
    GetHue(array, array_size, int(area[3]), histogram, total, hue);

    // paint mandelbrot set
    for (int j = 0; j < array_size; j++){
      for (int i = 0; i < array_size; i++){
        int color = GetColor(array[i][j], hue);
        int red = (color & 0x00ff0000) >> 16;
        int green = (color & 0x0000ff00) >> 8;
        int blue = (color & 0x000000ff) >> 0;
        canvas->SetPixel(i, j, red, green, blue);
      }
    }
    // wait ten seconds
    usleep(10*1000000);

    // show the smaller cutout but not the last
    if (k < n-1){
      // get cutout
      GetNewArea(array, array_size, area, 0.25, 1.5);

      int rx = int(area[4]);
      int ry = int(area[5]);
      int w = int(area[6]);
      for (int j = 0; j < array_size; j++){
        for (int i = 0; i < array_size; i++){
          if (!((i >= ry) && (i <= ry + w) & (j >= rx) && (j <= rx + w))){
            canvas->SetPixel(i, j, 0, 0, 0);
          }
        }
      }
      // wait three seconds
      usleep(3*1000000);
    }
  }

  // find point with connected julia set
  float* julia_pos = GetElement(array, array_size, area);

  // starting cutout for julia set
  area[0] = -2.00;
  area[1] = -2.00;
  area[2] =  4.00;
  area[3] = max_iter;

  n = 5;
  for (int k = 0; k < n; k++){
    // make julia set
    MakeJulia(array, array_size, area, julia_pos[0], julia_pos[1], area[3]);
    // get corresponding histogram
    int total = GetHistogram(array, array_size, int(area[3]), histogram);
    // get corresponding hue
    GetHue(array, array_size, int(area[3]), histogram, total, hue);
    // paint julia set
    for (int j = 0; j < array_size; j++){
      for (int i = 0; i < array_size; i++){
        int color = GetColor(array[i][j], hue);
        int red = (color & 0x00ff0000) >> 16;
        int green = (color & 0x0000ff00) >> 8;
        int blue = (color & 0x000000ff) >> 0;
        canvas->SetPixel(i, j, red, green, blue);
      }
    }
    // wait ten seconds
    usleep(10*1000000);

    // show the smaller cutout but not the last
    if (k < n-1){
      // get cutout
      GetNewArea(array, array_size, area, 0.25, 1.5);

      int rx = int(area[4]);
      int ry = int(area[5]);
      int w = int(area[6]);
      for (int j = 0; j < array_size; j++){
        for (int i = 0; i < array_size; i++){
          if (!((i >= ry) && (i <= ry + w) & (j >= rx) && (j <= rx + w))){
            canvas->SetPixel(i, j, 0, 0, 0);
          }
        }
      }
      // wait three seconds
      usleep(3*1000000);
    }
  }
}

static void DrawOnCanvas(Canvas *canvas) {
  srand (time(NULL));
  while(true){
    DoIt(canvas);
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
