/*   Shows three png images in a loop
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

#include "/home/pi/include/png++/png.hpp"
using namespace png;

using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

png::image<png::rgb_pixel> images[60];

static void LoadMovie(std::string frame, int n){
  for (int k = 0; k < n; k++){
    char buffer[10];
    sprintf(buffer,"%d", k+1);

    std::string framek = frame;
    if (k+1 < 10){
      framek = framek + "0";
    }

    framek = framek + buffer;
    framek = framek + ".png";

    png::image<png::rgb_pixel> image(framek);
    images[k] = image;
  }
}

static void ShowMovie(Canvas *canvas, int n){
  for (int k = 0; k < n; k++){
    png::image<png::rgb_pixel> image = images[k];

    int h = image.get_height();
    int w = image.get_width(); 

    for (int j = 0; j < w; j++){
      for (int i = 0; i < h; i++){
        int red = image.get_pixel(j, i).red;
        int green = image.get_pixel(j, i).green;
        int blue = image.get_pixel(j, i).blue;
        canvas->SetPixel(j, i, red, green, blue);
      }
    }

    usleep(1*100000);
  }
}

static void DrawOnCanvas(Canvas *canvas) {
  LoadMovie(std::string("dancing_beavis_and_butthead_32x32_black/frame"), 3);

  while(true){
      ShowMovie(canvas, 3);
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
