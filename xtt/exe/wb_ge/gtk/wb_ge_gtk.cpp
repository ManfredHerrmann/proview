/* 
 * Proview   Open Source Process Control.
 * Copyright (C) 2005-2016 SSAB EMEA AB.
 *
 * This file is part of Proview.
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
 * along with Proview. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking Proview statically or dynamically with other modules is
 * making a combined work based on Proview. Thus, the terms and 
 * conditions of the GNU General Public License cover the whole 
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * Proview give you permission to, from the build function in the
 * Proview Configurator, combine Proview with modules generated by the
 * Proview PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every 
 * copy of the combined work is accompanied by a complete copy of 
 * the source code of Proview (the version used to produce the 
 * combined work), being distributed under the terms of the GNU 
 * General Public License plus this exception.
 */

/* wb_ge_gtk.cpp -- graphical editor */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <vector>

#include "pwr.h"

#include <gtk/gtk.h>

typedef void *ldh_tSesContext;

#include "ge_gtk.h"
#include "co_lng.h"
#include "cow_xhelp_gtk.h"
#include "wb_log_gtk.h"

#define wnav_cInitFile "$pwrp_login/wtt_init"

/*  Fallback resources  */

static  GtkWidget *toplevel;
static  GtkWidget *mainwindow;

static void usage()
{
  printf("\nUsage: wb_ge [-l language] [graphname]\n");
}

#if 0
static void ge_close_cb( void *gectx)
{
  delete (Ge *)gectx;
  exit(0);
}
#endif

int main( int argc, char *argv[])
{
  int i;
  pwr_tFileName file;
  char graph_name[80];
  int sts;
  unsigned int opt = ge_mOption_EnableComment;
  Ge *gectx;
  pwr_tFileName fname;

  gtk_init( &argc, &argv);

  setlocale( LC_NUMERIC, "POSIX");
  setlocale( LC_TIME, "en_US");

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW,
					 "default-height", 100,
					 "default-width", 100,
					 "title", "Pwr wb_ge",
					 NULL);


  // Create help window
  CoXHelpGtk *xhelp = new CoXHelpGtk( mainwindow, 0, xhelp_eUtility_Wtt, (int *)&sts);
  CoXHelpGtk::set_default( xhelp);

  if ( argc > 1) {

    for ( i = 1; i < argc; i++) {
      if ( strcmp( argv[i], "-l") == 0) {
	if ( i+1 >= argc) {
	  usage();
	  exit(0);
	}
	Lng::set( argv[i+1]);
	i++;
      }
      else
	strcpy( file, argv[i]);
    }

    if ( file[0] == '@') {
      // Execute script
      pwr_tStatus sts;

      gectx = new GeGtk( NULL, toplevel, 0, 1, 0, NULL);
      sts = gectx->command( file);
      if ( EVEN(sts))
	gectx->message( sts);
    }
    else {      
      // Open graph
      strcpy( graph_name, file);
      gectx = new GeGtk( NULL, mainwindow, 0, 1, opt, graph_name);
      
      sprintf( fname, "@%s.pwr_com", wnav_cInitFile);
      gectx->command( fname);
    }
  }
  else {
    gectx = new GeGtk( NULL, mainwindow, 0, 1, opt, NULL);

    sprintf( fname, "@%s.pwr_com", wnav_cInitFile);
    gectx->command( fname);
  }

  gtk_widget_show_all( toplevel);
  g_object_set( toplevel, "visible", FALSE, NULL);

  new wb_log_gtk( toplevel);

  gtk_main();
  return (0);
}
