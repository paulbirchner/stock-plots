#ifndef WIDGET_CREATE_WIDGET_H
#define WIDGET_CREATE_WIDGET_H

#include <gtk/gtk.h>
#include "plotter.h"

GtkWidget *create_stock_row(char *symbol);
GtkWidget *create_chart_widget(stock_chart *chart);
GtkWidget *create_sidebar(AppState *app_state);
GtkWidget *create_category_header(const char *category_name);

#endif