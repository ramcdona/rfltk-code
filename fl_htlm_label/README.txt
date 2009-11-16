fl_html_label library
(c) 2009 Roman Lantor
License: LGPL + FLTK exceptions, see COPYING.txt


fl_html_label is a small library (about 200 lines) for fltk 1.1.x which defines new labeltype FL_HTML_LABEL
and enables more sophisticated text and label string formatting. In such a case
the label string is treated as html-formated text and all Fl_Help_View features
are available, including images, tables, lists...

Usage:

a) include fl_html.label.cxx in your project and fl_html_label.h where html formatting is required

b) assign html labeltype to your widget like
       widget->labeltype(FL_HTML_LABEL);
   By default, HTML labeltype is assigned to enumeration #15 within labeltype table. If you wish to use
   different enumeration value, assign functions fl_draw_html() and fl_measure_html() to that other value
   using Fl::set_labeltype() function or use eg. fl_define_html_label(FL_FREE_LABELTYPE).

c) assign your html label like
       widget->label("<h1>This is my first<font color=""#00FF00"">HTML</font> label!</h1>");

d) The measurement function tries to format the string with width as narrow as possible.
   This my lead to mmany line breaks with each word placed on separate line. To avoid line breaks
   within text use &nbsp; instead of white-space characters. You can also use <table> tag with
   specified width="xxx" to pre-define minimum width used for the label or its part.

Images:

  You can use (multiple) images within labels. You can use either:

a) in-memory images (like instances of Fl_Bitmap, Fl_Pixmap, Fl_PNG_Image, Fl_JPEG_Image, FL_RGB_Image, Fl_PNM_Image)
   In such a case you need to register the image under a virtual name using fl_name_image() function, eg

      Fl_Shared_Image * im = fl_name_image("/my_image", my_original_image_pointer);

   Note that the path does not need to exist but must be absolute(the easies way is to put slash as the first character
   of the name) After that you can use the image within html string like

        <img src="/my_image" />

   "Naming" the image makes it available for the functions Fl_Shared_Image::get() and Fl_Shared_Image::find() so that
   it can also be used within standard Fl_Help_View widget under specified virtual name. It also increases reference count
  (if allready registered) so release the image if you do not need it any more eg. im->release(); in the example above.
    
b) preloaded images from the disk. Before first use (label draw) you MUST preload image using
        Fl_Shared_Image::get()
   function to avoid reloading with each redraw so that the reference count never reaches 0. Afrter that you can use it
   within the label under its filesystem name. Indeed you can also use fl_name_image wrapper to associate it with
   different name eg if you  use hard-coded strings as the labels. When you do not need the image/label any more, again
   release() it.




Using HTML strings outside labels

  There are also three generic functions
     
     void fl_draw_html(const char * html_text, int x, int y);
     void fl_draw_html(const char * html_text, int x, int y, int w, int h, Fl_Align align, Fl_Image* img =0);
     void fl_measure_html(const char * html_text, int &w, int &h);

  which can be used in any place, not necesarily within labels. However these functions are not suited for multi-page
  drawings because formatting is performed with each call to measurement or drawing function. This formatting
  can be quite expensive so use it only with relatively short strings.


Implementation:
  The implementation uses Fl_Help_View functionality (a single static instance of the class)
  to perform the measurement and drawing. It is relatively dirty because it requires a trick to acces 
  some private members of the class (there is smuggled "friend class..." to the Fl_Help_View when included
  in fl_html_label.cxx) and includes several work-arrounds due to some Fl_Help_View bugs (eg non-accurate
  calculation of ducument height in certain circumstancies). The code compiles and work fine with MS VC
  and gcc, other compilers not tested. In the future (fltk-1.3?) it would make sense to make some methods public
  (or at least protested) although restructuralisation and possibility of plain use uf some generic measurement
  and drawing functions - without the need of the class instantiation - would be prefered.

  



  



