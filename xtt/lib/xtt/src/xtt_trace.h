/* 
 * Proview   $Id: rt_trace.h,v 1.12 2008-10-31 12:51:31 claes Exp $
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
 */

#ifndef xtt_trace_h
#define xtt_trace_h

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef flow_h
#include "flow.h"
#endif

#ifndef flow_api_h
#include "flow_api.h"
#endif


class RtTrace;
class CoWow;
class CoWowTimer;

typedef struct trace_sNode trace_tNode;

struct trace_sNode {
  trace_tNode  *Next;
  RtTrace    	*tractx;
};

class RtTrace {
 public:
  void			*parent_ctx;
  char			name[80];
  flow_tNodeClass	trace_analyse_nc;
  flow_tConClass	trace_con_cc;
  int			trace_started;
  CoWowTimer		*trace_timerid;
  flow_tCtx		flow_ctx;
  flow_tNode		trace_changenode;
  pwr_tObjid		objid;
  double		scan_time;
  void			(*close_cb) (RtTrace *);
  void			(*help_cb) (RtTrace *, const char *);
  void			(*subwindow_cb) (void *parent_ctx, pwr_tObjid objid);
  void			(*display_object_cb) (void *parent_ctx, pwr_tObjid objid);
  void			(*collect_insert_cb) (void *parent_ctx, pwr_tObjid objid);
  int                   (*is_authorized_cb)(void *parent_ctx, unsigned int access); 
  void                  (*popup_menu_cb)(void *parent_ctx, 
					 pwr_sAttrRef attrref,
					 unsigned long item_type,
					 unsigned long utility, 
					 char *arg, int x, int y); 
  int                   (*call_method_cb)(void *parent_ctx, const char *method,
					  const char *filter,
					  pwr_sAttrRef attrref,
					  unsigned long item_type,
					  unsigned long utility, 
					  char *arg);
  trace_tNode		*trace_list;
  pwr_tFileName	       	filename;
  int			version;
  int			m_has_host;
  pwr_tOName   		m_hostname;
  pwr_tAName   		m_plcconnect;
  CoWow			*wow;

  RtTrace( void *parent_ctx, pwr_tObjid objid, pwr_tStatus *status);
  virtual ~RtTrace();

  virtual void pop() {}
  virtual RtTrace *subwindow_new( void *ctx, pwr_tObjid oid, pwr_tStatus *sts) {return 0;}
  virtual void popup_menu_position( int event_x, int event_y, int *x, int *y) {}
  int search_object( char *object_str);
  void swap( int mode);

  void activate_close();
  void activate_print();
  void activate_printselect();
  void activate_savetrace();
  void activate_restoretrace();
  void activate_cleartrace();
  void activate_display_object();
  void activate_collect_insert();
  void activate_open_object();
  void activate_open_subwindow();
  void activate_show_cross();
  void activate_open_classgraph();
  void activate_trace();
  void activate_simulate();
  void activate_view();
  void activate_help();
  void activate_helpplc();

 protected:
  void get_trace_attr( flow_tObject object, 
		       char *object_str, char *attr_str, flow_eTraceType *type,
		       int *inverted, unsigned int *options);
  int get_objid( flow_tObject node, pwr_tObjid *objid);
  int get_attrref( flow_tObject node, pwr_tAttrRef *aref);
  int get_selected_node( flow_tObject *node);
  pwr_tStatus viewsetup();
  pwr_tStatus simsetup();
  pwr_tStatus trasetup();
  int trace_start();
  int trace_stop();
  void changevalue( flow_tNode fnode);
  
  static void get_save_filename( pwr_tObjid window_objid, char *filename);
  static int get_filename( pwr_tObjid window_objid, char *filename,
			   int *has_host, char *hostname, char *plcconnect);
  static int connect_bc( flow_tObject object, char *name, char *attr, 
			 flow_eTraceType type, void **p);
  static int disconnect_bc( flow_tObject object);
  static void trace_scan( void *data);
  static int init_flow( FlowCtx *ctx, void *client_data);
  static int flow_cb( FlowCtx *ctx, flow_tEvent event);
  static void trace_close_cb( RtTrace *child_tractx);
  static char *id_to_str( pwr_tObjid objid);

};

#endif





