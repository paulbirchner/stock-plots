#include "cmdline.h"
#include "config.h"
#include "widget_create.h"
#include "widget_functions.h"

#include <locale.h>
#include <gtk/gtk.h>
#include <argp.h>

void load_css(){
	GtkCssProvider *provider = gtk_css_provider_new();

	gtk_css_provider_load_from_path(provider, "style.css");

	GdkDisplay *display = gdk_display_get_default();
	gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	g_object_unref(provider);
}

static void activate(GtkApplication *app, gpointer user_data) {

	setlocale(LC_NUMERIC, "C");
	load_css();

    static AppState app_state = {0};
	app_state.active_range = RANGE_1D; //default
    app_state.config_path = get_file_path(CONFIG_FILE);
    app_state.log_path = get_file_path(LOG_FILE);

	GtkBuilder *builder;
	GObject *window;
	GObject *flow_box;
	GError *error = NULL;

	builder = gtk_builder_new();

	if(gtk_builder_add_from_file(builder, "builder.ui", &error) == 0) {
		g_printerr("Error loading builder.ui: %s\n", error->message);
		g_clear_error(&error); // gibt speicher frei und setzt error wieder auf NULL
		return;
	}

	window = gtk_builder_get_object(builder, "main-window"); //top level widget
	gtk_window_set_application(GTK_WINDOW(window), app);

	g_signal_connect(window, "notify::default-width", G_CALLBACK(on_window_resize), &app_state);
	g_signal_connect(window, "notify::default-height", G_CALLBACK(on_window_resize), &app_state);

	flow_box = gtk_builder_get_object(builder, "flow-box");
	if(flow_box){
		app_state.flow_box = GTK_WIDGET(flow_box);
	} else {
		g_printerr("flow_box not found in builder.ui");
	}

	//timerange buttons
	GObject *button_1d = gtk_builder_get_object(builder, "button-1d");
	g_signal_connect(button_1d, "toggled", G_CALLBACK(on_button_toggled), &app_state);

	GObject *button_1W = gtk_builder_get_object(builder, "button-1w");
	g_signal_connect(button_1W, "toggled", G_CALLBACK(on_button_toggled), &app_state);

	GObject *button_1M = gtk_builder_get_object(builder, "button-1mo");
	g_signal_connect(button_1M, "toggled", G_CALLBACK(on_button_toggled), &app_state);

	GObject *button_1Y = gtk_builder_get_object(builder, "button-1y");
	g_signal_connect(button_1Y, "toggled", G_CALLBACK(on_button_toggled), &app_state);

	//stock list
	GtkBox *sidebar_box = GTK_BOX(gtk_builder_get_object(builder, "stock-list"));

	if(sidebar_box){
		GtkWidget *sidebar_list = create_sidebar(&app_state);
		gtk_box_append(sidebar_box, sidebar_list);
	}else {
		g_printerr("sidebar_box not fount in builder.ui");
	}


	g_timeout_add_seconds(10, on_tick, &app_state);

	gtk_window_present(GTK_WINDOW(window));

    g_object_unref(builder);
}

int main(int argc, char **argv) {

struct arguments arguments = {0};

if(parse_commandline(argc, argv, &arguments) != 0){
    return EXIT_FAILURE;
}
if(argc > 1) return EXIT_SUCCESS;

GtkApplication *app;
GtkWidget *window;
int status;

app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
status = g_application_run(G_APPLICATION(app), 0, NULL);
g_object_unref(app);

free(arguments.api_key);

return status;
}