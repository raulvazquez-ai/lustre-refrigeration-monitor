#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "nevera_SistemaFrio.h"

nevera_SistemaFrio_ctx_type* ctx;
int paso_actual = 0;

GtkWidget *entradas[3][4];
GtkWidget *label_resultados, *btn_simular, *btn_parar;

static void cargar_estilos_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css =
        "window { background-color: #E3F2FD; } "
        "label { color: #1565C0; font-family: sans-serif; } "
        "entry { background-color: #FFFFFF; color: #0D47A1; font-weight: bold; font-size: 16px; border-radius: 8px; border: 2px solid #90CAF9; padding: 5px; } "
        "entry:focus { border: 2px solid #1976D2; } "
        "button { background-color: #4CAF50; color: white; border-radius: 10px; font-weight: bold; font-size: 16px; padding: 12px; border: none; transition: background-color 200ms; } "
        "button:hover { background-color: #388E3C; } "
        "button:disabled { background-color: #9E9E9E; color: #E0E0E0; } "
        ".btn-parar { background-color: #F44336; } "
        ".btn-parar:hover { background-color: #D32F2F; } "
        ".titulo { font-size: 24px; font-weight: bold; } "
        ".panel-resultados { background-color: #FFFFFF; border-radius: 12px; padding: 10px; border: 2px solid #BBDEFB; } ";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

const char* formato_alarma(int estado) {
    return estado ? "<span foreground='#D32F2F'><b>🚨 ALERTA</b></span>" : "<span foreground='#388E3C'>✅ OK</span>";
}

static void on_parar_clicked(GtkWidget *widget, gpointer data) {
    char buffer[512];
    sprintf(buffer, "<span size='x-large' foreground='#D32F2F'><b>🛑 Simulación detenida manualmente tras %d ciclos.</b></span>", paso_actual);
    gtk_label_set_markup(GTK_LABEL(label_resultados), buffer);

    gtk_widget_set_sensitive(btn_simular, FALSE);
    gtk_widget_set_sensitive(btn_parar, FALSE);
}

static void on_simular_clicked(GtkWidget *widget, gpointer data) {
    double lecturas[3][4];

    for (int c = 0; c < 3; c++) {
        for (int s = 0; s < 4; s++) {
            const gchar *texto = gtk_entry_get_text(GTK_ENTRY(entradas[c][s]));
            lecturas[c][s] = g_ascii_strtod(texto, NULL);
        }
    }

    double medias[3], media_global;
    int a_rango[3], a_desv[3], a_tend[3], n_diag;

    nevera_SistemaFrio_step(lecturas, medias, &media_global, a_rango, a_desv, a_tend, &n_diag, ctx);

    char buffer[2048];
    sprintf(buffer,
        "<span size='x-large' foreground='#0277BD'><b>📊 RESULTADOS DEL CICLO %d</b></span>\n\n"
        "<span size='large'>🧊 <b>CÁMARA 1:</b>  Media: <b>%.2f°C</b></span>\n"
        "      Rango: %s  |  Desviación: %s  |  Tendencia: %s\n\n"
        "<span size='large'>🧊 <b>CÁMARA 2:</b>  Media: <b>%.2f°C</b></span>\n"
        "      Rango: %s  |  Desviación: %s  |  Tendencia: %s\n\n"
        "<span size='large'>🧊 <b>CÁMARA 3:</b>  Media: <b>%.2f°C</b></span>\n"
        "      Rango: %s  |  Desviación: %s  |  Tendencia: %s\n\n"
        "<span size='large' foreground='#1565C0'>🌍 <b>ESTADO GLOBAL DEL SISTEMA:</b></span>\n"
        "      Media Total: <b>%.2f°C</b>  |  Diagnósticos activos: <b>%d</b>",
        paso_actual + 1,
        medias[0], formato_alarma(a_rango[0]), formato_alarma(a_desv[0]), formato_alarma(a_tend[0]),
        medias[1], formato_alarma(a_rango[1]), formato_alarma(a_desv[1]), formato_alarma(a_tend[1]),
        medias[2], formato_alarma(a_rango[2]), formato_alarma(a_desv[2]), formato_alarma(a_tend[2]),
        media_global, n_diag
    );

    gtk_label_set_markup(GTK_LABEL(label_resultados), buffer);
    paso_actual++;

    char btn_text[50];
    sprintf(btn_text, "🚀 Simular Paso %d", paso_actual + 1);
    gtk_button_set_label(GTK_BUTTON(btn_simular), btn_text);
    gtk_widget_set_sensitive(btn_parar, TRUE);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *grid, *box, *box_resultados, *hbox_botones, *scroll_resultados;

    cargar_estilos_css();

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Panel de Control - FríoGal S.A.");

    gtk_window_set_default_size(GTK_WINDOW(window), 650, 750);
    gtk_container_set_border_width(GTK_CONTAINER(window), 25);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *titulo = gtk_label_new("❄️ <b>FríoGal S.A. - Monitor de Temperaturas</b> ❄️");
    gtk_label_set_use_markup(GTK_LABEL(titulo), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(titulo), "titulo");
    gtk_box_pack_start(GTK_BOX(box), titulo, FALSE, FALSE, 0);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box), grid, FALSE, FALSE, 0);

    for (int c = 0; c < 3; c++) {
        char label_cam[100];
        sprintf(label_cam, "<span size='large'><b>Cámara %d:</b></span>", c + 1);
        GtkWidget *l_cam = gtk_label_new(label_cam);
        gtk_label_set_use_markup(GTK_LABEL(l_cam), TRUE);
        gtk_grid_attach(GTK_GRID(grid), l_cam, 0, c, 1, 1);

        for (int s = 0; s < 4; s++) {
            entradas[c][s] = gtk_entry_new();
            gtk_entry_set_width_chars(GTK_ENTRY(entradas[c][s]), 4);
            gtk_entry_set_alignment(GTK_ENTRY(entradas[c][s]), 0.5);
            gtk_entry_set_text(GTK_ENTRY(entradas[c][s]), "3.0");
            gtk_grid_attach(GTK_GRID(grid), entradas[c][s], s + 1, c, 1, 1);
        }
    }

    hbox_botones = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_halign(hbox_botones, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box), hbox_botones, FALSE, FALSE, 5);

    btn_simular = gtk_button_new_with_label("🚀 Iniciar Simulación (Paso 1)");
    g_signal_connect(btn_simular, "clicked", G_CALLBACK(on_simular_clicked), NULL);
    gtk_widget_set_size_request(btn_simular, 250, 50);
    gtk_box_pack_start(GTK_BOX(hbox_botones), btn_simular, FALSE, FALSE, 0);

    btn_parar = gtk_button_new_with_label("🛑 Detener Sistema");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_parar), "btn-parar");
    g_signal_connect(btn_parar, "clicked", G_CALLBACK(on_parar_clicked), NULL);
    gtk_widget_set_size_request(btn_parar, 200, 50);
    gtk_widget_set_sensitive(btn_parar, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox_botones), btn_parar, FALSE, FALSE, 0);

    box_resultados = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(box_resultados), "panel-resultados");
    gtk_box_pack_start(GTK_BOX(box), box_resultados, TRUE, TRUE, 0);

    scroll_resultados = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_resultados), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box_resultados), scroll_resultados, TRUE, TRUE, 0);

    label_resultados = gtk_label_new("<span size='large' foreground='#757575'><i>El sistema está en espera.\nIntroduce los valores y pulsa el botón para comenzar.</i></span>");
    gtk_label_set_use_markup(GTK_LABEL(label_resultados), TRUE);
    gtk_label_set_justify(GTK_LABEL(label_resultados), GTK_JUSTIFY_CENTER);

    gtk_container_add(GTK_CONTAINER(scroll_resultados), label_resultados);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    ctx = nevera_SistemaFrio_ctx_new_ctx();

    app = gtk_application_new("com.universidad.friogal.animada", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
