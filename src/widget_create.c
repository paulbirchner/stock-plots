#include "widget_create.h"
#include "plotter.h"
#include "config.h"
#include "widget_functions.h"

#include <gtk/gtk.h>


GtkWidget *create_chart_widget(stock_chart *chart){
    //container
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); //abstand in px

    gtk_widget_set_size_request(box, 300, 250);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    chart->container = box;

    //label
    GtkWidget *label = gtk_label_new(NULL);
    char *markup = g_strdup_printf("<b>%s</b>", chart->symbol);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);

    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), label);

    //drawing area
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_candle_series, chart, NULL);

    gtk_widget_set_hexpand(drawing_area, TRUE);
    gtk_widget_set_vexpand(drawing_area, TRUE);

    gtk_box_append(GTK_BOX(box), drawing_area);

    chart->drawing_area = drawing_area;

    return box;
}

GtkWidget *create_category_header(const char *category_name) {

	GtkWidget *row = gtk_list_box_row_new();
	gtk_widget_add_css_class(row, "category-header");

	gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row), FALSE);
	gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row), FALSE);

	GtkWidget *label = gtk_label_new(NULL);
	char *markup = g_strdup_printf("%s", category_name);
	gtk_label_set_markup(GTK_LABEL(label), markup);
	g_free(markup);

	gtk_widget_set_halign(label, GTK_ALIGN_START);

	gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
	return row;
}


GtkWidget *create_stock_row(char *symbol){

    GtkWidget *row = gtk_list_box_row_new();
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget *symbol_label = gtk_label_new(NULL);

									//bold
    char *markup = g_strdup_printf("<b>%s</b>", symbol);
    gtk_label_set_markup(GTK_LABEL(symbol_label), markup);
    g_free(markup);

    gtk_box_append(GTK_BOX(box), symbol_label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

    g_object_set_data_full(G_OBJECT(row), "symbol", g_strdup(symbol), g_free);

    return row;
}

GtkWidget *create_sidebar(AppState *app_state){

	GtkListBox *main_list = GTK_LIST_BOX(gtk_list_box_new());
	gtk_list_box_set_selection_mode(main_list, GTK_SELECTION_MULTIPLE);

	//Click Controller
	GtkGesture *click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_PRIMARY);//nur auf linksclick reagieren
	//Klick abfangen bevor er bei widget ankommt, sonst wird sofort wieder ausgewählt
	gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(click), GTK_PHASE_CAPTURE);

	g_signal_connect(click, "pressed", G_CALLBACK(on_list_clicked), app_state);
	gtk_widget_add_controller(GTK_WIDGET(main_list), GTK_EVENT_CONTROLLER(click));

	g_signal_connect(main_list, "selected-rows-changed", G_CALLBACK(on_selection_changed), app_state);

	stock_config config = get_stock_config(app_state->config_path);

	for(int i=0;i<config.category_count;i++){
		stock_category *current_category = &config.categories[i];

		GtkWidget *header = create_category_header(current_category->category_name);
		gtk_list_box_append(main_list, header);

		for(int j=0;j<current_category->symbol_count;j++){

			char *current_symbol = current_category->symbols[j];

			GtkWidget *row = create_stock_row(current_symbol);

			gtk_list_box_append(main_list, row);
		}
	}

	free_stock_config(&config);

return GTK_WIDGET(main_list);
}
