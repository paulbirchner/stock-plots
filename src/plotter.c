#include "plotter.h"
#include "parse_json.h"
#include "config.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <math.h>
#include <float.h>
#include <stdio.h>

void draw_candle_series(GtkDrawingArea *widget, cairo_t *cr, int width, int height, gpointer user_data){
	stock_chart *chart = (stock_chart*)user_data;

	if(chart == NULL || chart->data.data == NULL || chart->data.count < 1) return;

	candle_series *series = &chart->data;

	double min_price = DBL_MAX;
	double max_price = -DBL_MAX;

	const char *time_format;

	switch(chart->state->active_range){
		case RANGE_1D:
			time_format = "%H:%M";
			break;
		case RANGE_1W:
			time_format = "%a:%d";
			break;
		case RANGE_1M:
			time_format = "%d.%m";
			break;
		case RANGE_1Y:
			time_format = "%b '%y";
			break;
		default:
			time_format = "%d.%m.%y";
			break;
	}

	//y-skalierung
	for(int i=0; i<series->count; i++){
		if(series->data[i].h > max_price) max_price = series->data[i].h;
		if(series->data[i].l < min_price) min_price = series->data[i].l;
	}

	double price_range = max_price - min_price;
	if(price_range == 0) price_range = 1;

	double margin_right = 60.0; //preise
	double margin_bottom = 30.0; //Zeit
	double margin_top = 20.0;

	double plot_width = width-margin_right;
	double plot_height = height-margin_bottom-margin_top;

	#define TO_Y(price) (margin_top + plot_height - (((price - min_price) / price_range) * plot_height))

	//Grid
	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 10);

	//y-achse
	int num_lines = 10;
	for(int i=0;i<num_lines;i++){
		double ratio = (double)i/num_lines;
		double price = min_price + (price_range * ratio);
		double y = TO_Y(price);

		cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.7);
		cairo_set_line_width(cr, 0.5);
		cairo_move_to(cr, 0, y);
		cairo_line_to(cr, plot_width, y);
		cairo_stroke(cr);

		int length = snprintf(NULL, 0, "%.2f", price); //gibt länge zurück
		char *price_str = malloc(length+1); // \0
		snprintf(price_str, length+1, "%.2f", price);

		cairo_move_to(cr, plot_width+5, y+3);
		cairo_show_text(cr, price_str);
		free(price_str);
	}

	//x-achse
	double label_px_width = 60;
 	int max_labels = (int)(plot_width/label_px_width);
	if(max_labels < 2) max_labels = 2; //min 2 labels
	if(max_labels > series->count) max_labels = series->count; //max 1 label pro kerze

	int step_x_index = series->count/max_labels;
	if (step_x_index < 1) step_x_index = 1;

	for(int i=1; i<series->count; i+=step_x_index){
		double x = ((double)i/series->count) * plot_width;

	    cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.7);
	    cairo_set_line_width(cr, 0.5);
	    cairo_move_to(cr, x, margin_top);
	    cairo_line_to(cr, x, height - margin_bottom);
	    cairo_stroke(cr);

	    time_t time = series->data[i].timestamp;
	    struct tm *tm = localtime(&time);

		char *time_str = get_time_str_length(tm, time_format);

		if (time_str) {
			cairo_move_to(cr, x-10, height-10);
			cairo_show_text(cr, time_str);
			free(time_str);
		}
    }

	//kerzen
	double candle_step = plot_width/series->count; //plot gleichm#ßig aufteilen
	double candle_width = candle_step*0.8;	//candles sollen nicht an einander kleben
	cairo_set_line_width(cr, 1.0);

	for(int i=0; i<series->count; i++){
		candle c = series->data[i];

		double x_center = (i*candle_step) + (candle_step/2.0);
		double x_left = x_center - candle_width/2; //für cairo rectangle

		double y_high = TO_Y(c.h);
		double y_low = TO_Y(c.l);
		double y_open = TO_Y(c.o);
		double y_close = TO_Y(c.c);

		if(c.c >= c.o){					//#88DDA0 Grün
			cairo_set_source_rgb(cr, 0.5333, 0.8667, 0.6275);
		} else {						//#DB6867 Rot
			cairo_set_source_rgb(cr, 0.8588, 0.4078, 0.5294);
		}
		//Docht von high zu low
		cairo_move_to(cr, x_center, y_high);
		cairo_line_to(cr, x_center, y_low);
		cairo_stroke(cr);

		//Körper
		double body_y; //obere kante
		double body_height;

		if(y_open < y_close){
			body_y = y_open;
			body_height = y_close - y_open;
		} else {
			body_y = y_close;
			body_height = y_open - y_close;
		}

		if(body_height < 1) body_height = 1;

	    cairo_rectangle(cr, x_left, body_y, candle_width, body_height);	//linke kante, obere kante, breite nach rechts, höhe nach unten
	    cairo_fill(cr);
	}
}
