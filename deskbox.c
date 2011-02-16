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

static gboolean deskbox_draw (GtkWidget    *widget,
			      cairo_t      *cr,
			      gpointer      user_data)
{
  guint
    wwidth = gtk_widget_get_allocated_width(widget),
    wheight = gtk_widget_get_allocated_height(widget);
  
  cairo_pattern_t *strokient = cairo_pattern_create_linear(0, 0, 0, wheight);
  {
    cairo_pattern_add_color_stop_rgba(strokient, 0, 0, 0, 0, 0.5);
    cairo_pattern_add_color_stop_rgba(strokient, 0.25, 0.2, 0.2, 0.2, 0.5);
    cairo_pattern_add_color_stop_rgba(strokient, 0.75, 0.2, 0.2, 0.2, 0.5);
    cairo_pattern_add_color_stop_rgba(strokient, 1, 1, 1, 1, 0.5);
  }

  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR); 
  cairo_paint (cr);

  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  cairo_arc(cr, CDP_ARC_OFFSET, CDP_ARC_OFFSET, 
	    CDP_ARC_OFFSET - 2,
	    M_PI, 3 * M_PI / 2); 
  cairo_arc(cr, wwidth - CDP_ARC_OFFSET, CDP_ARC_OFFSET,
	    CDP_ARC_OFFSET - 2,
	    3 * M_PI / 2, 0);
  cairo_arc(cr, wwidth - CDP_ARC_OFFSET, wheight - CDP_ARC_OFFSET,
	    CDP_ARC_OFFSET - 2,
	    0, M_PI / 2);
  cairo_arc(cr, CDP_ARC_OFFSET, wheight - CDP_ARC_OFFSET,
	    CDP_ARC_OFFSET - 2,
	    M_PI / 2, M_PI);
  cairo_close_path (cr);

  cairo_set_line_width(cr, 2);

  cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.5);

  cairo_stroke_preserve(cr);

  cairo_set_source(cr, strokient);

  cairo_stroke_preserve(cr);

  cairo_pattern_destroy(strokient);

  cairo_set_source_rgba(cr, 0.13, 0.12, 0.11, 0.475);

  cairo_fill(cr);

  GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

  if(child && gtk_widget_get_has_window(child)){
    GdkWindow *child_window = gtk_widget_get_window(child);
    gdouble sourcex, sourcey;

    cairo_push_group(cr);
    {
      gdk_window_coords_to_parent(child_window, 0, 0, &sourcex, &sourcey);

      gdk_cairo_set_source_window(cr, child_window, sourcex, sourcey);

      cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
      cairo_paint(cr);
    }
    cairo_pattern_t *vte_ptn = cairo_pop_group(cr);

    cairo_save(cr);
    {
      cairo_rectangle(cr, sourcex, sourcey,
		      gdk_window_get_width(child_window),
		      gdk_window_get_height(child_window));

      cairo_clip(cr);

      cairo_translate(cr, 1, 0);
      
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      
      cairo_mask(cr, vte_ptn);
    }
    cairo_restore(cr);

    cairo_save(cr);
    {
      cairo_rectangle(cr, sourcex, sourcey,
		      gdk_window_get_width(child_window),
		      gdk_window_get_height(child_window));

      cairo_clip(cr);

      cairo_translate(cr, 0, 1);
      
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      
      cairo_mask(cr, vte_ptn);
    }
    cairo_restore(cr);
      
    cairo_set_source(cr, vte_ptn);     
    cairo_paint(cr);
    cairo_pattern_destroy(vte_ptn);
    
  }

  return TRUE;
}

static void split_deskbox(GtkMenuItem *menuitem, guint orientation)
{
  GtkWidget *widget = 
    GTK_WIDGET(g_object_get_data(G_OBJECT(gtk_widget_get_parent(GTK_WIDGET(menuitem))), "split-deskbox"));

  g_return_if_fail(GTK_IS_WIDGET(widget));

  GError *vte_error = NULL;

  GtkWidget *new_terminal = terminal_create(&vte_error);

  if (vte_error){
    fprintf(stderr, "Error %d creating new terminal (%s)\n", vte_error->code, vte_error->message);
    g_error_free(vte_error);
    return;
  }

  GtkWidget
    *new_pane = deskpaned_create(orientation),
    *new_box = deskbox_create();

  gtk_container_add(GTK_CONTAINER(new_box), new_terminal);

  gtk_paned_pack2(GTK_PANED(new_pane), new_box, TRUE, FALSE);

  g_object_ref(widget);

  g_object_set_data(G_OBJECT(widget), "replace-with", new_pane);

  gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(widget)), widget);

  gtk_paned_pack1(GTK_PANED(new_pane), widget, TRUE, FALSE);
  
  gtk_widget_show_all(new_pane);

  g_object_set_data(G_OBJECT(widget), "replace-with", NULL);
  
  g_object_unref(widget);
}

static gboolean deskbox_popup(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  static GtkWidget *popup = 0;
  if(!popup){
    popup = gtk_menu_new();
    GtkWidget *split_horiz = gtk_menu_item_new_with_label("Split Horizontal");
    gtk_menu_shell_append(GTK_MENU_SHELL(popup), split_horiz);
    g_signal_connect(G_OBJECT(split_horiz), "activate",
		     G_CALLBACK(split_deskbox), (gpointer)GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *split_vert = gtk_menu_item_new_with_label("Split Vertical");
    gtk_menu_shell_append(GTK_MENU_SHELL(popup), split_vert);
    g_signal_connect(G_OBJECT(split_vert), "activate",
		     G_CALLBACK(split_deskbox), (gpointer)GTK_ORIENTATION_VERTICAL);
    gtk_menu_set_title(GTK_MENU(popup), "CairoDeskPane");
    gtk_widget_show_all(popup);
  }

  g_object_set_data(G_OBJECT(popup), "split-deskbox", user_data);

  if(event->button == 3)
    gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL, event->button, event->time);

  return FALSE;
}

static void deskbox_child_realize (GtkWidget *widget, gpointer user_data)
{
  if(gtk_widget_get_has_window(widget)){
    gdk_window_set_composited(gtk_widget_get_window(widget), TRUE);
    g_signal_connect(G_OBJECT(widget), "button-press-event",
		     G_CALLBACK(deskbox_popup), gtk_widget_get_parent(widget));
  }
}

static void deskbox_child_add (GtkContainer *self, GtkWidget *widget, gpointer user_data)
{
  gtk_widget_set_margin_top(widget, CDP_INNER_OFFSET);
  gtk_widget_set_margin_right(widget, CDP_INNER_OFFSET);
  gtk_widget_set_margin_bottom(widget, CDP_INNER_OFFSET);
  gtk_widget_set_margin_left(widget, CDP_INNER_OFFSET);  

  g_signal_connect_after(G_OBJECT(widget), "realize", G_CALLBACK(deskbox_child_realize), NULL);
}

static gboolean deskbox_enter_notify (GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
  GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

  if(gtk_widget_get_can_focus(child))
    gtk_widget_grab_focus(child);

  return FALSE;
}

static void deskbox_child_removed (GtkContainer *container, GtkWidget *child, gpointer user_data)
{
  gtk_widget_destroy(GTK_WIDGET(container));
}

GtkWidget *deskbox_create (void)
{
  GtkWidget *self = gtk_event_box_new();
 
  gtk_widget_set_app_paintable(self, TRUE);

  g_signal_connect(G_OBJECT(self), "draw", G_CALLBACK(deskbox_draw), NULL); 
  g_signal_connect(G_OBJECT(self), "add", G_CALLBACK(deskbox_child_add), NULL); 
  g_signal_connect(G_OBJECT(self), "enter-notify-event", G_CALLBACK(deskbox_enter_notify), NULL);
  g_signal_connect(G_OBJECT(self), "remove", G_CALLBACK(deskbox_child_removed), NULL);
  g_signal_connect(G_OBJECT(self), "button-press-event",
		   G_CALLBACK(deskbox_popup), self);
  return self;
}
