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

static void terminal_child_exited (GPid pid, gint status, gpointer data)
{
  VtePty *self_pty = vte_terminal_get_pty_object(VTE_TERMINAL(data));
  vte_pty_close(self_pty);
  gtk_widget_destroy(GTK_WIDGET(data));
  g_object_unref(self_pty);
}

GtkWidget *terminal_create (GError **error)
{
  GtkWidget *self = vte_terminal_new();

  VtePty *self_pty = vte_pty_new(0, error);
  
  if (!self_pty){
    g_free(self);
    return NULL;
  } else {
    GPid c_pid = 0;
    char *c_argv[] = { "dmesg", NULL };
    
    vte_terminal_set_pty_object(VTE_TERMINAL(self), self_pty);
    
    c_argv[0] = getenv("SHELL");
    
    if(g_spawn_async(getenv("HOME"), c_argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
		     (GSpawnChildSetupFunc) vte_pty_child_setup,
		     (void *) self_pty, &c_pid, error)){
      g_child_watch_add(c_pid, terminal_child_exited, self);
    } else {
      vte_pty_close(self_pty);
      g_object_unref(self_pty);
      g_object_unref(self);
      return NULL;
    }
  }

  vte_terminal_set_opacity(VTE_TERMINAL(self), 0);

  vte_terminal_set_scrollback_lines(VTE_TERMINAL(self), 255);

  return self;
}
