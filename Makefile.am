bin_PROGRAMS = stock-plots
AM_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/cJSON

stock_plots_SOURCES = src/main.c src/cmdline.c src/curl.c cJSON/cJSON.c src/parse_json.c src/config.c src/datalogger.c src/plotter.c src/daemon.c src/widget_create.c src/widget_functions.c

stock_plots_CFLAGS = $(GTK4_CFLAGS)

stock_plots_LDADD = $(LIBS) $(LIBCURL_LIBS) $(GTK4_LIBS)