#ifndef WIDGET_FUNCTIONS_H
#define WIDGET_FUNCTIONS_H

#include <gtk/gtk.h>
#include "plotter.h"

void on_list_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
void on_button_toggled(GtkToggleButton *button, gpointer user_data);
stock_chart *find_chart_by_symbol(GList *charts, const char *symbol);
void on_selection_changed(GtkListBox *box, gpointer user_data);
gboolean on_tick(gpointer user_data);
void update_layout(AppState *app_state);
void on_window_resize(GObject *object, GParamSpec *pspec, gpointer user_data);


#endif