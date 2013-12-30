// this pragma solves the atan2, ATAN2, pow(complex, real) problem !

// #pragma optimize ( "p" , on )

// the above doesn't solve issue under VC2005 and it slows down
// the calc. , removing and resolving with Z=C see FractalsView::CalculatePoint()


#if defined(_Type0_) || !defined(COMPLEX_H)

/*
  C++ Complex Library v1.0
  Laurent Deniau, last modified 03/97
  e-mail: Laurent.Deniau@cern.ch
*/

#include "gdefs.h"
// #include "WhichBuild.h"
#include "Math.h"
#pragma inline_depth(64)
 
// #include <CL/cl.hpp>
// #include "quad4helper.h"

// -------------------
// define default defs
// -------------------
#ifndef _Type0_
#define _Type0_ Complex
#define _Type1_ float4
#define _DefaultDefs_
#define COMPLEX_H
#endif

#define HALFPI	1.570796326794896619

extern inline double ATAN2( double y, double x );



class _Type0_ {
public:
  static _Type0_  I; // root of z^2+1 = 0
  static _Type0_  J; // root of z^2+z+1 = 0
  static _Type0_ _I; // 1/I
  static _Type0_ _J; // 1/J

//private:
public:
  _Type1_ r, i;
  _Type1_ RE( void )
  {
	  return this->r;
  };
  _Type1_ IM( void )
  {
	  return this->i;
  };

  // left char, separator char and right char
  static char lch, sep, rch; 

public:
  _Type0_ (_Type1_ x=0, _Type1_ y=0): r(x), i(y) {}

  _Type0_& _Type0_::operator = (const _Type0_& a )
   {
	  r = a.r ;
	  i = a.i ;
	  return *this ;
   }
  
  //_Type0_& _Type0_::operator = (CNumber& a )
  // {
	//  r = a.r ;
	//  i = a.i ;
	//  return *this ;
  // }
  
  _Type0_& operator  = (_Type1_);

  _Type0_& operator += (const _Type0_&);
  _Type0_& operator += (_Type1_);

  _Type0_& operator -= (const _Type0_&);
  _Type0_& operator -= (_Type1_);

  _Type0_& operator *= (const _Type0_&);
  _Type0_& operator *= (_Type1_);

  _Type0_& operator /= (const _Type0_&);
  _Type0_& operator /= (_Type1_);

  _Type0_& operator + ();  // plus sign
  _Type0_  operator - ();  // minus sign
  _Type0_  operator ~ ();  // conjugate
  bool     operator ! ();  // not
  
  _Type0_& operator ++ (); // inc re(x)
  _Type0_& operator -- (); // dec re(x)
  _Type0_& operator ++ (int); // inc re(x) (postfix)
  _Type0_& operator -- (int); // dec re(x) (prefix)

  _Type0_& setstr (char, char, char);
  _Type0_& getstr (char*, char*, char*);

  friend _Type1_ re   (const _Type0_&);
  friend _Type1_ im   (const _Type0_&);
  friend _Type0_ ir   (const _Type0_&); // swap re and im

  friend _Type1_ abs  (const _Type0_&);
  friend _Type1_ norm (const _Type0_&);
  friend _Type1_ arg  (const _Type0_&);
  friend Int     sgn  (const _Type0_&);

  friend _Type0_ rec (const _Type0_&); // P -> R
  friend _Type0_ pol (const _Type0_&); // R -> P

  friend _Type0_ rec (const _Type1_, const _Type1_); // P -> R
  friend _Type0_ pol (const _Type1_, const _Type1_); // R -> P

  friend _Type0_ operator + (const _Type0_&, const _Type0_&);
  friend _Type0_ operator + (const _Type0_&, _Type1_);
  friend _Type0_ operator + (_Type1_, const _Type0_&);

  friend _Type0_ operator - (const _Type0_&, const _Type0_&);
  friend _Type0_ operator - (const _Type0_&, _Type1_);
  friend _Type0_ operator - (_Type1_, const _Type0_&);

  friend _Type0_ operator * (const _Type0_&, const _Type0_&);
  friend _Type0_ operator * (const _Type0_&, _Type1_);
  friend _Type0_ operator * (_Type1_, const _Type0_&);

  friend _Type0_ operator / (const _Type0_&, const _Type0_&);
  friend _Type0_ operator / (const _Type0_&, _Type1_);
  friend _Type0_ operator / (_Type1_, const _Type0_&);

  friend bool operator == (const _Type0_&, const _Type0_&);
  friend bool operator == (const _Type0_&, _Type1_);
  friend bool operator == (_Type1_, const _Type0_&);

  friend bool operator != (const _Type0_&, const _Type0_&);
  friend bool operator != (const _Type0_&, _Type1_);
  friend bool operator != (_Type1_, const _Type0_&);

  friend bool operator < (const _Type0_&, const _Type0_&);
  friend bool operator < (const _Type0_&, _Type1_);
  friend bool operator < (_Type1_, const _Type0_&);

  friend bool operator > (const _Type0_&, const _Type0_&);
  friend bool operator > (const _Type0_&, _Type1_);
  friend bool operator > (_Type1_, const _Type0_&);

  friend bool operator <= (const _Type0_&, const _Type0_&);
  friend bool operator <= (const _Type0_&, _Type1_);
  friend bool operator <= (_Type1_, const _Type0_&);

  friend bool operator >= (const _Type0_&, const _Type0_&);
  friend bool operator >= (const _Type0_&, _Type1_);
  friend bool operator >= (_Type1_, const _Type0_&);

  friend bool operator && (const _Type0_&, const _Type0_&);
  friend bool operator && (const _Type0_&, _Type1_);
  friend bool operator && (_Type1_, const _Type0_&);

  friend bool operator || (const _Type0_&, const _Type0_&);
  friend bool operator || (const _Type0_&, _Type1_);
  friend bool operator || (_Type1_, const _Type0_&);

  friend _Type0_ sin (const _Type0_&);
  friend _Type0_ cos (const _Type0_&);
  friend _Type0_ tan (const _Type0_&);

  friend _Type0_ sinh (const _Type0_&);
  friend _Type0_ cosh (const _Type0_&);
  friend _Type0_ tanh (const _Type0_&);

  friend _Type0_ asin (const _Type0_&);
  friend _Type0_ acos (const _Type0_&);
  friend _Type0_ atan (const _Type0_&);

  friend _Type0_ asinh (const _Type0_&);
  friend _Type0_ acosh (const _Type0_&);
  friend _Type0_ atanh (const _Type0_&);

  friend _Type0_ exp   (const _Type0_&);
  friend _Type0_ ln    (const _Type0_&); // natural logarithm
  friend _Type0_ log   (const _Type0_&); // natural logarithm
  friend _Type0_ log10 (const _Type0_&); // decimal logarithm
  friend _Type0_ sqrt  (const _Type0_&);

  friend _Type0_ pow (const _Type0_&, const _Type0_&);
  friend _Type0_ pow ( _Type0_, _Type1_);
  friend _Type0_ pow (_Type1_, const _Type0_&);
  friend _Type0_ pow (_Type0_, Int);

  // friend istream& operator>> (istream&, _Type0_&);
  // friend ostream& operator<< (ostream&, const _Type0_&);

};

/* 	
#ifndef Cnumber_Once
class CNumber : public Complex 
{  
public:
	// double r;    double i;    // inheritance is/was the solution to chicken-egg issue
								// trying to set CNumber = Complex .
	 
	CNumber::CNumber () { r = 0; i = 0; }
	// CNumber (const double& x) { r = x; i = 0; }
	CNumber::CNumber (double x) { r = x; i = 0; }
	CNumber::CNumber (const bool x) { r = x; i = 0; }
	CNumber::CNumber (const int x) { r = x; i = 0; }
	CNumber::CNumber (double& x, double& y) { r = x; i = y; }
	CNumber::CNumber (const double& x, const double& y) { r = x; i = y; }
	CNumber::CNumber (Complex& a) { r = a.r; i = a.i; }
	CNumber::CNumber (const CNumber& a) { r = a.r; i = a.i; }
	
	CNumber& CNumber::operator = (const CNumber& a)
	{
		r = a.r;  i = a.i;
		return *this;
	}
	 
	CNumber& CNumber::operator = (double a)
	{
		r = a;
		i = 0;
		return *this;
	}
	 
	CNumber& CNumber::operator = (bool a)
	{
		r = a;
		i = 0;
		return *this;
	} 
	 
	CNumber& CNumber::operator = (int a)
	{
		r = a;
		i = 0;
		return *this;
	} 
	 
	CNumber& CNumber::operator = (Complex& a)
	{
		r = a.r;
		i = a.i;
		return *this;
	} 
	 
	CNumber CNumber::operator<< (CString& input)
	{
		CNumber a;
		char* stop, *stop2; char inp[100] ;
		sprintf(inp , input) ;
		double b = strtod(inp, &stop);
		if (stop[0] == 0)
		{ a=b; return a; }
		else
		if (stop[0] == '(')
		{
			double c = strtod(inp+1, &stop);
			if (stop[0] == ',')
			{
				double d = strtod(stop+1, &stop2);
				if (stop2[0] == ')')
				{ a.r=c; a.i=d; return a; }
			}
		}
		a.r = NULL; a.i = 0;
		return a;
	}
};
#define Cnumber_Once
#endif	
*/
  

// Classical functions

_inline _Type1_
re (const _Type0_& x)
{
  return x.r;
}

_inline _Type1_
im (const _Type0_& x)
{
  return x.i;
}

_inline _Type0_
ir(const _Type0_& x)
{
  return _Type0_(x.i, x.r);
}

_inline _Type1_
abs (const _Type0_& x) 
{
  return sqrt( (x.r * x.r) + (x.i * x.i) );
}

_inline _Type1_
norm (const _Type0_& x)
{
  return x.r*x.r+x.i*x.i;
}

_inline _Type1_
arg (const _Type0_& x)
{
   return ATAN2(x.i, x.r);
	
}

_inline Int
sgn (const _Type0_& x)
{
  return x.r==0.0 ? (x.i==0.0 ? 0:(x.i<0.0 ? -1:1)) : (x.r<0.0 ? -1:1);
}

_inline _Type0_
rec (const _Type0_& x) 
{
  return _Type0_ (x.r*cos(x.i), x.r*sin(x.i));
}

_inline _Type0_
rec (const _Type1_ r, const _Type1_ t) 
{
  return _Type0_ (r*cos(t), r*sin(t));
}

_inline _Type0_
pol (const _Type0_& x) 
{
  return _Type0_ (_hypot(x.r,x.i), ATAN2(x.i,x.r));
}

_inline _Type0_
pol (const _Type1_ x, const _Type1_ y) 
{
  return _Type0_ (_hypot(x,y), ATAN2(y,x));
}

// Assignement operators

_inline _Type0_&
_Type0_::operator = (_Type1_ x)
{
  r = x;
  i = 0;
  return *this;
}

_inline _Type0_&
_Type0_::operator += (const _Type0_& x)
{
  r += x.r;
  i += x.i;
  return *this;
}

_inline _Type0_&
_Type0_::operator += (_Type1_ x)
{
  r += x;
  return *this;
}

_inline _Type0_&
_Type0_::operator -= (const _Type0_& x)
{
  r -= x.r;
  i -= x.i;
  return *this;
}

_inline _Type0_&
_Type0_::operator -= (_Type1_ x)
{
  r -= x;
  return *this;
}

_inline _Type0_&
_Type0_::operator *= (const _Type0_& x)
{
  _Type1_ t = r*x.r-i*x.i;
  i = r*x.i+i*x.r;
  r = t;
  return *this;
}

_inline _Type0_&
_Type0_::operator *= (_Type1_ x)
{
  r *= x;
  i *= x;
  return *this;
}

_inline _Type0_&
_Type0_::operator /= (_Type1_ x)
{
  r /= x;
  i /= x;
  return *this;
}

// Unary operators

_inline _Type0_&
_Type0_::operator + () 
{
  return *this;
}

_inline _Type0_
_Type0_::operator - () 
{
  return _Type0_ (-r, -i);
}

_inline _Type0_&
_Type0_::operator ++ () 
{
  ++r; 
  return *this;
}

_inline _Type0_&
_Type0_::operator -- () 
{
  --r; 
  return *this;
}

_inline _Type0_&
_Type0_::operator ++ (int)
{
  ++r; 
  return *this;
}

_inline _Type0_&
_Type0_::operator -- (int)
{
  --r; 
  return *this;
}

_inline _Type0_
_Type0_::operator ~ ()
{
  return _Type0_ (r, -i);
}

_inline bool
_Type0_::operator ! () 
{
  return !(r || i);
}

// Arithmetic operators

_inline _Type0_
operator + (const _Type0_& x, const _Type0_& y)
{
  return _Type0_ (x.r+y.r, x.i+y.i);
}

_inline _Type0_
operator + (const _Type0_& x, _Type1_ y) 
{
  return _Type0_ (x.r+y, x.i);
}

_inline _Type0_
operator + (_Type1_ x, const _Type0_& y) 
{
  return _Type0_ (x+y.r, y.i);
}

_inline _Type0_
operator - (const _Type0_& x, const _Type0_& y)
{
  return _Type0_ (x.r-y.r, x.i-y.i);
}

_inline _Type0_
operator - (const _Type0_& x, _Type1_ y) 
{
  return _Type0_ (x.r-y, x.i);
}

_inline _Type0_
operator - (_Type1_ x, const _Type0_& y) 
{
  return _Type0_ (x-y.r, -y.i);
}

_inline _Type0_
operator * (const _Type0_& x, const _Type0_& y)
{
	//_asm{						// this does work. I Don't know what it does to inlining.
	//	xorpd xmm4, xmm4		// this one instr. took user Z*Z+C from 106 to 120ms
	//}							// this could be because it didn't inline
	return _Type0_ (x.r*y.r-x.i*y.i, x.r*y.i+x.i*y.r);
}

_inline _Type0_
operator * (const _Type0_& x, _Type1_ y) 
{
  return _Type0_ (x.r*y, x.i*y);
}

_inline _Type0_
operator * (_Type1_ x, const _Type0_& y) 
{
  return _Type0_ (x*y.r, x*y.i);
}

_inline _Type0_
operator / (const _Type0_& x, _Type1_ y) 
{
  return _Type0_ (x.r/y, x.i/y);
}

// Comparison operators

_inline bool
operator == (const _Type0_& x, const _Type0_& y)
{
  return x.r==y.r && x.i==y.i;
}

_inline bool
operator == (const _Type0_& x, _Type1_ y) 
{
  return x.r==y && x.i==0;
}

_inline bool
operator == (_Type1_ x, const _Type0_& y) 
{
  return x==y.r && y.i==0;
}

_inline bool
operator != (const _Type0_& x, const _Type0_& y)
{
  return x.r!=y.r || x.i!=y.i;
}

_inline bool
operator != (const _Type0_& x, _Type1_ y) 
{
  return x.r!=y || x.i!=0;
}

_inline bool
operator != (_Type1_ x, const _Type0_& y) 
{
  return x!=y.r || y.i!=0;
}

_inline bool
operator < (const _Type0_& x, const _Type0_& y)
{
  _Type1_ ax = abs(x)*sgn(x);
  _Type1_ ay = abs(y)*sgn(y);

  if (fabs(ax-ay)<DBL_EPSILON) return false;
  else return ax < ay;
}

_inline bool
operator < (const _Type0_& x, _Type1_ y) 
{
  _Type1_ ax = abs(x)*sgn(x);

  if (fabs(ax-y)<DBL_EPSILON) return false;
  else return ax < y;
}

_inline bool
operator < (_Type1_ x, const _Type0_& y) 
{
  _Type1_ ay = abs(y)*sgn(y);

  if (fabs(x-ay)<DBL_EPSILON) return false;
  else return x < ay;
}

_inline bool
operator > (const _Type0_& x, const _Type0_& y)
{
  return y < x;
}

_inline bool
operator > (const _Type0_& x, _Type1_ y) 
{
  return y < x;
}

_inline bool
operator > (_Type1_ x, const _Type0_& y) 
{
  return y < x;
}

_inline bool
operator <= (const _Type0_& x, const _Type0_& y)
{
  _Type1_ ax = abs(x)*sgn(x);
  _Type1_ ay = abs(y)*sgn(y);

  if (fabs(ax-ay)<DBL_EPSILON) return true;
  else return ax < ay;
}

_inline bool
operator <= (const _Type0_& x, _Type1_ y) 
{
  _Type1_ ax = abs(x)*sgn(x);

  if (fabs(ax-y)<DBL_EPSILON) return true;
  else return ax < y;
}

_inline bool
operator <= (_Type1_ x, const _Type0_& y) 
{
  _Type1_ ay = abs(y)*sgn(y);

  if (fabs(x-ay)<DBL_EPSILON) return true;
  else return x < ay;
}

_inline bool
operator >= (const _Type0_& x, const _Type0_& y)
{
  return y <= x;
}

_inline bool
operator >= (const _Type0_& x, _Type1_ y) 
{
  return y <= x;
}

_inline bool
operator >= (_Type1_ x, const _Type0_& y) 
{
  return y <= x;
}

_inline bool 
operator && (const _Type0_& x, const _Type0_& y)
{
  return (x.r || x.i) && (y.r || y.i);
}

_inline bool 
operator && (const _Type0_& x, _Type1_ y)
{
  return (x.r || x.i) && y;
}

_inline bool 
operator && (_Type1_ x, const _Type0_& y)
{
  return x && (y.r || y.i);
}

_inline bool 
operator || (const _Type0_& x, const _Type0_& y)
{
  return x.r || x.i || y.r || y.i;
}

_inline bool 
operator || (const _Type0_& x, _Type1_ y)
{
  return x.r || x.i || y;
}

_inline bool 
operator || (_Type1_ x, const _Type0_& y)
{
  return x || y.r || y.i;
}

// Power/Logarithmic functions

_inline _Type0_
exp (const _Type0_& x)
{
  return rec( _Type1_(exp(x.r)), x.i );
}

_inline _Type0_
ln (const _Type0_& x)
{
  return _Type0_ (log(abs(x)), arg(x));
}

_inline _Type0_
log (const _Type0_& x)
{
  return _Type0_ (log(abs(x)), arg(x));
}

_inline _Type0_
log10 (const _Type0_& x)
{
  return ln(x)/log((double)10.0); // FGZ VC2005
}

_inline _Type0_
pow (const _Type0_& x, const _Type0_& y)
{
	/*	if(y.i == 0) {   // this slowed Z^2 down to .444 from .111 
			if(x.i == 0) {
				if( y.r == int(y.r) ) return _Type0_( pow(x.r, int(y.r)), 0 ) ;
				else return _Type0_( pow(x.r, y.r), 0 ) ;
			}
			else
				return pow(x, y.r);
		} 
		else if(x.i == 0) 
			return pow(x.r , y); */
	 
  _Type1_ r = log(abs(x));
  _Type1_ t = arg(x);
  return rec( _Type1_ (exp(r*y.r - y.i*t)), y.i*r + y.r*t );
}

//_inline _Type0_
//pow (const _Type0_& x, _Type1_ y)
//{
//  return exp(y*ln(x));
//}

_inline _Type0_
pow (_Type1_ x, const _Type0_& y)
{
  return exp(y*ln(x));
}

// Trigonometric functions

_inline _Type0_ 
sin (const _Type0_& x)
{
  	return _Type0_ ( sin(x.r) * cosh(x.i), cos(x.r) * sinh(x.i) );
}

_inline _Type0_ 
cos (const _Type0_& x)
{
	return _Type0_ ( cos(x.r)*cosh(x.i), -sin(x.r)*sinh(x.i) );
}

_inline _Type0_ 
tan (const _Type0_& x)
{
  return _Type0_ ( sin(x)/cos(x) );
}

// Hyperbolic functions

_inline _Type0_ 
sinh (const _Type0_& x)
{
	return _Type0_ ( sinh(x.r)*cos(x.i), cosh(x.r)*sin(x.i) );
}

_inline _Type0_
cosh (const _Type0_& x)
{
	return _Type0_ ( cosh(x.r)*cos(x.i), sinh(x.r)*sin(x.i) );
}

_inline _Type0_ 
tanh (const _Type0_& x)
{
  return _Type0_ ( sinh(x)/cosh(x) );
}

// Hyperbolic functions

_inline _Type0_
asinh (const _Type0_& x)
{
  return -ln(sqrt(1+x*x)-x);
}

_inline _Type0_
acosh (const _Type0_& x)
{
  return ln(sqrt(x*x-1)+x);
}

_inline _Type0_
atanh (const _Type0_& x)
{
  static const _Type0_ I = _Type0_(0,1);
  _Type0_ Ix = ir(~_Type0_(x));
  return ln((I+Ix)/(I-Ix))/2;
}

// Trigonometric functions

_inline _Type0_
asin (const _Type0_& x)
{
  return ~ir(ln(sqrt(1-x*x)+ir(~_Type0_(x))));
}

_inline _Type0_
acos (const _Type0_& x)
{
  if (x.r*x.i<0)
    return ir(~acosh(x));
  else
    return ~ir(acosh(x));
}

_inline _Type0_
atan (const _Type0_& x)
{
  static const _Type0_ I = _Type0_(0,1);
  return ir(~(ln((I+x)/(I-x))/2));
}

// I/O functions

_inline _Type0_&
_Type0_::setstr(char lc, char sp, char rc)
{
  lch = lc;
  sep = sp; 
  rch = rc;
  return *this;
}

_inline _Type0_&
_Type0_::getstr(char *lc, char *sp, char *rc)
{
  *lc = lch;
  *sp = sep; 
  *rc = rch;
  return *this;
}




// } // extern "C++"

// ---------------------
// undefine default defs
// ---------------------
#ifdef _DefaultDefs_
#  ifndef _DefaultCode_
#    undef _Type0_
#    undef _Type1_
#    undef _DefaultDefs_
#  endif
#endif

#endif // defined(_Type0_) || !defined(COMPLEX_H)
