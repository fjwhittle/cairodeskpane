/* Copyright (c) 2011 Francis Whittle <FJ.Whittle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "cdp_widgets.h"

static void window_child_removed(GtkContainer *container, GtkWidget *child, gpointer user_data)
{
  GtkWidget *replace_with = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "replace-with"));
  if(replace_with){
    gtk_container_add(container, replace_with);
  } else
    gtk_main_quit();
}

int main (int argc, char **argv)
{
  GtkWidget *deskbox, *cairodeskpane;

  gtk_init(&argc, &argv);

  cairodeskpane = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_decorated(GTK_WINDOW(cairodeskpane), FALSE);
  gtk_window_maximize(GTK_WINDOW(cairodeskpane));
  gtk_window_set_type_hint(GTK_WINDOW(cairodeskpane), GDK_WINDOW_TYPE_HINT_DESKTOP);

  deskbox = deskbox_create();

  GError *vte_error = NULL;
  
  GtkWidget *first_terminal = terminal_create(&vte_error);

  if(vte_error){
    fprintf(stderr, "Error %d creating terminal (%s)\n",
	    vte_error->code,
	    vte_error->message);
    return vte_error->code;
  }

  {
    GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (cairodeskpane));
    GdkVisual *visual = gdk_screen_get_rgba_visual (screen);
    
    if (visual == NULL)
      visual = gdk_screen_get_system_visual (screen);
    
    gtk_widget_set_visual (cairodeskpane, visual);
  }

  gtk_container_add(GTK_CONTAINER(deskbox), first_terminal);

  gtk_container_add(GTK_CONTAINER(cairodeskpane), deskbox);

  g_signal_connect(G_OBJECT(cairodeskpane), "remove", G_CALLBACK(window_child_removed), NULL);

  gtk_widget_show_all(cairodeskpane);

  gtk_main();
  
  return 0;
}
