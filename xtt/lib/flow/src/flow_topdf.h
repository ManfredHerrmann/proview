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

#ifndef cnv_topdf_h
#define cnv_topdf_h

/* cnv_topdf.h -- Postscript output */

using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include "pwr.h"
#include "flow.h"
//#include "cnv_style.h"
//#include "cnv_content.h"

typedef enum {
  cnv_eAlignment_Left,
  cnv_eAlignment_Center
} cnv_eAlignment;

class CnvStyle {
 public:
  CnvStyle() {}
  CnvStyle( string s_font, int s_font_size, int s_indentation,
	    int s_top_offset, int s_bottom_offset, int s_alignment,
	    int s_pagebreak, int s_sidebreak, int s_display_number) :
    font(s_font), font_size(s_font_size), indentation(s_indentation),
    top_offset(s_top_offset), bottom_offset(s_bottom_offset), 
    alignment(s_alignment), pagebreak(s_pagebreak), sidebreak(s_sidebreak),
    display_number(s_display_number)
    {}
  string font;
  int font_size;
  int indentation;
  int top_offset;
  int bottom_offset;
  int alignment;
  int pagebreak;
  int sidebreak;
  int display_number;
};

class CnvIdStyle {
 public:
  CnvIdStyle() {
    h1 = CnvStyle( "Helvetica-Bold-ISOLatin1", 14, 0, 30, 25, cnv_eAlignment_Left, 0, 0, 1);
    h2 = CnvStyle( "Helvetica-Bold-ISOLatin1", 12, 0, 25, 20, cnv_eAlignment_Left, 0, 0, 1);
    h3 = CnvStyle( "Helvetica-Bold-ISOLatin1", 10, 0, 15, 10, cnv_eAlignment_Left, 0, 0, 1);
    text = CnvStyle( "Helvetica-ISOLatin1", 10, 0, 11, 2, cnv_eAlignment_Left, 0, 0, 0);
    boldtext = CnvStyle( "Helvetica-Bold-ISOLatin1", 10, 0, 11, 2, cnv_eAlignment_Left, 0, 0, 0);
    link = CnvStyle( "Helvetica-Oblique-ISOLatin1", 8, 0, 11, 2, cnv_eAlignment_Left, 0, 0, 0);
    image = CnvStyle( "Helvetica-Oblique-ISOLatin1", 10, 0, 10, 0, cnv_eAlignment_Left, 0, 0, 0);
    code = CnvStyle( "Courier-ISOLatin1", 10, 0, 11, 2, cnv_eAlignment_Left, 0, 0, 0);
  }
  CnvStyle h1;
  CnvStyle h2;
  CnvStyle h3;
  CnvStyle text;
  CnvStyle boldtext;
  CnvStyle link;
  CnvStyle image;
  CnvStyle code;
};

class CnvContentElem {
 public:
  CnvContentElem() :
    page_number(0), header_level(0) 
    { strcpy( text, ""); strcpy( subject, "");}

  int page_number;
  int header_level;
  char header_number[40];
  char subject[80];
  char text[80];
};

class CnvContent {
 public:
  CnvContent() {}
  vector<CnvContentElem> tab;
  void add( CnvContentElem& elem);
  int find_link( char *subject, char *text, int *page);
};

#define pdf_cMaxLevel 4
#define pdf_cPageHeight 790
#define pdf_cPageWidth 535
#define pdf_cPageNumX (pdf_cPageWidth - 10)
#define pdf_cPageNumY (pdf_cPageHeight - 13)
#define pdf_cLeftMargin 100
#define pdf_cTopMargin 100
#define pdf_cBottomMargin 50
#define pdf_cCellSize 110
#define pdf_cTmpFile "/tmp/ptmp.ps"

typedef enum {
  pdf_mPrintMode_Pos 	= 1 << 0,
  pdf_mPrintMode_Start	= 1 << 1,
  pdf_mPrintMode_Continue= 1 << 2,
  pdf_mPrintMode_End	= 1 << 3,
  pdf_mPrintMode_KeepY	= 1 << 4,
  pdf_mPrintMode_FixX	= 1 << 5
} pdf_mPrintMode;

typedef enum {
  pdf_eFile_Body,
  pdf_eFile__
} pdf_eFile;

typedef enum {
  pdf_eId_TitlePage,
  pdf_eId_InfoPage,
  pdf_eId_Content,
  pdf_eId_Chapter,
  pdf_eId_TopicL1,
  pdf_eId_TopicL2,
  pdf_eId_TopicL3,
  pdf_eId_Function,
  pdf_eId_Class,
  pdf_eId__
} pdf_eId;


typedef enum {
  pdf_eObjType_Catalog,
  pdf_eObjType_Page,
  pdf_eObjType_Pages,
  pdf_eObjType_Outline,
  pdf_eObjType_OutlineO,
  pdf_eObjType_Content,
  pdf_eObjType_Process,
  pdf_eObjType_Font,
  pdf_eObjType_Resource,
  pdf_eObjType_Image
} pdf_eObjType;

class CnvToPdf;

class CnvPdfObj {
 public:
  CnvPdfObj( CnvToPdf *otopdf, pdf_eObjType otype, int onumber) : topdf(otopdf),
    type(otype), number(onumber), length(0), offset(0), start(0), parent(0), first(0),
    last(0), next(0), prev(0), dest(0), count(0), resource(-1), xobject_cnt(0)
    { strcpy(fontname,""); strcpy( text, "");}

  void print_begin();
  void print_end();
  int print_image();

  CnvToPdf *topdf;
  pdf_eObjType type;
  int number;
  int length;
  int offset;
  int start;
  char fontname[80];
  int parent;
  int first;
  int last;
  int next;
  int prev;
  int dest;
  int count;
  char text[200];
  int resource;
  int xobject_cnt;
  int xobject[40];
};

class CnvToPdf  {
 public:
  CnvToPdf() :
    cf(0), ci(0), prev_ci(0), conf_pass(false), xref_offset(0), im_cnt(0),
    use_outlines(1)
    {
      for ( int i = 0; i < pdf_cMaxLevel; i++)
	header_number[i] = 0;
      for ( int i = 0; i < pdf_eFile__; i++)
	page_number[i] = 0;
      strcpy( previous_chapter, "");
      strcpy( current_chapter, "");

      style[pdf_eId_TitlePage].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 35, 0, 100, 20, cnv_eAlignment_Center, 0, 0, 0);
      style[pdf_eId_TitlePage].h2 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 25, 0, 50, 20, cnv_eAlignment_Center, 0, 0, 0);
      style[pdf_eId_TitlePage].h3 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 15, 0, 25, 10, cnv_eAlignment_Center, 0, 0, 0);
      style[pdf_eId_TitlePage].text = 
	CnvStyle( "Helvetica-ISOLatin1", 8, 0, 9, 1, cnv_eAlignment_Center, 0, 0, 0);
      style[pdf_eId_TitlePage].boldtext = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 8, 0, 9, 1, cnv_eAlignment_Center, 0, 0, 0);
      style[pdf_eId_InfoPage].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 24, 0, 100, 20, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_Chapter].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 24, 0, 24, 40, cnv_eAlignment_Left, 0, 1, 1);
      style[pdf_eId_TopicL2].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 12, 0, 20, 10, cnv_eAlignment_Left, 0, 0, 1);
      style[pdf_eId_TopicL2].h2 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 10, 0, 16, 8, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_TopicL3].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 10, 0, 16, 8, cnv_eAlignment_Left, 0, 0, 1);
      style[pdf_eId_TopicL3].h2 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 8, 0, 16, 5, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_Function].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 24, 0, 24, 20, cnv_eAlignment_Left, 1, 0, 1);
      style[pdf_eId_Class].h1 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 24, 0, 24, 20, cnv_eAlignment_Left, 1, 0, 1);
      style[pdf_eId_Class].h2 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 20, 0, 24, 8, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_Class].h3 = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 16, -40, 24, 6, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_Content].boldtext = 
	CnvStyle( "Helvetica-Bold-ISOLatin1", 10, 0, 11, 1, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_Chapter].text = 
	CnvStyle( "TimesNewRoman-ISOLatin1", 10, 0, 11, 1, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_TopicL1].text = 
	CnvStyle( "TimesNewRoman-ISOLatin1", 10, 0, 11, 1, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_TopicL2].text = 
	CnvStyle( "TimesNewRoman-ISOLatin1", 10, 0, 11, 1, cnv_eAlignment_Left, 0, 0, 0);
      style[pdf_eId_TopicL3].text = 
	CnvStyle( "TimesNewRoman-ISOLatin1", 10, 0, 11, 1, cnv_eAlignment_Left, 0, 0, 0);
    }
  ~CnvToPdf();

  void close();
  void print_text( char *text, CnvStyle& style, int mode = pdf_mPrintMode_Pos);
  void draw_rect( double lw, double x, double y, double w, double h);
  void draw_triangle( double lw, double x, double y, double w, double h);
  void draw_filled_triangle( flow_eDrawType color, double x, double y, double w, double h);
  void draw_arc( double lw, double x, double y, double w, double h,
		  int angle1, int angle2);
  void draw_line( double lw, double x1, double y1, double x2, double y2,
		  int dashed, int gray);
  void draw_text( double x, double y, char *text, int bold, double size);
  void draw_arrow( double x1, double y1, double x2, double y2, double x3, double y3,
		   int gray);
  void print_pagebreak( int print_num);
  void print_content();
  void print_h1( char *text, int hlevel, char *subject);
  void print_h2( char *text);
  void print_h3( char *text);
  void print_horizontal_line();
  int print_image( char *filename);
  int print_image_inline( char *filename);
  void cnv_text( char *to, char *from);
  void set_confpass( bool conf) { 
    conf_pass = conf;
    if ( !conf) {
      // Reset
      for ( int i = 0; i < pdf_cMaxLevel; i++)
	header_number[i] = 0;
      for ( int i = 0; i < pdf_eFile__; i++)
	page_number[i] = 0;
      cf = 0;
      ci = 0;
    }
  }
  void set_ci( int val) { prev_ci = ci; ci = val;}
  void set_cf( int val) { cf = val;}
  void set_filename( int idx, char *name) {
    strcpy( filename[idx], name);
  }
  void open();
  void incr_headerlevel();
  void decr_headerlevel();
  void reset_headernumbers( int level);
  void set_pageheader( char *text);
  void set_useoutlines( int u) { use_outlines = u;}
  char *fontname( CnvStyle& style);

  vector<CnvPdfObj> v_pages;
  vector<CnvPdfObj> v_outline;
  vector<CnvPdfObj> v_content;
  vector<CnvPdfObj> v_font;
  vector<CnvPdfObj> v_resource;
  vector<CnvPdfObj> v_image;
  CnvContent content;
  CnvIdStyle style[pdf_eId__];
  ofstream fp[pdf_eFile__];
  pwr_tFileName filename[pdf_eFile__];
  int cf;
  int ci;
  int prev_ci;
  double x;
  double y;
  int page_number[pdf_eFile__];
  int header_number[pdf_cMaxLevel];
  bool conf_pass;
  char current_chapter[160];
  char previous_chapter[160];
  int start_offset;
  int xref_offset;
  int im_cnt;
  int use_outlines;
};

#endif
