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

static gboolean deskpaned_draw (GtkWidget *widget, cairo_t *cr)
{
  if (gtk_widget_get_visible (widget)) {
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
  }

  return TRUE;
}

static void deskpaned_child_removed(GtkContainer *container, GtkWidget *child, gpointer user_data)
{
  GtkWidget *replace_with = GTK_WIDGET(g_object_get_data(G_OBJECT(child), "replace-with"));
  if(replace_with){
    if (!gtk_paned_get_child1(GTK_PANED(container)))
      gtk_paned_pack1(GTK_PANED(container), replace_with, TRUE, FALSE);
    else if (!gtk_paned_get_child2(GTK_PANED(container)))
      gtk_paned_pack2(GTK_PANED(container), replace_with, TRUE, FALSE);
    g_object_set_data(G_OBJECT(child), "replace-with", NULL);
  } else {
    GtkWidget *other = gtk_paned_get_child1(GTK_PANED(container));
    
    if (other == NULL)
      other = gtk_paned_get_child2(GTK_PANED(container));

    g_object_ref(other);
    
    g_signal_handlers_block_by_func(G_OBJECT(container), deskpaned_child_removed, NULL);

    gtk_container_remove(container, other);

    g_signal_handlers_unblock_by_func(G_OBJECT(container), deskpaned_child_removed, NULL);

    g_object_set_data(G_OBJECT(container), "replace-with", other);

    gtk_widget_destroy(GTK_WIDGET(container));

    g_object_unref(other);
  }
}

GtkWidget *deskpaned_create (GtkOrientation orientation)
{
  GtkWidget *self = gtk_paned_new(orientation);

  g_signal_connect(G_OBJECT(self), "draw", G_CALLBACK(deskpaned_draw), NULL); 
  g_signal_connect(G_OBJECT(self), "remove", G_CALLBACK(deskpaned_child_removed), NULL); 

  return self;
}
