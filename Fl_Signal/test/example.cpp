// Fl_CallBack libraries for FLTK toolkit v. 0.1
//
// Copyright 2002 by Roman Kantor.
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



#include <FL/Fl.H>

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Simple_Counter.H>
#include "../Fl_Signal.H"
#include "../Fl_Signal.cpp"



class Settings:public Fl_Window{
public:
	Fl_Simple_Counter font_size_;
	Settings():
		Fl_Window(180,150,"Settings"),
		font_size_(50,30,80,30, "Font Size")
	{
		font_size_.value(10);
		font_size_.step(2);
		end();
	}
} settings; // creating settings window;

//class My_Window;

void suicide(Fl_Widget * w, void *){Fl::delete_widget(w);} //for deleting a window


Fl_Signal<const char *> text_signal("This is a text area.\nChanges are immediately copied\nto all windows during typing...\n");


class My_Window:public Fl_Window{

	Fl_Multiline_Input text_area;
	Fl_Button button;

	////////   Following are callbacks associated with each window
	////////   Because they ate true members of My_Window class,
	////////   they are properly destructed/removed when windows is destructed,
	////////   no need to remove them in window destructor

	Fl_Slot<> send_text;                 // sends new text (pointer) to the other widgets via text_node signal
	Fl_Slot<> accept_text;		           // accepts new text (pointer) from other window usung signal text_node
	Fl_Slot<> accept_font_size;          // accepts new font size from settings.font_size_
	Fl_Call<> redraw_after_font_changed; // when the new font size is set, we have to tell explicitly to redraw (fltk bug?)
	Fl_Call<> new_window_callback;       // creates new window when the button is pressed

public:
	static My_Window * create_window();
	My_Window():Fl_Window(310,260,"Window"),
	text_area(20,20,250,150),
	button(150,190,120,25,"New Window"),

		/////////////////////////////////////////////////
		////// Initialization of callbacks //////////////

		send_text(&text_area,&text_signal), // this is a slot
		////// Sends new text from text_area to text_signal which propagates it further to other windows.
		////// It creates standard slot(using text_signal->value(const char * method) and uses signal text_area.value()
    ////// for evaluation of the parameter.

		//accept_text(&text_node,&text_area,(int (Fl_Multiline_Input::*)(const char*))(&Fl_Multiline_Input::value)),
    accept_text(&text_signal,&text_area),
		////// Accepts new text from text_node using again standard slot text_area.value(const char *)
    ////// and value() method of text_node

		
		accept_font_size(&settings.font_size_, &text_area, (void (Fl_Multiline_Input::*)(uchar))&Fl_Multiline_Input::textsize),
		////// Accpepts font size fron default widget signal "settings.font_size_" (settings.font_size_.value())
		////// by inserting it as a parameter into the text_area.textsize. 
    ////// Note, that we have to use here typecasting-like syntax because there exists two methodds "textsize",
    ////// otherwise we would just write: 
    ////// accept_font_size(&settings.font_size_, &text_area, &Fl_Multiline_Input::textsize


		redraw_after_font_changed(&settings.font_size_, &text_area, &Fl_Widget::redraw),
		////// When settings.font_size_ is changed, we need to tell explicitly the text area to redraw.


		new_window_callback(&button,&My_Window::create_window)
		////// Fl_Callback to create new My_Window when "New Window" button is pressed
	{
		accept_text.disable(&text_area);
		////// we do not need to change the text if it was sent from the same text_area from the same window.

		
		text_area.textsize(settings.font_size_.value()); 
		/////// initializing the size of font according to last value of settings.font_size_

		text_area.value(text_signal.value());
		/////// initializing ine text according to actual value stored in text_node 

		text_area.when(FL_WHEN_CHANGED|FL_WHEN_ENTER_KEY);
		/////// we have to add FL_WHEN_ENTER_KEY to when (fltk bug?)
		
		callback(&suicide);
		////// Standard FLTK callback. When "x" icon of the window is pressed,
		////// the window is destroyed. We use it here instead of default callback
		////// which  hide()-s window not only to spare memory but also to demonstrate 
    ////// that window destruction properly removes all callbacks corresponding 
    ////// to the window and application do not crash.

		end(); // just to be sure...
		show();

	};
};


My_Window * My_Window::create_window(){return new My_Window();};



int main(int argc, char ** argv) {
	settings.show(argc, argv);
	new My_Window();
	new My_Window();
        new My_Window();


	
	
	Fl::run();

	return 0;
};
