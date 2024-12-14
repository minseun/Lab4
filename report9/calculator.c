#include <gtk/gtk.h>

typedef enum {
  OPERATOR_NONE,
  OPERATOR_ADD,
  OPERATOR_SUBTRACT,
  OPERATOR_MULTIPLY,
  OPERATOR_DIVIDE
} Operator;

double current_value = 0.0;
double previous_value = 0.0;  
Operator current_operator = OPERATOR_NONE;

static void button_clicked(GtkWidget* widget, gpointer data) {
  const gchar* label = gtk_button_get_label(GTK_BUTTON(widget));

  if (g_ascii_isdigit(label[0])) {
    double digit = atof(label);  
    current_value = current_value * 10 + digit; 
  } else {
    switch (label[0]) {
      case '+':
        current_operator = OPERATOR_ADD;
        previous_value = current_value;
        current_value = 0.0;
        break;
      case '-':
        current_operator = OPERATOR_SUBTRACT;
        previous_value = current_value;
        current_value = 0.0;
        break;
      case '*':
        current_operator = OPERATOR_MULTIPLY;
        previous_value = current_value;
        current_value = 0.0;
        break;
      case '/':
        current_operator = OPERATOR_DIVIDE;
        previous_value = current_value;
        current_value = 0.0;
        break;
      case '=':
        switch (current_operator) {
          case OPERATOR_ADD:
            current_value = previous_value + current_value;
            break;
          case OPERATOR_SUBTRACT:
            current_value = previous_value - current_value;
            break;
          case OPERATOR_MULTIPLY:
            current_value = previous_value * current_value;
            break;
          case OPERATOR_DIVIDE:
            if (current_value != 0.0) {
              current_value = previous_value / current_value;
            } else {
              g_print("Error: Division by zero\n");
              current_value = 0.0;
            }
            break;
          case OPERATOR_NONE:
            break;
        }
        break;
      case 'C':  
        current_value = 0.0;
        previous_value = 0.0;
        current_operator = OPERATOR_NONE;
        gtk_entry_set_text(GTK_ENTRY(data), "0.00");  
        return; 
    }
  }

  gchar result_text[20];
  snprintf(result_text, sizeof(result_text), "%.2f", current_value);
  gtk_entry_set_text(GTK_ENTRY(data), result_text);
}

int main(int argc, char* argv[]) {
  gtk_init(&argc, &argv);

  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Calculator");
  gtk_window_set_default_size(GTK_WINDOW(window), 200, 300);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget* grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  GtkWidget* entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), "0.00");
  gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 4, 1);

  gchar* button_labels[] = {"1", "2", "3", "/", "4", "5", "6", "*",
                            "7", "8", "9", "-", "0", ".", "=", "+", "C"};

  for (int i = 0; i < 17; i++) {
    GtkWidget* button = gtk_button_new_with_label(button_labels[i]);
    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), entry);
    gtk_grid_attach(GTK_GRID(grid), button, i % 4, 1 + i / 4, 1, 1);
  }

  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
