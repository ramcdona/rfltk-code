// Fl_Fignal source file
//
// Copyright 2003 Roman Kantor
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "rkantor at tcd dot ie".
//



#include "Fl_Signal.H"
#include <FL/Fl_Group.H>

const void * _fl_signal_sender = 0;

void cb_widget_wrap(Fl_Widget * w, void * data){
  bool me = 0;
  if(! _fl_signal_sender){
	  _fl_signal_sender = w;
    me = 1;
  }
	Fl_Call_ * c = (Fl_Call_ *) data;
	Fl_Call_ * cur =c;
	do{
		cur->next_->call();
		cur = cur->next_;
	}while(cur!=c);
  if(me)
    _fl_signal_sender = 0;
};


void widget_update_callback(void * n, Fl_Call_ * what,Fl_Call_ * by){
	Fl_Call_ * is = (Fl_Call_ *)(((Fl_Widget *)n)->user_data());
	if(is == what)
		if(what==by) //it is the last callback - removing...
			((Fl_Widget *)n)->callback(&Fl_Widget::default_callback);
		else
			((Fl_Widget *)n)->user_data(by);	
};


void node_update_callback(void * n, Fl_Call_ * what,Fl_Call_ * by){
	if((Fl_Call_ *)(((Fl_Signal_ *)n)->callbacks) == what)
		if(what==by) //it is the last callback - removing ....
			((Fl_Signal_ *)n)->callbacks=0;
		else
			((Fl_Signal_ *)n)->callbacks=by;	
};


Fl_Call_ * get_callback(Fl_Widget * w){
	if(w->callback() == &cb_widget_wrap)
		return (Fl_Call_ *)(w->user_data());
	else
		return 0;
};


Fl_Call_ * get_callback(Fl_Signal_ * n){
	return n->callbacks;
};

R_Callback * set_callback(Fl_Widget * w, Fl_Call_ * c){
	w->callback(&cb_widget_wrap, c);
	return &widget_update_callback;
};

R_Callback * set_callback(Fl_Signal_ * n, Fl_Call_ * c){
	n->callbacks = c;
	return &node_update_callback;
};


void attach_callback(Fl_Widget * w, Fl_Call_ * c){
	c->next_=c->prev_=c;
	if(w->callback() == &cb_widget_wrap){
		Fl_Call_ * last =(Fl_Call_ *)(w->user_data());
		if(last){
			c->next_=last->next_;
			c->prev_=last;
			last->next_->prev_=c;
			last->next_=c;
		}
		w->user_data(c);
	}else
		w->callback(&cb_widget_wrap, c);
};

void Fl_Signal_::detach_callbacks(){
	if(!callbacks) return;
	Fl_Call_ * c =  callbacks;
	do{
		c->next_->node_=0;
		c =  c->next_;
	}while	(c!=callbacks);
};

void fl_detach_callbacks(Fl_Widget * w){
	if(w->callback() != &cb_widget_wrap) return;
	Fl_Call_ * c = (Fl_Call_ *) w->user_data();
	if(!c) return;
	Fl_Call_ * cur =c;
		do{
		cur->next_->node_=0;
		cur =  cur->next_;
	}while	(cur!=c);
	w->callback(&Fl_Widget::default_callback);
};

void Fl_Signal_::execute(){
	if(!callbacks) return;
	Fl_Call_ * cur = callbacks;
	do{
		cur = cur->next_;
		cur->call();
	}while (cur!=callbacks);
}

Fl_Call_::~Fl_Call_(){
	if(node_)
		(*update_)(node_,this, this->prev_);
	prev_->next_=next_;
	next_->prev_=prev_;
};


