//
// Copyright 2000-2008 by Roman Kantor.
// License: LGPL + fltk exceptions
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// version 2 as published by the Free Software Foundation.
//
// This library is distributed  WITHOUT ANY WARRANTY;
// WITHOUT even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

#include "../fl_html_label.H"
#include "../fl_html_label.cxx"

#include <math.h>


#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Help_View.H>
#include <FL/Fl_Button.H>

Fl_Text_Buffer buffer;

void callback(Fl_Widget *, void *data){
  ((Fl_Text_Editor *)data)->copy_label(buffer.text());
  ((Fl_Text_Editor *)data)->parent()->redraw();

}

int main(int argc, char ** argv) {






  Fl_Double_Window *w= new Fl_Double_Window(400, 400, "Cartesian graphics example");
  Fl_Text_Editor *e = new Fl_Text_Editor(50, 200, 200, 150);
  Fl_Button  *b = new Fl_Button(50, 350, 200, 25, "Update Label");
  b->callback(&callback, e);
  //Fl_Help_View * hw =   new Fl_Help_View(210, 200, 170, 150);
  e->align(FL_ALIGN_TOP);
  e->labeltype(FL_HTML_LABEL);
  e->label("aaaaa");
  buffer.text(e->label());
  //e->labelsize(32);
  //hw->value("a");
  e->buffer(buffer);
  w->end();
  w->show(argc, argv);
	Fl::run();
	return 0;
};

