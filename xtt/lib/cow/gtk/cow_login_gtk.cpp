/* 
 * Proview   $Id: co_login_gtk.cpp,v 1.2 2008-10-31 12:51:30 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include "co_login_msg.h"
#include "cow_login_gtk.h"
#include "co_api.h"
#include "co_dcli.h"

//
//	Callback from the pushbutton.
//
void CoLoginGtk::activate_ok( GtkWidget *w, gpointer data)
{
  CoLogin *loginctx = (CoLogin *)data;
  loginctx->activate_ok();
}

//
//	Callback from the pushbutton.
//
void CoLoginGtk::activate_cancel( GtkWidget *w, gpointer data)
{ 
  CoLogin *loginctx = (CoLogin *)data;
  loginctx->activate_cancel();
}

//
//	Callback when value changed.
//
void CoLoginGtk::valchanged_passwordvalue( GtkWidget *w, gpointer data)
{
  CoLogin *loginctx = (CoLogin *)data;

  gtk_widget_activate( ((CoLoginGtk *)loginctx)->widgets.okbutton);
}

//
//	Callback when value changed.
//
void CoLoginGtk::valchanged_usernamevalue( GtkWidget *w, gpointer data)
{
  CoLogin *loginctx = (CoLogin *)data;
  gtk_widget_grab_focus( ((CoLoginGtk *)loginctx)->widgets.passwordvalue);
}

static gboolean login_action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  return FALSE;
}

static gint login_delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CoLogin *loginctx = (CoLogin *)data;

  loginctx->activate_cancel();
  return TRUE;
}

//
// Constructor
//
CoLoginGtk::CoLoginGtk( void		*wl_parent_ctx,
			GtkWidget       *wl_parent_wid,
			const char   	*wl_name,
			const char   	*wl_groupname,
			void		(* wl_bc_success)( void *),
			void		(* wl_bc_cancel)( void *),
			pwr_tStatus  	*status) :
  CoLogin(wl_parent_ctx,wl_name,wl_groupname,wl_bc_success,wl_bc_cancel,status),
  parent_wid(wl_parent_wid)
{
  const int	window_width = 500;
  const int    	window_height = 200;

  // Create an input dialog
  widgets.toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
						 "default-height", window_height,
						 "default-width", window_width,
						 "title", "Proview Login",
						 NULL);

  g_signal_connect( widgets.toplevel, "delete_event", G_CALLBACK(login_delete_event), this);
  g_signal_connect( widgets.toplevel, "focus-in-event", G_CALLBACK(login_action_inputfocus), this);

  widgets.usernamevalue = gtk_entry_new();
  gtk_widget_set_size_request( widgets.usernamevalue, -1, 20);
  g_signal_connect( widgets.usernamevalue, "activate", 
  		    G_CALLBACK(valchanged_usernamevalue), this);

  GtkWidget *usernamelabel = gtk_label_new("Username");
  gtk_widget_set_size_request( usernamelabel, -1, 20);

  widgets.passwordvalue = gtk_entry_new();
  gtk_widget_set_size_request( widgets.passwordvalue, -1, 20);
  g_signal_connect( widgets.passwordvalue, "activate", 
  		    G_CALLBACK(valchanged_passwordvalue), this);

  GtkWidget *passwordlabel = gtk_label_new("Password");
  gtk_widget_set_size_request( passwordlabel, -1, 20);
  gtk_entry_set_visibility( GTK_ENTRY(widgets.passwordvalue), FALSE);

  pwr_tFileName fname;
  dcli_translate_filename( fname, "$pwr_exe/proview_icon2.png");
  GtkWidget *india_image = gtk_image_new_from_file( fname);

  widgets.okbutton = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( widgets.okbutton, 70, 25);
  g_signal_connect( widgets.okbutton, "clicked", 
  		    G_CALLBACK(activate_ok), this);
  GtkWidget *india_cancel = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( india_cancel, 70, 25);
  g_signal_connect( india_cancel, "clicked", 
  		    G_CALLBACK(activate_cancel), this);

  widgets.label = gtk_label_new("");

  GtkWidget *vbox1 = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox1), usernamelabel, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(vbox1), passwordlabel, FALSE, FALSE, 15);

  GtkWidget *vbox2 = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox2), widgets.usernamevalue, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(vbox2), widgets.passwordvalue, FALSE, FALSE, 15);

  GtkWidget *hbox = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(hbox), india_image, FALSE, FALSE, 25);
  gtk_box_pack_start( GTK_BOX(hbox), vbox1, FALSE, FALSE, 15);
  gtk_box_pack_end( GTK_BOX(hbox), vbox2, TRUE, TRUE, 15);

  GtkWidget *india_hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(india_hboxbuttons), widgets.okbutton, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(india_hboxbuttons), india_cancel, FALSE, FALSE, 0);

  GtkWidget *india_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(india_vbox), hbox, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(india_vbox), widgets.label, FALSE, FALSE, 5);
  gtk_box_pack_start( GTK_BOX(india_vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(india_vbox), india_hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(widgets.toplevel), india_vbox);

  gtk_widget_show_all( widgets.toplevel);
  gtk_widget_grab_focus( widgets.usernamevalue);

  *status = 1;
}

//
// Destructor
//
CoLoginGtk::~CoLoginGtk()
{
  /* Destroy the widget */
  gtk_widget_destroy( widgets.toplevel);
}

//
//	Get values in username and password.
//
pwr_tStatus CoLoginGtk::get_values()
{
  char	passwd[40];
  char	username[40];
  char	*value;
  pwr_tStatus	sts;

  /* Get UserName */
  value = gtk_editable_get_chars( GTK_EDITABLE(widgets.usernamevalue), 0, -1);
  strcpy(username, value);
  g_free( value);

  /* Get Password */
  value = gtk_editable_get_chars( GTK_EDITABLE(widgets.passwordvalue), 0, -1);
  strcpy(passwd, value);

  sts = user_check( groupname, username, passwd);
  if ( EVEN(sts))
    return sts;

  return LOGIN__SUCCESS;
}

//
//	Displays a message in the login window.
//
void CoLoginGtk::message( const char *new_label)
{
  gtk_label_set_text( GTK_LABEL(widgets.label), new_label);
}
