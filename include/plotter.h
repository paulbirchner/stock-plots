#ifndef PLOTTER_H
#define PLOTTER_H

#include <gtk/gtk.h>
#include "parse_json.h"



void draw_candle_series(GtkDrawingArea *widget, cairo_t *cr, int width, int height, gpointer user_data);

#endif