/*
  C++ Complex Library v1.0
  Laurent Deniau, last modified 03/97
  e-mail: Laurent.Deniau@cern.ch
*/

// -------------------
// define default code
// -------------------
// #include "stdafx.h"

#define _DefaultCode_

#include "math.h"
#include "NewComplex.h"

// --------------
// static members
// --------------

_Type0_  _Type0_:: I = _Type0_(  0.0 , 1.0 );           // root of z^2+1 = 0
_Type0_  _Type0_:: J = _Type0_( -0.5 , 0.5*sqrt(3.0) ); // root of z^2+z+1 = 0
_Type0_  _Type0_::_I = 1.0/I;
_Type0_  _Type0_::_J = 1.0/J;

char _Type0_::lch = '(';
char _Type0_::sep = ',';
char _Type0_::rch = ')';

 
extern inline double ATAN2( double y, double x )
{
	// if ( y == 0 && x == 0 ) { return 0 ; }
	//SetLastError( 0 ) ;
	//double resval = atan2(y, x) ;
	// atan2 returns between -pi and pi
	//if ( resval > -4 && resval < 4 ) { return resval ; }
	//return 0 ;
	// seems to be no way to trap -1#IND00
	 
	if ( y == 0 && x == 0 ) { return 0 ; }
	if ( x == 0 && y > 0  ) { return HALFPI ; }
	if ( x == 0 && y < 0  ) { return HALFPI * -1 ; }
	if ( y == 0 && x < 0  ) { return 2 * HALFPI ; }
	double amt = atan( y / fabs(x) ); 
	if ( x < 0 && y > 0 ) { amt = HALFPI - amt + HALFPI ; }
	if ( x < 0 && y < 0 ) { amt = -HALFPI - amt - HALFPI ; }
	return amt ; 
}


// Divisions avoiding overflow

_Type0_&
_Type0_::operator /= (const _Type0_& x)
{
  _Type1_ nr, ni;
  if (fabs(x.r) <= fabs(x.i)) {
    _Type1_ t = x.r/x.i;
    _Type1_ n = x.i*(1+t*t);
    nr = (r*t+i)/n;
    ni = (i*t-r)/n;
  } else {
    _Type1_ t = x.i/x.r;
    _Type1_ n = x.r*(1+t*t);
    nr = (r+i*t)/n;
    ni = (i-r*t)/n;
  }
  r = nr;
  i = ni;
  return *this;
}

extern _inline _Type0_
operator / (_Type1_ x, const _Type0_& y) 
{
  _Type1_ nr, ni;
  if (fabs(y.r) <= fabs(y.i)) {
    _Type1_ t = y.r/y.i;
    _Type1_ n = y.i*(1+t*t);
    nr = x*t/n;
    ni = -x/n;
  } else {
    _Type1_ t = y.i/y.r;
    _Type1_ n = y.r*(1+t*t);
    nr = x/n;
    ni = -x*t/n;
  }
  return _Type0_ (nr, ni);
}

extern _inline _Type0_
operator / (const _Type0_& x, const _Type0_& y)
{
  _Type1_ nr, ni;
  if (fabs(y.r) <= fabs(y.i)) {
    _Type1_ t = y.r/y.i;
    _Type1_ n = y.i*(1+t*t);
    nr = (x.r*t+x.i)/n;
    ni = (x.i*t-x.r)/n;
  } else {
    _Type1_ t = y.i/y.r;
    _Type1_ n = y.r*(1+t*t);
    nr = (x.r+x.i*t)/n;
    ni = (x.i-x.r*t)/n;
  }
  return _Type0_ (nr, ni);
}


extern _inline _Type0_
pow ( _Type0_ x, _Type1_ y)
{
    register double R = exp( log( sqrt((x.r * x.r) + (x.i * x.i)) ) * y);
	register double I = ATAN2(x.i, x.r) * y;
	
	return  _Type0_ (R * cos(I), R * sin(I));
} 

extern _inline _Type0_
pow (_Type0_ x, Int y)
{
  if (y == 0) return _Type0_ (1);
  
  _Type0_ r (1);
  if (y < 0) { y = -y; x = 1.0/x; }
  for (;;) {
    if (y & 1) r *= x;
    if (y >>= 1) x *= x;
    else return r;
  }
}

extern _inline _Type0_
sqrt (const _Type0_& x)
{
  _Type1_ n = abs(x);
  _Type1_ nr, ni;
  if (n == 0.0) nr = ni = n;
  else if (x.r >= 0) {
    nr = sqrt(0.5*(n+x.r));
    ni = 0.5*x.i/nr;
  } else {
    ni = sqrt(0.5*(n-x.r));
    if (x.i < 0) ni = -ni;
    nr = 0.5*x.i/ni;
  }
  return _Type0_ (nr, ni); 
}

