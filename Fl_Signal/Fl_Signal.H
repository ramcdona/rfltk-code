// Fl_Fignal header file
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

#ifndef Fl_Signal_H
#define Fl_Signal_H


#ifdef _MSC_VER
  #define CLASS
#else
  #define CLASS class
#endif

// I am not surehow to recognize VC6...
// VC6 explicitly specializes classes when using as a friend with "class" keyword!!!
// On the other hand new versions of gcc require class keywoard with friend (ANSI C++?)


#include <FL/Fl_Widget.H>


#ifndef SIG_PARAMETERS
	#define SIG_PARAMETERS 5
#endif

class Fl_Call_;

typedef void (R_Callback)(void * n, Fl_Call_ * what, Fl_Call_ * by); //helper function for removal of callbacks from node or widget

typedef void FL_SIGNAL_VOID; // this is work-arround to allow something like template<typename T = void> ;-(

extern const void * _fl_signal_sender;
//extern char res;
//extern char a1_;

/////////////  Base class for node to which callbacks can be attacged (apart from widgets) //////

class Fl_Signal_{
	friend CLASS Fl_Call_;
	friend void node_update_callback(void * n, Fl_Call_ * what,Fl_Call_ * by);
	friend Fl_Call_ * get_callback(Fl_Signal_ * n);
	friend R_Callback * set_callback(Fl_Signal_ * n, Fl_Call_ * c);
	friend void fl_detach_callbacks(Fl_Signal_ * w);

	void detach_callbacks();

	Fl_Signal_(const Fl_Signal_ & );    // no, we realy do not want to use these
  const Fl_Signal_ & operator=(const Fl_Signal_ &);

protected:
	Fl_Call_ * callbacks; // this is last callback, first is callbacks->next_
public:
	void execute();

public:
	Fl_Signal_():callbacks(0){};
	virtual ~Fl_Signal_(){detach_callbacks();};
};



template<typename T = FL_SIGNAL_VOID>
class Fl_Signal:public Fl_Signal_{     // you can attach slots to this node version
protected:
	T p_;
public:
	T value(){ return p_;};

  void value(T p){
    bool me = 0;
    if(!_fl_signal_sender)  {_fl_signal_sender = this; me = 1;}
    p_ = p; execute();
    if(me) _fl_signal_sender = 0;
  };
  void operator ()(T p){/*if(p != p_)*/ value(p);};
	Fl_Signal<T>& operator = (T p){value(p); return * this;}; // needed for result slots
  T operator ()(){return value();}
	Fl_Signal(T p):p_(p){};
};

template<>
class Fl_Signal<void>:public Fl_Signal_{
public:
	void operator ()(){
   bool me = 0;
   if(!_fl_signal_sender) {_fl_signal_sender = this; me = 1;}
   execute();
   if(me) _fl_signal_sender = 0;
  };
};




//////////////////// Base clases for functors  /////////////////////////////////////

class Fl_Cb{

public:
	virtual void call()=0;
	virtual ~Fl_Cb(){};
};


template<typename R>
class Fl_Cb_:public Fl_Cb{
protected:
	R res;      // here is stored return value of the callback

	template<typename C>
	static inline void execute(C  t){ t->exec_r();}; // this is work-arround for partial specialization of templated classes, not supported in VC6,

public:
	R value(){return res;};
};



template<>
class Fl_Cb_<void>:public Fl_Cb{           // explicit specialization for void
protected:

	template<typename C>
	static inline void execute(C  t){t->exec();};
};


///////////  Helper functions, which figure out  what are the parameters //////////
///////////             and how they should be evaluated                 //////////


template <typename R, typename RES>
inline R _get(R v){return v;};

template <typename R, typename RES>
inline R _get(R * v){return *v;};

template <typename R, typename RES>
inline R _get(Fl_Cb &v){return (*reinterpret_cast<RES *>(&v))();};


template<typename P>
class _Ref{
public:
	template<typename T>
	static inline P r(T * p){return p;};
	static inline P * r(Fl_Cb & p){return (P*)(&p);};
  template <typename TT>
  static inline P * r(Fl_Cb_<TT> & p){return (P*)(&p);};
};



//////  CALLBACK FUNCTORS: you do not use them directly  /////////////////////
//////    so user do not need to know about these     /////////////////////

	template <typename R>
	class Fl_Cb_f0:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)();
		void exec_r(){Fl_Cb_<R>::res = (*f_)();};   // only one of of exec/exec_r is implemented,
		void exec(){(*f_)();};           // depending on return type (see function execute in base class)
		void call(){execute(this);};     // and finaly inplementation of functor execution
	public:
		Fl_Cb_f0(R(*f)()):Fl_Cb_<R>(), f_(f){};
	};


	template <typename R, typename R_>
	class Fl_Cb_f0r:public Fl_Cb_f0<R>{
    //friend CLASS Fl_Cb_f0<R>;
    friend CLASS Fl_Cb_<R>;
		R_ r_;
		//void call(){Fl_Cb_f0<R>::call();  *(_Ref<R_>::r(r_)) = Fl_Cb_f0<R>::res;};
    void call(){Fl_Cb_f0<R>::call();  *(_Ref<R_>::r(r_)) = Fl_Cb_<R>::res ;};
	public:
		Fl_Cb_f0r(R(*f)(), R_ r):Fl_Cb_f0<R>(f), r_(r){};
	};


////////////////// methods ///////////

	template <typename R, typename O, typename O_>
	class Fl_Cb_m0:public Fl_Cb_<R>{
		//friend CLASS Fl_Cb_f0<R>;
    friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)();
		O_ o_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)();};
		void exec(){(_get<O*, O_>(o_)->*m_)();};
		void call(){execute(this);};
	public:
		Fl_Cb_m0( O_ o, R(O::*m)()):Fl_Cb_<R>(), m_(m), o_(o){};
		Fl_Cb_m0( O_ o, R(O::*m)()const):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)()>(m)), o_(o){};
	};



	template <typename R, typename O, typename O_, typename R_>
	class Fl_Cb_m0r: public Fl_Cb_m0<R,O,O_>{
    //friend CLASS Fl_Cb_f0r<R,R_>;
    friend CLASS Fl_Cb_<R>;
		R_ r_;
		void call(){Fl_Cb_m0<R,O,O_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_m0<R,O,O_>::res;};
	public:
		Fl_Cb_m0r( O_ o, R(O::*m)(), R_ r):Fl_Cb_m0<R,O,O_>(o,m), r_(r){};
		Fl_Cb_m0r( O_ o, R(O::*m)() const, R_ r):Fl_Cb_m0<R,O,O_>(o,m), r_(r){};
	};


/////////////////////////////////// 1 argument /////////////////////////////

	template <typename R, typename A1, typename A1_>
	class Fl_Cb_f1:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)(A1);
		A1_ a1_;
		void exec_r(){Fl_Cb_<R>::res = (*f_)(_get<A1,A1_>(a1_));};
		void exec(){(*f_)(_get<A1,A1_>(a1_));};
		void call(){execute(this);};
	public:
		Fl_Cb_f1(R(*f)(A1), A1_ a1):Fl_Cb_<R>(), f_(f), a1_(a1){};
  protected: // for slots
    Fl_Cb_f1(R(*f)(A1)):Fl_Cb_<R>(), f_(f){};

	};

	template <typename R, typename A1, typename A1_, typename R_>
	class Fl_Cb_f1r:public Fl_Cb_f1<R,A1,A1_>{
		R_ r_;
		void call(){Fl_Cb_f1<R,A1,A1_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_f1<R,A1,A1_>::res;};
	public:
		Fl_Cb_f1r(R(*f)(A1), A1_ a1, R_ r):Fl_Cb_f1<R,A1,A1_>(f, a1), r_(r){};
	};


	template <typename R, typename A1>
	class Fl_Cb_f1p:public Fl_Cb_f1<R,A1,A1>{
    friend CLASS Fl_Cb_<R>;
  public:
		void operator = (A1 p){((Fl_Cb_f1<R,A1,A1> *)this)->a1_=p; execute(this);};
	public:
		Fl_Cb_f1p(R(*f)(A1)):Fl_Cb_f1<R,A1,A1>(f){};
	};

///////////

	template <typename R, typename A1, typename R_>
	class Fl_Cb_f1rp:public Fl_Cb_f1r<R,A1,A1,R_>{
  public:
		void operator = (A1 p){Fl_Cb_f1r<R,A1,A1,R_>::a1_= p; execute(this);};
	public:
		Fl_Cb_f1rp(R(*f)(A1),R_ r):Fl_Cb_f1r<R,A1,A1,R_>(f,&Fl_Cb_f1r<R,A1,A1,R_>::a1_,r){};
	};


//////////////// methods //////////////

	template <typename R, typename O, typename O_, typename A1, typename A1_>
	class Fl_Cb_m1:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)(A1);
		O_ o_;
		A1_ a1_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_));};
		void exec(){(_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_));};
		void call(){execute(this);};
	public:
		Fl_Cb_m1(O_ o, R(O::*m)(A1), A1_ a1):Fl_Cb_<R>(), m_(m), o_(o), a1_(a1){};
		Fl_Cb_m1(O_ o, R(O::*m)(A1)const, A1_ a1):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1)>(m)), o_(o), a1_(a1){};
  protected: // for slots
    Fl_Cb_m1(O_ o, R(O::*m)(A1)):Fl_Cb_<R>(), m_(m), o_(o){};
		Fl_Cb_m1(O_ o, R(O::*m)(A1)const):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1)>(m)), o_(o){};
	};

	// slot-type functors
	template <typename R, typename O, typename O_, typename A1>
	class Fl_Cb_m1p:public Fl_Cb_m1<R,O,O_,A1,A1>{
    friend CLASS Fl_Cb_<R>;
    friend CLASS _Ref<R>;

  public:
		void operator = (A1 p){Fl_Cb_m1<R,O,O_,A1,A1>::a1_=p; execute(this);};
	public:
		Fl_Cb_m1p(O_ o, R(O::*m)(A1)):Fl_Cb_m1<R,O,O_,A1,A1>(o,m){};
		Fl_Cb_m1p(O_ o, R(O::*m)(A1)const):Fl_Cb_m1<R,O,O_,A1,A1>(o,m){};
	};


	template <typename R, typename O, typename O_,  typename A1, typename A1_, typename R_>
	class Fl_Cb_m1r: public Fl_Cb_m1<R,O,O_,A1,A1_>{
		R_ r_;
		void call(){Fl_Cb_m1<R,O,O_,A1,A1_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_m1<R,O,O_,A1,A1_>::res;};
	public:
		Fl_Cb_m1r(O_ o, R(O::*m)(A1), A1_ a1, R_ r):Fl_Cb_m1<R,O,O_,A1,A1_>(o,m,a1), r_(r){};
		Fl_Cb_m1r(O_ o, R(O::*m)(A1) const, A1_ a1, R_ r):Fl_Cb_m1<R,O,O_,A1,A1_>(o,m,a1), r_(r){};
	};



	template <typename R, typename O, typename O_, typename A1, typename R_>
	class Fl_Cb_m1rp:public Fl_Cb_m1r<R,O,O_,A1,A1,R_>{
    friend CLASS Fl_Cb_<R>;
    friend CLASS _Ref<R>;
  public:
		void operator = (A1 p){Fl_Cb_m1r<R,O,O_,A1,A1,R_>::a1_=p; execute(this);};
	public:
		Fl_Cb_m1rp(O_ o, R(O::*m)(A1), R_ r):Fl_Cb_m1r<R,O,O_,A1,A1,R_>(o,m,&Fl_Cb_m1r<R,O,O_,A1,A1,R_>::a1_,r){};
		Fl_Cb_m1rp(O_ o, R(O::*m)(A1)const, R_ r):Fl_Cb_m1r<R,O,O_,A1,A1,R_>(o,m,&Fl_Cb_m1r<R,O,O_,A1,A1,R_>::a1_,r){};
	};


/////////////////////////////////// 2 arguments /////////////////////////////
#if SIG_PARAMETERS > 1

	template <typename R, typename A1, typename A1_, typename A2, typename A2_>
	class Fl_Cb_f2:public Fl_Cb_<R>
	{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)(A1,A2);
		A1_ a1_;
		A2_ a2_;
		void exec_r(){Fl_Cb_<R>::res = (*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_));};
		void exec(){(*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_));};
		void call(){execute(this);};
	public:
		Fl_Cb_f2(R(*f)(A1,A2), A1_ a1, A2_ a2):Fl_Cb_<R>(), f_(f), a1_(a1), a2_(a2){};
	};


	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename R_>
	class Fl_Cb_f2r:public Fl_Cb_f2<R,A1,A1_,A2,A2_>{
		R_ r_;
		void call(){Fl_Cb_f2<R,A1,A1_,A2,A2_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_f2<R,A1,A1_,A2,A2_>::res;};
	public:
		Fl_Cb_f2r(R(*f)(A1,A2), A1_ a1, A2_ a2, R_ r):Fl_Cb_f2<R,A1,A1_,A2,A2_>(f, a1, a2), r_(r){};
	};


////////////////

	template <typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_>
	class Fl_Cb_m2:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)(A1,A2);
		O_ o_;
		A1_ a1_;
		A2_ a2_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_));};
		void exec(){(_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_));};
		void call(){execute(this);};
	public:
		Fl_Cb_m2(O_ o, R(O::*m)(A1,A2), A1_ a1, A2_ a2):Fl_Cb_<R>(), m_(m), o_(o), a1_(a1), a2_(a2){};
		Fl_Cb_m2(O_ o, R(O::*m)(A1,A2)const, A1_ a1, A2_ a2):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1,A2)>(m)), o_(o), a1_(a1), a2_(a2){};
	};



	template <typename R, typename O, typename O_,  typename A1, typename A1_, typename A2, typename A2_, typename R_>
	class Fl_Cb_m2r: public Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>{
		R_ r_;
		void call(){Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>::res;};
	public:
		Fl_Cb_m2r(O_ o, R(O::*m)(A1,A2), A1_ a1, A2_ a2, R_ r):Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o,m,a1,a2), r_(r){};
		Fl_Cb_m2r(O_ o, R(O::*m)(A1,A2) const, A1_ a1, A2_ a2, R_ r):Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o,m,a1,a2), r_(r){};
	};

/////////////////////////////////// 3 arguments /////////////////////////////
#if SIG_PARAMETERS > 2

	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	class Fl_Cb_f3:public Fl_Cb_<R>	{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)(A1,A2,A3);
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		void exec_r(){Fl_Cb_<R>::res = (*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_),_get<A3,A3_>(a3_));};
		void exec(){(*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_),_get<A3,A3_>(a3_));};
		void call(){execute(this);};
	public:
		Fl_Cb_f3(R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3):Fl_Cb_<R>(), f_(f), a1_(a1), a2_(a2), a3_(a3){};
	};


	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	class Fl_Cb_f3r:public Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>{
		R_ r_;
		void call(){Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>::res;};
	public:
		Fl_Cb_f3r(R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>(f, a1, a2, a3), r_(r){};
	};


////////////////


	template <typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	class Fl_Cb_m3:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)(A1,A2,A3);
		O_ o_;
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_),_get<A3,A3_>(a3_));};
		void exec(){(_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_),_get<A3,A3_>(a3_));};
		void call(){execute(this);};
	public:
		Fl_Cb_m3(O_ o, R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3):Fl_Cb_<R>(), m_(m), o_(o), a1_(a1), a2_(a2), a3_(a3){};
		Fl_Cb_m3(O_ o, R(O::*m)(A1,A2,A3)const, A1_ a1, A2_ a2, A3_ a3):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1,A2,A3)>(m)), o_(o), a1_(a1), a2_(a2), a3_(a3){};
	};


	template <typename R, typename O, typename O_,  typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	class Fl_Cb_m3r: public Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>{
		R_ r_;
		void call(){Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>::res;};
	public:
		Fl_Cb_m3r(O_ o, R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o,m,a1,a2,a3), r_(r){};
		Fl_Cb_m3r(O_ o, R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o,m,a1,a2,a3), r_(r){};
	};

/////////////////////////////////// 4 arguments /////////////////////////////
#if SIG_PARAMETERS > 3

	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	class Fl_Cb_f4:public Fl_Cb_<R>	{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)(A1,A2,A3,A4);
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		A4_ a4_;
		void exec_r(){Fl_Cb_<R>::res = (*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_));};
		void exec(){(*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_));};
		void call(){execute(this);};
	public:
		Fl_Cb_f4(R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Cb_<R>(), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4){};
	};


	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	class Fl_Cb_f4r:public Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>{
		R_ r_;
		void call(){Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>::res;};
	public:
		Fl_Cb_f4r(R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(f, a1, a2, a3, a4), r_(r){};
	};


////////////////

	template <typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	class Fl_Cb_m4:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)(A1,A2,A3,A4);
		O_ o_;
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		A4_ a4_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_), _get<A3,A3_>(a3_), _get<A4,A4_>(a4_));};
		void exec(){(_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_));};
		void call(){execute(this);};
	public:
		Fl_Cb_m4(O_ o, R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Cb_<R>(), m_(m), o_(o), a1_(a1), a2_(a2), a3_(a3), a4_(a4){};
		Fl_Cb_m4(O_ o, R(O::*m)(A1,A2,A3,A4)const, A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1,A2,A3,A4)>(m)), o_(o), a1_(a1), a2_(a2), a3_(a3), a4_(a4){};
	};



	template <typename R, typename O, typename O_,  typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	class Fl_Cb_m4r: public Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>{
		R_ r_;
		void call(){Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>::call(); *(_Ref<R_>::r(r_)) =  Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>::res;};
	public:
		Fl_Cb_m4r(O_ o, R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o,m,a1,a2,a3,a4), r_(r){};
		Fl_Cb_m4r(O_ o, R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o,m,a1,a2,a3,a4), r_(r){};
	};


/////////////////////////////////// 5 arguments /////////////////////////////
#if SIG_PARAMETERS > 4

	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	class Fl_Cb_f5:public Fl_Cb_<R>
	{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(*f_)(A1,A2,A3,A4,A5);
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		A4_ a4_;
		A5_ a5_;
		void exec_r(){Fl_Cb_<R>::res = (*f_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_), _get<A3,A3_>(a3_), _get<A4,A4_>(a4_), _get<A5,A5_>(a5_));};
		void exec(){(*f_)(_get<A1,A1_>(a1_),_get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_),_get<A5,A5_>(a5_));};
		void call(){execute(this);};
	public:
		Fl_Cb_f5(R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Cb_<R>(), f_(f), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5){};
	};


	template <typename R, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	class Fl_Cb_f5r:public Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>{
		R_ r_;
		void call(){Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>::res;};
	public:
		Fl_Cb_f5r(R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(f, a1, a2, a3, a4, a5), r_(r){};
	};



////////////////


	template <typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	class Fl_Cb_m5:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		R(O::*m_)(A1,A2,A3,A4,A5);
		O_ o_;
		A1_ a1_;
		A2_ a2_;
		A3_ a3_;
		A4_ a4_;
		A5_ a5_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_),_get<A5,A5_>(a5_));};
		void exec(){(_get<O*, O_>(o_)->*m_)(_get<A1,A1_>(a1_), _get<A2,A2_>(a2_),_get<A3,A3_>(a3_),_get<A4,A4_>(a4_),_get<A5,A5_>(a5_));};
		void call(){execute(this);};
	public:
		Fl_Cb_m5(O_ o, R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4,A5_ a5):Fl_Cb_<R>(), m_(m), o_(o), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5){};
		Fl_Cb_m5(O_ o, R(O::*m)(A1,A2,A3,A4,A5)const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Cb_<R>(), m_(reinterpret_cast<R(O::*)(A1,A2,A3,A4,A5)>(m)), o_(o), a1_(a1), a2_(a2), a3_(a3), a4_(a4), a5_(a5){};
	};



	template <typename R, typename O, typename O_,  typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	class Fl_Cb_m5r: public Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>{
		R_ r_;
		void call(){Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>::call(); *(_Ref<R_>::r(r_)) = Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>::res;};
	public:
		Fl_Cb_m5r(O_ o, R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o,m,a1,a2,a3,a4,a5), r_(r){};
		Fl_Cb_m5r(O_ o, R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o,m,a1,a2,a3,a4,a5), r_(r){};
	};


#endif // SIG_PARAMETERS > 4
#endif // SIG_PARAMETERS > 3
#endif // SIG_PARAMETERS > 2
#endif // SIG_PARAMETERS > 1

////////////////// Fl_Slot functors /////////////////////////

	template <typename S, typename R, typename A1>
	class Sl_f1:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		S s_;
		R(*f_)(A1);
		void exec_r(){Fl_Cb_<R>::res = (*f_)(s_->value());};
		void exec(){(*f_)(s_->value());};
		void call(){execute(this);};
	public:
		Sl_f1(S s, R(*f)(A1)):Fl_Cb_<R>(), s_(s), f_(f){};
	};


	template <typename S, typename R, typename A1, typename R_>
	class Sl_f1r:public Sl_f1<S,R,A1>{
		R_ r_;
		void call(){Sl_f1<S,R,A1>::call(); *(_Ref<R_>::r(r_)) = Sl_f1<S,R,A1>::res;};
	public:
		Sl_f1r(S s, R(*f)(A1), R_ r):Sl_f1<S,R,A1>(s,f), r_(r){};
	};



	template <typename S, typename R, typename O, typename O_, typename A1>
	class Sl_m1:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		S s_;
		R(O::*m_)(A1);
		O_ o_;
		void exec_r(){Fl_Cb_<R>::res = (_get<O*, O_>(o_)->*m_)(s_->value());};
		void exec(){(_get<O*, O_>(o_)->*m_)(s_->value());};
		void call(){execute(this);};
	public:
		Sl_m1(S s, O_ o, R(O::*m)(A1)):Fl_Cb_<R>(), s_(s), m_(m), o_(o){};
		Sl_m1(S s, O_ o, R(O::*m)(A1)const):Fl_Cb_<R>(), s_(s), m_(reinterpret_cast<R(O::*)(A1)>(m)), o_(o){};
	};

	template <typename S, typename R, typename O, typename O_,  typename A1, typename R_>
	class Sl_m1r: public Sl_m1<S,R,O,O_,A1>{
		R_ r_;
		void call(){Sl_m1<S,R,O,O_,A1>::call(); *(_Ref<R_>::r(r_)) = Sl_m1<S,R,O,O_,A1>::res;};
	public:
		Sl_m1r(S s, O_ o, R(O::*m)(A1), R_ r):Sl_m1<S,R,O,O_,A1>(s,o,m), r_(r){};
		Sl_m1r(S s, O_ o, R(O::*m)(A1) const, R_ r):Sl_m1<S,R,O,O_,A1>(s,o,m), r_(r){};
	};

///////////////////////////// value()slots /////////////////////////////


 template <typename S, typename R, typename O_>
	class Sl_W:public Fl_Cb_<R>{
		friend CLASS Fl_Cb_<R>;
	protected:
		S s_;
		//R(O::*m_)(A1);
		O_ o_;
		void exec_r(){Fl_Cb_<R>::res = o_->value(s_->value());};
		void exec(){o_->value(s_->value());};
		void call(){execute(this);};
	public:
		Sl_W(S s, O_ o):Fl_Cb_<R>(), s_(s), o_(o){};
	};

	template <typename S, typename R, typename O_, typename R_>
	class Sl_Wr: public Sl_W<S,R,O_>{
		R_ r_;
		void call(){Sl_W<S,R,O_>::call(); *(_Ref<R_>::r(r_)) = Sl_W<S,R,O_>::res;};
	public:
		Sl_Wr(S s, O_ o, R_ r):Sl_W<S,R,O_>(s,o), r_(r){};
	};




	// slot functor for Fl_Signal

	template<typename N1, typename P>
	class Sln: public Fl_Cb{
		N1 n1_;
		Fl_Signal<P> * n2_;
		void call(){n2_->value((n1_->value()));};
	public:
		Sln(N1 n1, Fl_Signal<P> *n2):n1_(n1), n2_(n2){};
	};



////////////////////  slot(...) functions  /////////////////

template<typename R, typename A1>
inline Fl_Cb_f1p<R,A1> fl_slot(R(*f)(A1)){return Fl_Cb_f1p<R,A1>(f);};

template<typename R, typename A1, typename R_>
inline Fl_Cb_f1rp<R,A1,R_> fl_slot(R(*f)(A1)){return Fl_Cb_f1rp<R,A1,R_>(f);};


template<typename R, typename O, typename O_, typename A1>
inline Fl_Cb_m1p<R,O,O_,A1> fl_slot(O_ o, R(O::*m)(A1)){return Fl_Cb_m1p<R,O,O_,A1>(o, m);};

template<typename R, typename O, typename O_, typename A1>
inline Fl_Cb_m1p<R,O,O_,A1> fl_slot(O_ o, R(O::*m)(A1)const){return Fl_Cb_m1p<R,O,O_,A1>(o, m);};

template<typename R, typename O, typename O_, typename A1, typename R_>
inline Fl_Cb_m1rp<R,O,O_,A1,R_> fl_slot(O_ o, R(O::*m)(A1), R_ r){return Fl_Cb_m1rp<R,O,O_,A1,R_>(o, m, r);};

template<typename R, typename O, typename O_, typename A1, typename R_>
inline Fl_Cb_m1rp<R,O,O_,A1,R_> fl_slot(O_ o, R(O::*m)(A1)const, R_ r){return Fl_Cb_m1rp<R,O,O_,A1,R_>(o, m, r);};


/////////////////// call(...) functions  //////////////////////


template<typename R>
inline Fl_Cb_f0<R> fl_call(R(*f)()){return  Fl_Cb_f0<R>(f);};

template<typename R, typename R_>
inline Fl_Cb_f0r<R,R_> fl_call(R(*f)(), R_ r){return Fl_Cb_f0r<R,R_>(f, r);};


template<typename R, typename O, typename O_>
inline Fl_Cb_m0<R,O,O_> fl_call( O_ o, R(O::*m)()){return Fl_Cb_m0<R,O,O_>(o, m);};

template<typename R, typename O, typename O_>
inline Fl_Cb_m0<R,O,O_> fl_call(O_ o, R(O::*m)() const){return Fl_Cb_m0<R,O,O_>(o, m);};

template<typename R, typename O, typename O_, typename R_>
inline Fl_Cb_m0r<R,O,O_,R_> fl_call(O_ o ,R(O::*m)(), R_ r){return Fl_Cb_m0r<R,O,O_,R_>(o, m, r);};

template<typename R, typename O, typename O_, typename R_>
inline Fl_Cb_m0r<R,O,O_,R_> fl_call( O_ o, R(O::*m)() const, R_ r){return Fl_Cb_m0r<R,O,O_,R_>(o, m, r);};


/////////////////// 1 argument /////////////////////////////////

template<typename R, typename A1, typename A1_>
inline Fl_Cb_f1<R,A1,A1_> fl_call(R(*f)(A1), A1_ a1){return Fl_Cb_f1<R,A1,A1_>(f, a1);};

template<typename R, typename A1, typename A1_, typename R_>
inline Fl_Cb_f1r<R,A1,A1_,R_> fl_call(R(*f)(A1), A1_ a1, R_ r){return Fl_Cb_f1r<R,A1,A1_,R_>(f, a1, r);};


template<typename R, typename O, typename A1, typename O_, typename A1_>
inline Fl_Cb_m1<R,O,O_,A1,A1_> fl_call(O_ o , R(O::*m)(A1), A1_ a1){return  Fl_Cb_m1<R,O,O_,A1,A1_>(o, m, a1);};

template<typename R, typename O, typename A1, typename O_, typename A1_>
inline Fl_Cb_m1<R,O,O_,A1,A1_> fl_call(O_ o , R(O::*m)(A1) const, A1_ a1){return Fl_Cb_m1<R,O,O_,A1,A1_>(o, m, a1);};

template<typename R, typename O, typename A1, typename O_, typename A1_, typename R_>
inline Fl_Cb_m1r<R,O,O_,A1,A1_,R_> fl_call(O_ o , R(O::*m)(A1), A1_ a1, R_ r){return Fl_Cb_m1r<R,O,O_,A1,A1_,R_>(o, m, a1, r);};

template<typename R, typename O, typename A1, typename O_, typename A1_, typename R_>
inline Fl_Cb_m1r<R,O,O_,A1,A1_,R_> fl_call(O_ o , R(O::*m)(A1) const, A1_ a1, R_ r){return Fl_Cb_m1r<R,O,O_,A1,A1_,R_>(o, m, a1, r);};



///////////////// 2 arguments /////////////////////
#if SIG_PARAMETERS > 1

template<typename R, typename A1, typename A1_, typename A2,  typename A2_>
inline Fl_Cb_f2<R,A1,A1_,A2,A2_> fl_call(R(*f)(A1,A2), A1_ a1, A2_ a2){return Fl_Cb_f2<R,A1,A1_,A2,A2_>(f, a1, a2);};

template<typename R, typename A1, typename A2, typename A1_, typename A2_, typename R_>
inline Fl_Cb_f2r<R,A1,A1_,A2,A2_,R_> fl_call(R(*f)(A1,A2), A1_ a1, A2_ a2, R_ r){return Fl_Cb_f2r<R,A1,A1_,A2,A2_,R_>(f, a1, a2, r);};


template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_>
inline Fl_Cb_m2<R, O, O_, A1, A1_, A2, A2_> fl_call(O_ o , R(O::*m)(A1,A2), A1_ a1, A2_ a2){return Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o, m, a1, a2);};

template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_>
inline Fl_Cb_m2<R, O, O_, A1, A1_, A2, A2_> fl_call(O_ o , R(O::*m)(A1,A2) const, A1_ a1, A2_ a2){return Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o, m, a1, a2);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
inline Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_> fl_call(O_ o , R(O::*m)(A1,A2), A1_ a1, A2_ a2, R_ r){return Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_>(o, m, a1, a2, r);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
inline Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_> fl_call(O_ o , R(O::*m)(A1,A2) const, A1_ a1, A2_ a2, R_ r){return Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_>(o, m, a1, a2, r);};


///////////////// 3 arguments /////////////////////
#if SIG_PARAMETERS > 2

template<typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_>
inline Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_> fl_call(R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3){return Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>(f, a1, a2, a3);};

template<typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename R_>
inline Fl_Cb_f3r<R,A1,A1_,A2,A2_,A3,A3_,R_> fl_call(R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r){return Fl_Cb_f3r<R,A1,A1_,A2,A2_,A3,A3_,R_>(f, a1, a2, a3, r);};


template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
inline Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_> fl_call(O_ o , R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3){return Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o, m, a1, a2, a3);};

template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
inline Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_> fl_call(O_ o , R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3){return Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o, m, a1, a2, a3);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
inline Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r){return Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_>(o, m, a1, a2, a3, r);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
inline Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3, R_ r){return Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_>(o, m, a1, a2, a3, r);};


///////////////// 4 arguments /////////////////////
#if SIG_PARAMETERS > 3

template<typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_, typename A4, typename A4_>
inline Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_> fl_call(R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4){return Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(f, a1, a2, a3, a4);};

template<typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
inline Fl_Cb_f4r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_> fl_call(R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r){return Fl_Cb_f4r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(f, a1, a2, a3, a4, r);};


template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
inline Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4){return Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o, m, a1, a2, a3, a4);};

template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
inline Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4){return Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o, m, a1, a2, a3, a4);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
inline Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r){return Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(o, m, a1, a2, a3, a4, r);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
inline Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r){return Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(o, m, a1, a2, a3, a4, r);};


///////////////// 5 arguments /////////////////////
#if SIG_PARAMETERS > 5

template<typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
inline Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_> fl_call(R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5){return Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(f, a1, a2, a3, a4,a5);};

template<typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
inline Fl_Cb_f5r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_> fl_call(R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r){return Fl_Cb_f5r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(f, a1, a2, a3, a4, a5, r);};


template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
inline Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5){return Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o, m, a1, a2, a3, a4, a5);};

template<typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
inline Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5){return Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o, m, a1, a2, a3, a4, a5);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
inline Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r){return Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(o, m, a1, a2, a3, a4, a5, r);};

template<typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
inline Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_> fl_call(O_ o , R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r){return Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(o, m, a1, a2, a3, a4, a5, r);};

#endif // SIG_PARAMETERS > 4
#endif // SIG_PARAMETERS > 3
#endif // SIG_PARAMETERS > 2
#endif // SIG_PARAMETERS > 1
/////////////////////////////


//inline Fl_Cb_m0<void,Fl_Signal_, Fl_Signal_ *>	call(Fl_Signal_ * node){return Fl_Cb_m0<void,Fl_Signal_, Fl_Signal_ *>(node, &Fl_Signal_::call);};



///////////////////////////////////////////////////////////////////////////////////

extern Fl_Call_ * get_callback(Fl_Widget * w);
extern Fl_Call_ * get_callback(Fl_Signal_ * n);
extern R_Callback * set_callback(Fl_Widget * w, Fl_Call_ * c);
extern R_Callback * set_callback(Fl_Signal_ * n, Fl_Call_ * c);

//template<typename T>
//inline T * new_t(T t){return new T(t);};

class Fl_Call_ {
protected:
	friend CLASS Fl_Signal_;
	friend void cb_widget_wrap(Fl_Widget * , void * );
	friend void attach_callback(Fl_Widget * w, Fl_Call_ * c);
	friend void fl_detach_callbacks(Fl_Widget * w);

	void * node_; // addres of the node where the list is stored
	Fl_Call_ * next_;   // a circular list
	Fl_Call_ * prev_;	// this is for fast detaching when deleted
	R_Callback * update_; // this is for updating Fl_Signal_ or widget when removing callback
	const void * skip_;
	void attach(Fl_Call_ * c){
		if(c){
			next_=c->next_;
			prev_=c;
			c->next_->prev_=this;
			c->next_=this;
		}else
			next_=prev_=this;
	};

	bool skip(){
		if(!skip_) return 0;
		if(skip_ == &_fl_signal_sender || skip_ == _fl_signal_sender) return 1;
		return 0;
	};

private:
	Fl_Call_(const Fl_Call_ & );    // no, we realy do not want to use these
    const Fl_Call_ & operator=(const Fl_Call_ &);

public:
	virtual void call()=0;
	void enable(){skip_=0;};
	void disable(){skip_ = &_fl_signal_sender;}; // just some engaged pointer;
	void disable(const void * from){skip_ = from;};
	virtual ~Fl_Call_();

protected: //constructors
	Fl_Call_(Fl_Signal_ * node):node_(node),skip_(0){
		attach(get_callback(node));
		update_ = set_callback(node, this);
	};
	Fl_Call_(Fl_Widget * node):node_(node),skip_(0){
		attach(get_callback(node));
		update_ = set_callback(node, this);
	};
};

template<typename R>
class Fl_Call__:public Fl_Call_{
protected:
public:
	R value(){return cb_->value();};
protected:
	Fl_Cb_<R> * cb_;
	Fl_Call__(Fl_Signal_ * node):Fl_Call_(node){};
	Fl_Call__(Fl_Widget * node):Fl_Call_(node){};
public:
	void call(){
		if(skip()) return;
		cb_->call();
	};
	~Fl_Call__(){delete cb_;};

};

template<>
class Fl_Call__<void>:public Fl_Call_{

protected:
	Fl_Cb * cb_;
	Fl_Call__(Fl_Signal_ * node):Fl_Call_(node){};
	Fl_Call__(Fl_Widget * node):Fl_Call_(node){};
public:
	void call(){
		if(skip()) return;
		cb_->call();
	};
	~Fl_Call__(){delete cb_;};
};



template<typename T=FL_SIGNAL_VOID>
class Fl_Call:public Fl_Call__<T>, public Fl_Signal_{
	///// overloaded/templated constructors ///////////////
public:
  void call(){
		if(Fl_Call_::skip()) return;
		Fl_Call__<T>::cb_->call();
		execute();
	}

	// constructors

	template<typename N, typename R>
	Fl_Call(N node, R(*f)()):Fl_Call__<T>(node){ Fl_Call__<T>::cb_ = new  Fl_Cb_f0<R>(f);};

	template<typename N, typename R, typename R_>
	Fl_Call(N node, R(*f)(), R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f0r<R,R_>(f, r);};

	template<typename N, typename R, typename O, typename O_>
	Fl_Call(N node, O_ o, R(O::*m)()):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0<R,O,O_>(o, m);};

	template<typename N, typename R, typename O>
	Fl_Call(N node, R(O::*m)()):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0<R,O,N>(node, m);};

	template<typename N, typename R, typename O, typename O_>
	Fl_Call(N node, O_ o, R(O::*m)() const):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0<R,O,O_>(o, m);};

	template<typename N, typename R, typename O>
	Fl_Call(N node, R(O::*m)() const):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0<R,O,N>(node, m);};

	template<typename N,typename R, typename O, typename O_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(), R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0r<R,O,O_,R_>(o, m, r);};

	template<typename N,typename R, typename O, typename R_>
	Fl_Call(N node, R(O::*m)(), R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0r<R,O,N,R_>(node, m, r);};

	template<typename N, typename R, typename O, typename O_, typename R_>
	Fl_Call(N node, O_ o, R(O::*m)() const, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0r<R,O,O_,R_>(o, m, r);};

	template<typename N, typename R, typename O, typename R_>
	Fl_Call(N node, R(O::*m)() const, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m0r<R,O,N,R_>(node, m, r);};


	///////////////////////

	template<typename N, typename R, typename A1, typename A1_>
	Fl_Call(N node, R(*f)(A1), A1_ a1):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f1<R,A1,A1_>(f, a1);};

	template<typename N, typename R, typename A1, typename A1_, typename R_>
	Fl_Call(N node, R(*f)(A1), A1_ a1, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f1r<R,A1,A1_,R_>(f, a1, r);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename A1_>
	Fl_Call(N node, O_ o , R(O::*m)(A1), A1_ a1):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1<R,O,O_,A1,A1_>(o, m, a1);};

	template<typename N, typename R, typename O, typename A1, typename A1_>
	Fl_Call(N node, R(O::*m)(A1), A1_ a1):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1<R,O,N,A1,A1_>(node, m, a1);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename A1_>
	Fl_Call(N node, O_ o , R(O::*m)(A1) const, A1_ a1):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1<R,O,O_,A1,A1_>(o, m, a1);};

	template<typename N, typename R, typename O, typename A1, typename A1_>
	Fl_Call(N node, R(O::*m)(A1) const, A1_ a1):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1<R,O,N,A1,A1_>(node, m, a1);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename A1_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1), A1_ a1, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1r<R,O,O_,A1,A1_,R_>(o, m, a1, r);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename R_>
	Fl_Call(N node, R(O::*m)(A1), A1_ a1, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1r<R,O,N,A1,A1_,R_>(node, m, a1, r);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename A1_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1) const, A1_ a1, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1r<R,O,O_,A1,A1_,R_>(o, m, a1, r);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename R_>
	Fl_Call(N node, R(O::*m)(A1) const, A1_ a1, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m1r<R,O,N,A1,A1_,R_>(node, m, a1, r);};

	///////////////// 2 arguments /////////////////////
	#if  SIG_PARAMETERS > 1

	template<typename N, typename R, typename A1, typename A1_, typename A2,  typename A2_>
	Fl_Call(N node, R(*f)(A1,A2), A1_ a1, A2_ a2):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f2<R,A1,A1_,A2,A2_>(f, a1, a2);};

	template<typename N, typename R, typename A1, typename A2, typename A1_, typename A2_, typename R_>
	Fl_Call(N node, R(*f)(A1,A2), A1_ a1, A2_ a2, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f2r<R,A1,A1_,A2,A2_,R_>(f, a1, a2, r);};


	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2), A1_ a1, A2_ a2):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o, m, a1, a2);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_>
	Fl_Call(N node, R(O::*m)(A1,A2), A1_ a1, A2_ a2):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2<R,O,N,A1,A1_,A2,A2_>(node, m, a1, a2);};

	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2) const, A1_ a1, A2_ a2):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2<R,O,O_,A1,A1_,A2,A2_>(o, m, a1, a2);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_>
	Fl_Call(N node, R(O::*m)(A1,A2) const, A1_ a1, A2_ a2):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2<R,O,N,A1,A1_,A2,A2_>(node, m, a1, a2);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2), A1_ a1, A2_ a2, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_>(o, m, a1, a2, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2), A1_ a1, A2_ a2, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2r<R,O,N,A1,A1_,A2,A2_,R_>(node, m, a1, a2, r);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2) const, A1_ a1, A2_ a2, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2r<R,O,O_,A1,A1_,A2,A2_,R_>(o, m, a1, a2, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2) const, A1_ a1, A2_ a2, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m2r<R,O,N,A1,A1_,A2,A2_,R_>(node, m, a1, a2, r);};

	///////////////// 3 arguments /////////////////////
	#if  SIG_PARAMETERS > 2

	template<typename N, typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_>
	Fl_Call(N node, R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f3<R,A1,A1_,A2,A2_,A3,A3_>(f, a1, a2, a3);};

	template<typename N, typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename R_>
	Fl_Call(N node, R(*f)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f3r<R,A1,A1_,A2,A2_,A3,A3_,R_>(f, a1, a2, a3, r);};


	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o, m, a1, a2, a3);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3<R,O,N,A1,A1_,A2,A2_,A3,A3_>(node, m, a1, a2, a3);};

	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3<R,O,O_,A1,A1_,A2,A2_,A3,A3_>(o, m, a1, a2, a3);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3<R,O,N,A1,A1_,A2,A2_,A3,A3_>(node, m, a1, a2, a3);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_>(o, m, a1, a2, a3, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3), A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3r<R,O,N,A1,A1_,A2,A2_,A3,A3_,R_>(node, m, a1, a2, a3, r);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,R_>(o, m, a1, a2, a3, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3) const, A1_ a1, A2_ a2, A3_ a3, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m3r<R,O,N,A1,A1_,A2,A2_,A3,A3_,R_>(node, m, a1, a2, a3, r);};

	///////////////// 4 arguments /////////////////////
	#if  SIG_PARAMETERS > 3

	template<typename N, typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	Fl_Call(N node, R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f4<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(f, a1, a2, a3, a4);};

	template<typename N, typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	Fl_Call(N node, R(*f)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f4r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(f, a1, a2, a3, a4, r);};


	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o, m, a1, a2, a3, a4);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(node, m, a1, a2, a3, a4);};

	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(o, m, a1, a2, a3, a4);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_>(node, m, a1, a2, a3, a4);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(o, m, a1, a2, a3, a4, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4), A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4r<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(node, m, a1, a2, a3, a4, r);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(o, m, a1, a2, a3, a4, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m4r<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,R_>(node, m, a1, a2, a3, a4, r);};

	///////////////// 5 arguments /////////////////////
	#if  SIG_PARAMETERS > 4

	template<typename N, typename R, typename A1, typename A1_, typename A2,  typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	Fl_Call(N node, R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f5<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(f, a1, a2, a3, a4, a5);};

	template<typename N, typename R, typename A1, typename A2, typename A1_, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	Fl_Call(N node, R(*f)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_f5r<R,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(f, a1, a2, a3, a4, a5, r);};


	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o, m, a1, a2, a3, a4, a5);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(node, m, a1, a2, a3, a4, a5);};

	template<typename N, typename R, typename O, typename O_, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(o, m, a1, a2, a3, a4, a5);};

	template<typename N, typename R, typename O, typename A1, typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_>(node, m, a1, a2, a3, a4, a5);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(o, m, a1, a2, a3, a4, a5, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4,A5), A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5r<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(node, m, a1, a2, a3, a4, a5, r);};

	template<typename N, typename R, typename O, typename O_, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	Fl_Call(N node, O_ o , R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5r<R,O,O_,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(o, m, a1, a2, a3, a4, a5, r);};

	template<typename N, typename R, typename O, typename A1,  typename A1_, typename A2, typename A2_, typename A3, typename A3_, typename A4, typename A4_, typename A5, typename A5_, typename R_>
	Fl_Call(N node, R(O::*m)(A1,A2,A3,A4,A5) const, A1_ a1, A2_ a2, A3_ a3, A4_ a4, A5_ a5, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Fl_Cb_m5r<R,O,N,A1,A1_,A2,A2_,A3,A3_,A4,A4_,A5,A5_,R_>(node, m, a1, a2, a3, a4, a5, r);};

	#endif  // SIG_PARAMETERS > 4
	#endif  // SIG_PARAMETERS > 3
	#endif  // SIG_PARAMETERS > 2
	#endif  // SIG_PARAMETERS > 1

	/////////////////////

	template <typename N>
	Fl_Call(N node,  Fl_Signal_ * n2):Fl_Call__<T>(node){ Fl_Call__<T>::cb_ = new Fl_Cb_m0<void,Fl_Signal_, Fl_Signal_ *>(n2, &Fl_Signal_::execute);};

};


template<typename W, typename RET>
struct _Get_Slot_{

  template<typename N, typename R, typename A1>
  static Sl_f1<N,R,A1> * get(N node, R(*f)(A1)){return new Sl_f1<N,R,A1>(node,f);};

  template<typename N, typename P>
  static Sln<N,P> * get(N node, Fl_Signal<P> * n2){ return new Sln<N,P>(node,n2);};

  template<typename N>
  static Sl_W<N, RET, W> * get(N node, Fl_Widget * w){return new Sl_W<N, RET, W>(node,(W)w);};
};





template<typename T = FL_SIGNAL_VOID>
class Fl_Slot:public Fl_Call__<T>, public Fl_Signal_{
public:
  void call(){
		if(Fl_Call_::skip()) return;
		Fl_Call__<T>::cb_->call();
		execute();
	};


  template<typename N, typename P>
  Fl_Slot(N node, P slot):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = _Get_Slot_<P,T>::get(node,slot);};

	template<typename N, typename R, typename A1, typename R_>
	Fl_Slot(N node, R(*f)(A1), R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Sl_f1r<N,R,A1,R_>(node,f, r);};

	template<typename N, typename R, typename O, typename A1, typename O_>
	Fl_Slot(N node, O_ o , R(O::*m)(A1)):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Sl_m1<N,R,O,O_,A1>(node, o, m);};

	template<typename N, typename R, typename O, typename A1, typename O_>
	Fl_Slot(N node, O_ o , R(O::*m)(A1) const):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Sl_m1<N,R,O,O_,A1>(node, o, m);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename R_>
	Fl_Slot(N node, O_ o , R(O::*m)(A1), R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Sl_m1r<N,R,O,O_,A1,R_>(node, o, m, r);};

	template<typename N, typename R, typename O, typename A1, typename O_, typename R_>
	Fl_Slot(N node, O_ o , R(O::*m)(A1) const, R_ r):Fl_Call__<T>(node){Fl_Call__<T>::cb_ = new Sl_m1r<N,R,O,O_,A1,R_>(node, o, m, r);};


};



/////////////////////////////////////////////////////////////////////

inline void fl_detach_callbacks(Fl_Signal_ * n){n->detach_callbacks();};
extern void fl_detach_callbacks(Fl_Widget * w);



#endif
