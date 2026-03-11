#include "widget_functions.h"
#include "widget_create.h"
#include "parse_json.h"

#include <gtk/gtk.h>


//Fange klick ab vor ListBox (kein Strg+Click um Zeilen abzuwählen)
void on_list_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data) {

	if(n_press != 1)return;

	GtkWidget *list_box = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));

	GtkListBoxRow *row = gtk_list_box_get_row_at_y(GTK_LIST_BOX(list_box), y);

	if(row != NULL){
		if(gtk_list_box_row_is_selected(row)){
			//Zeile ausgewählt -> abwählen
			gtk_list_box_unselect_row(GTK_LIST_BOX(list_box), row);
		} else {
			//Zeile nicht ausgewählt -> auswählen
			gtk_list_box_select_row(GTK_LIST_BOX(list_box), row);
		}

		//Klick wurde erledigt
		gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
	} else {
		g_print("No row found");
	}
}


void on_button_toggled(GtkToggleButton *button, gpointer user_data) {

    if(!gtk_toggle_button_get_active(button)) return;

    AppState *app_state = (AppState *)user_data;
    const char *button_id = gtk_buildable_get_buildable_id(GTK_BUILDABLE(button));

    if(strcmp(button_id, "button-1d") == 0) app_state->active_range = RANGE_1D;
    else if (strcmp(button_id, "button-1w") == 0) app_state->active_range = RANGE_1W;
    else if (strcmp(button_id, "button-1mo") == 0) app_state->active_range = RANGE_1M;
    else if (strcmp(button_id, "button-1y") == 0) app_state->active_range = RANGE_1Y;

    for(GList *l = app_state->charts; l != NULL; l = l->next) {
        stock_chart *chart = (stock_chart *)l->data;

        if (chart->data.data) {
            free(chart->data.data);
            chart->data.data = NULL;
            chart->data.count = 0;
        }

        chart->data = resample_stock_data(chart->symbol,
                                        app_state->log_path,
                                        get_interval(app_state->active_range),
                                        get_cutoff(app_state->active_range));

        if (chart->drawing_area) {
            gtk_widget_queue_draw(GTK_WIDGET(chart->drawing_area));
        }
    }
}

stock_chart *find_chart_by_symbol(GList *charts, const char *symbol) {

    for (GList *l = charts; l != NULL; l = l->next) {
        stock_chart *c = (stock_chart *)l->data;
        if(strcmp(c->symbol, symbol) == 0) return c;
    }
    return NULL;
}


void on_selection_changed(GtkListBox *box, gpointer user_data) {
    AppState *app_state = (AppState *)user_data;

    //aktuelle zeilen holen
    GList *selected_rows = gtk_list_box_get_selected_rows(box);

    //nicht ausgewählte charts löschen
    GList *l = app_state->charts;
    while(l != NULL){
        stock_chart *chart = (stock_chart *)l->data	;
        GList *next = l->next;

        int is_selected = 0;

        for(GList *s = selected_rows; s != NULL; s = s->next){
            GtkListBoxRow *row = (GtkListBoxRow *)s->data;
            char *selected_symbol = (char *)g_object_get_data(G_OBJECT(row), "symbol");
            if(strcmp(selected_symbol, chart->symbol) == 0){
                is_selected = 1;
                break;
            }
        }

        if(!is_selected){
            gtk_flow_box_remove(GTK_FLOW_BOX(app_state->flow_box), chart->container);

            free(chart->symbol);
            if (chart->data.data) free(chart->data.data);
            free(chart);

            app_state->charts = g_list_delete_link(app_state->charts, l);
        }
        l = next;
    }

    for(GList *s = selected_rows; s != NULL; s = s->next) {

        GtkListBoxRow *row = GTK_LIST_BOX_ROW(s->data);
        char *symbol = (char *)g_object_get_data(G_OBJECT(row), "symbol");

        if (find_chart_by_symbol(app_state->charts, symbol) == NULL) {
            //chart nicht gefunden -> neu erstellen
            stock_chart *new_chart = malloc(sizeof(stock_chart));
			new_chart->state = app_state;
            new_chart->symbol = strdup(symbol);
            new_chart->data = resample_stock_data(symbol, app_state->log_path,
                                                    get_interval(app_state->active_range),
                                                    get_cutoff(app_state->active_range));

            GtkWidget *widget_box = create_chart_widget(new_chart);
            new_chart->container = widget_box;

            gtk_flow_box_append(GTK_FLOW_BOX(app_state->flow_box), widget_box);

            app_state->charts = g_list_append(app_state->charts, new_chart);
        }
    }
    update_layout(app_state);
    g_list_free(selected_rows);
}

gboolean on_tick(gpointer user_data){

    AppState *app_state = (AppState*)user_data;

    for (GList *l = app_state->charts; l != NULL; l = l->next) {

        stock_chart *chart = (stock_chart *)l->data;

        if (chart->data.data) {
            free(chart->data.data);
            chart->data.data = NULL;
        }

        chart->data = resample_stock_data(chart->symbol, app_state->log_path,
                                    get_interval(app_state->active_range),
                                    get_cutoff(app_state->active_range));

        if (chart->drawing_area) {
            gtk_widget_queue_draw(chart->drawing_area);
        }
    }

    return G_SOURCE_CONTINUE;
}

void update_layout(AppState *app_state) {
    int count = g_list_length(app_state->charts);
    GtkFlowBox *flow_box = GTK_FLOW_BOX(app_state->flow_box);

	int col_spacing = gtk_flow_box_get_column_spacing(flow_box);
	int row_spacing = gtk_flow_box_get_row_spacing(flow_box);

	int total_width = gtk_widget_get_width(GTK_WIDGET(flow_box));

	GtkWidget *scroll_box = gtk_widget_get_ancestor(GTK_WIDGET(flow_box), GTK_TYPE_SCROLLED_WINDOW);
	int total_height;

	if(scroll_box){
		total_height = gtk_widget_get_height(scroll_box);
	}else{
		total_height = gtk_widget_get_height(GTK_WIDGET(flow_box));
	}

	total_height -= 20;
	total_width -= 20;

    switch (count) {
        case 1:
            gtk_flow_box_set_max_children_per_line(flow_box, 1);
			gtk_flow_box_set_min_children_per_line(flow_box, 1);

            stock_chart *chart = app_state->charts->data;

            gtk_widget_set_size_request(chart->container, total_width, total_height);
            break;
        case 2:
            gtk_flow_box_set_max_children_per_line(flow_box, 1);
            gtk_flow_box_set_min_children_per_line(flow_box, 1);

            for (GList *l = app_state->charts; l != NULL; l = l->next) {
                stock_chart *chart = (stock_chart *)l->data;

				int half_height = (total_height - row_spacing) / 2;

                gtk_widget_set_size_request(chart->container, total_width, half_height);
            }
            break;
        default:
            gtk_flow_box_set_max_children_per_line(flow_box, 2);
            gtk_flow_box_set_min_children_per_line(flow_box, 2);

			int width = (total_width - col_spacing) / 2;
			int height = (total_height - row_spacing) / 2;


            for (GList *l = app_state->charts; l != NULL; l = l->next) {
                stock_chart *chart = (stock_chart *)l->data;

                gtk_widget_set_size_request(chart->container, width, height);
            }
            break;
    }
}

void on_window_resize(GObject *object, GParamSpec *pspec, gpointer user_data){
	AppState *app_state = (AppState*)user_data;
	update_layout(app_state);
}