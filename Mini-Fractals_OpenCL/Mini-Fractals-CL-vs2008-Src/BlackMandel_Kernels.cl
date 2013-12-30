/**********************************************************************
Copyright ©2012 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/

/**
*  A fractal generator that calculates the mandlebrot set 
*  http://en.wikipedia.org/wiki/Mandelbrot_set 
* @param mandelbrotImage    mandelbrot images is stored in this
* @param scale              Represents the distance from which the fractal 
*                           is being seen if this is greater more area and 
*                           less detail is seen           
* @param maxIterations      More iterations gives more accurate mandelbrot image 
* @param width              size of the image 
*/

// #define ENABLE_DOUBLE 

#ifndef max 
#define max(A,B) (((A)>(B))?(A):(B)) 
#endif 

#define __COLOR_MAP_SIZE   31
#define __CMPSF  31.0f
  const float4 cmap_size4 = {__CMPSF, __CMPSF, __CMPSF, __CMPSF} ;

// #define zmod(num) (num - (((num) / __COLOR_MAP_SIZE) * __COLOR_MAP_SIZE))
#define zmod(num)  ( remainder(num, cmap_size4) )

#ifndef sqr
#define sqr(x)  ((x)*(x)) 
#endif

  const uint _color_map[__COLOR_MAP_SIZE] = 
	{0x00ffff, 0x206010, 0x200000, 0x38c020, 0x386010, 0x380000, 0x50c020, 0x506010, 0x500000, 0x68c020, 
	 0x686010, 0x680000, 0x80c020, 0x806010, 0x800000, 0x98c020, 0x986010, 0x980000, 0xb0c020, 0xb06010, 
	 0xb00000, 0xc8c020, 0xc86010, 0xc80000, 0xe0c020, 0xe06010, 0xe00000, 0xf8c020, 0xf86010, 0xf80000, 0x0000ff } ;
// {0x000000ff, 0x0000ffff, 0x0000ff00, 0x00ffff00, 0x00ff0000, 0x00ff00ff}

#define iFACTOR   256 
#define fFACTOR   256.0f
// #define dFACTOR   256.0

  const int4     iFACTOR4 = {iFACTOR, iFACTOR, iFACTOR, iFACTOR} ;
  const float4   fFACTOR4 = {fFACTOR, fFACTOR, fFACTOR, fFACTOR} ;
// const double4  dFACTOR4 = {dFACTOR, dFACTOR, dFACTOR, dFACTOR} ;
  const float4   ZEROf4    = {0.0f, 0.0f, 0.0f, 0.0f} ;
  const float4   ONEf4     = {1.0f, 1.0f, 1.0f, 1.0f} ;
  const float4   TWOf4     = {2.0f, 2.0f, 2.0f, 2.0f} ;
  const float4   FOURf4    = {4.0f, 4.0f, 4.0f, 4.0f} ;

// __local const Complexf4 ZEROcf4(ZEROf4, ZEROf4);
// __local const Complexf4 ONEcf4(ONEf4, ZEROf4); 

/*
//   I THINK THESE HAVE TO BE DEFINED LOWER ... AFTER THE ENABLE-DOUBLE DEFINE
const double4  ZEROd4 = {0.0,0.0,0.0,0.0} ;
const double4  ONEd4  = {1.0,1.0,1.0,1.0} ;
*/

/*
const Complexd4  ZEROc4(ZEROd4, ZEROd4);
const Complexd4  ONEc4(ONEd4, ZEROd4);
*/

#define maglimit 16.0f 

  const float4 fLOGpow   = {0.6931472f, 0.6931472f, 0.6931472f, 0.6931472f} ;     //  ln(2) =  0.6931472f 
  const float4 fLOGpow4  = {1.3862943f, 1.3862943f, 1.3862943f, 1.3862943f} ;     //  ln(4) = 1.3862943f  
  const float4 fLOGLOGml = {1.0197814f, 1.0197814f, 1.0197814f, 1.0197814f} ;     //  ln(ln(16)) = 1.0197814f    ln(ln(4)) =  0.3266343f   ln(ln(2)) = -0.3665129f   

// MakeGradientColor template inforces _nextColor as int and bulletproofs _mantissa, 
// result is always a color from base_color to next_color, inclusive !
inline uint MakeGradientColor(const uint _BaseColor, const uint _NextColor, const int _iMan)
{
	  uchar* _pBC = (uchar*)(&_BaseColor);
	  uchar* _pNC = (uchar*)(&_NextColor);
	  int __br = _pBC[0];
	  int __bg = _pBC[1];
	  int __bb = _pBC[2];
	  int __iMan =  max( 0, min( iFACTOR, _iMan ) );  // adds xx.x ms over haze-color, on std image + AMD RM-70 ... was 2.3 ms
	return (uint)(	  (( __br -  ( ((__br - (_pNC[0])) * __iMan)/iFACTOR )))       | 
					( (( __bg -  ( ((__bg - (_pNC[1])) * __iMan)/iFACTOR ))) << 8) | 
					( (( __bb -  ( ((__bb - (_pNC[2])) * __iMan)/iFACTOR ))) <<16) ); 
}
		
	
	inline int4 GetGradColor(float4 fMU)
	{
			  int4 bigMU = convert_int4(  max(ZEROf4, fMU) * fFACTOR4) ; 
		//	bigMU.s0 = (max(1.0f, fMU.s0 + 1.0f)*fFACTOR); 
		//	bigMU.s1 = (max(1.0f, fMU.s1 + 1.0f)*fFACTOR); 
		//	bigMU.s2 = (max(1.0f, fMU.s2 + 1.0f)*fFACTOR);
		//	bigMU.s3 = (max(1.0f, fMU.s3 + 1.0f)*fFACTOR);
			 
			  int4 iMU = (bigMU / iFACTOR4) ;
			  int4 iMan = bigMU % iFACTOR4  ; // bigMU - (iMU * iFACTOR4);
							
			// iMU-- ; 
			 
			  int4 clue, clue2, ecgs = {__COLOR_MAP_SIZE, __COLOR_MAP_SIZE, __COLOR_MAP_SIZE, __COLOR_MAP_SIZE};
			clue = iMU % ecgs;
			clue2.s0 = ((clue.s0+1>=__COLOR_MAP_SIZE)? 0 : clue.s0+1); 
			clue2.s1 = ((clue.s1+1>=__COLOR_MAP_SIZE)? 0 : clue.s1+1); 
			clue2.s2 = ((clue.s2+1>=__COLOR_MAP_SIZE)? 0 : clue.s2+1);
			clue2.s3 = ((clue.s3+1>=__COLOR_MAP_SIZE)? 0 : clue.s3+1);
			  int4 retc ;
			retc.s0 = MakeGradientColor( _color_map[clue.s0], _color_map[clue2.s0], iMan.s0 );
			retc.s1 = MakeGradientColor( _color_map[clue.s1], _color_map[clue2.s1], iMan.s1 );
			retc.s2 = MakeGradientColor( _color_map[clue.s2], _color_map[clue2.s2], iMan.s2 );
			retc.s3 = MakeGradientColor( _color_map[clue.s3], _color_map[clue2.s3], iMan.s3 );
			 
			return retc; 
	}


#ifndef ENABLE_DOUBLE

__kernel void mandelbrot_vector_float(
				__global uchar4 * mandelbrotImage,
                const    float posx, 
                const    float posy, 
                const    float stepSizeX,
                const    float stepSizeY,
                const    uint maxIterations,
                const    int width,
                const    int bench)
{
    int tid = get_global_id(0);

    int i = tid % (width / 4);
    int j = tid / (width / 4);
    
    int4 veci = {4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3};
    int4 vecj = {j, j, j, j};
    
    float4 x0;
    x0.s0 = (float)(posx + stepSizeX * (float)veci.s0);
    x0.s1 = (float)(posx + stepSizeX * (float)veci.s1);
    x0.s2 = (float)(posx + stepSizeX * (float)veci.s2);
    x0.s3 = (float)(posx + stepSizeX * (float)veci.s3);
    float4 y0;
    y0.s0 = (float)(posy + stepSizeY * (float)vecj.s0);
    y0.s1 = (float)(posy + stepSizeY * (float)vecj.s1);
    y0.s2 = (float)(posy + stepSizeY * (float)vecj.s2);
    y0.s3 = (float)(posy + stepSizeY * (float)vecj.s3);

    float4 x = x0;
    float4 y = y0;
    
    uint iter=0;
    float4 tmp;
    int4 stay;
    int4 ccount = 0;

    stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimit;
    stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimit;
    stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimit;
    stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimit;
    float4 savx = x;
    float4 savy = y;
    for(iter=0; (stay.s0 | stay.s1 | stay.s2 | stay.s3) && (iter < maxIterations); iter+= 16)
    {
        x = savx;
        y = savy;

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);
        
        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);
/*
		// /// another 16 ...

		// Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);
        
        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
        y = MUL_ADD(2.0f * x, y, y0);
        x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0f * tmp, y, y0);
*/ 
        stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimit;
        stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimit;
        stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimit;
        stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimit;

        savx.s0 = (stay.s0 ? x.s0 : savx.s0);
        savx.s1 = (stay.s1 ? x.s1 : savx.s1);
        savx.s2 = (stay.s2 ? x.s2 : savx.s2);
        savx.s3 = (stay.s3 ? x.s3 : savx.s3);
        savy.s0 = (stay.s0 ? y.s0 : savy.s0);
        savy.s1 = (stay.s1 ? y.s1 : savy.s1);
        savy.s2 = (stay.s2 ? y.s2 : savy.s2);
        savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        ccount += stay*16;
    }
    // Handle remainder
    if (!(stay.s0 & stay.s1 & stay.s2 & stay.s3))
    {
        iter = 16;
        do
        {
            x = savx;
            y = savy;
            stay.s0 = ((x.s0 * x.s0 + y.s0 * y.s0) <= maglimit) && 
						   (ccount.s0 < maxIterations);
            stay.s1 = ((x.s1 * x.s1 + y.s1 * y.s1) <= maglimit) && 
				       (ccount.s1 < maxIterations);
            stay.s2 = ((x.s2 * x.s2 + y.s2 * y.s2) <= maglimit) && 
				       (ccount.s2 < maxIterations);
            stay.s3 = ((x.s3 * x.s3 + y.s3 * y.s3) <= maglimit) && 
				       (ccount.s3 < maxIterations);
            tmp = x;
            x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
            y = MUL_ADD(2.0f * tmp, y, y0);
            ccount += stay;
            iter--;
            savx.s0 = (stay.s0 ? x.s0 : savx.s0);
            savx.s1 = (stay.s1 ? x.s1 : savx.s1);
            savx.s2 = (stay.s2 ? x.s2 : savx.s2);
            savx.s3 = (stay.s3 ? x.s3 : savx.s3);
            savy.s0 = (stay.s0 ? y.s0 : savy.s0);
            savy.s1 = (stay.s1 ? y.s1 : savy.s1);
            savy.s2 = (stay.s2 ? y.s2 : savy.s2);
            savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        } while ((stay.s0 | stay.s1 | stay.s2 | stay.s3) && iter);
    }
    x = savx;
    y = savy;

	// fgz - edit coloring method to all gpu-cl math and vectors 
	/*
    float4 fc = convert_float4(ccount);
    fc.s0 = (float)ccount.s0 + 1 - 
			           native_log2(native_log2(x.s0 * x.s0 + y.s0 * y.s0));
    fc.s1 = (float)ccount.s1 + 1 - 
		           native_log2(native_log2(x.s1 * x.s1 + y.s1 * y.s1));
    fc.s2 = (float)ccount.s2 + 1 - 
				   native_log2(native_log2(x.s2 * x.s2 + y.s2 * y.s2));
    fc.s3 = (float)ccount.s3 + 1 - 
				   native_log2(native_log2(x.s3 * x.s3 + y.s3 * y.s3));
				   
    float c = fc.s0 * 2.0f * 3.1416f / 256.0f;
    */
	/*  ESC-TIME COLORING :
	uchar4 color[4];
	int4 _mod = convert_int4(zmod(convert_float4(ccount)));
	uint _color = _color_map[((_mod.s0<0)? __COLOR_MAP_SIZE + _mod.s0 : _mod.s0)];
    
	color[0].s0 = _color & 0x0000ff ;
    color[0].s1 = (_color & 0x00ff00) >> 8  ;
    color[0].s2 = (_color & 0xff0000) >> 16 ;
    color[0].s3 = 0xff;
	*/
	
	// x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
    // y = MUL_ADD(2.0f * tmp, y, y0);
    
	//x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
    //y = MUL_ADD(2.0f * tmp, y, y0);
    //x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
    //y = MUL_ADD(2.0f * tmp, y, y0);
    

	
	float4 fMU   = convert_float4(ccount) + (( fLOGLOGml - log(log( sqrt(convert_float4(x*x + y*y))   )) ) / fLOGpow ) ;   //  TWOf4 - ( (log( /* sqrt */ (convert_float4(x*x + y*y))   )) / log(FOURf4) )  ;   
	int4 _color = GetGradColor( max(ZEROf4, fMU) );   
	
	uchar4 color[4];
	
	// int4 _mod = convert_int4(zmod(convert_float4(ccount)));
	// uint _color = _color_map[_mod.s0];
    
	color[0].s0 =  _color.s0 & 0x0000ff ;
    color[0].s1 = (_color.s0 & 0x00ff00) >> 8  ;
    color[0].s2 = (_color.s0 & 0xff0000) >> 16 ;
    color[0].s3 = 0xff;
    if (ccount.s0 == maxIterations)
    {
        color[0].s0 = 0;
        color[0].s1 = 0;
        color[0].s2 = 0;
    }
    if (bench)
    {
        color[0].s0 = ccount.s0 & 0xff;
        color[0].s1 = (ccount.s0 & 0xff00) >> 8;
        color[0].s2 = (ccount.s0 & 0xff0000) >> 16;
        color[0].s3 = (ccount.s0 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid] = color[0];    
    
	// _mod = zmod(ccount.s1);
	// _color = _color_map[((_mod.s1<0)? __COLOR_MAP_SIZE + _mod.s1 : _mod.s1)];
	color[1].s0 = _color.s1 & 0x0000ff ;
    color[1].s1 = (_color.s1 & 0x00ff00) >> 8  ;
    color[1].s2 = (_color.s1 & 0xff0000) >> 16 ;
    color[1].s3 = 0xff;
    if (ccount.s1 == maxIterations)
    {
        color[1].s0 = 0;
        color[1].s1 = 0;
        color[1].s2 = 0;
    }
    if (bench)
    {
        color[1].s0 = ccount.s1 & 0xff;
        color[1].s1 = (ccount.s1 & 0xff00) >> 8;
        color[1].s2 = (ccount.s1 & 0xff0000) >> 16;
        color[1].s3 = (ccount.s1 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 1] = color[1];    
    
	// _mod = zmod(ccount.s2);
	// _color = _color_map[((_mod.s2<0)? __COLOR_MAP_SIZE + _mod.s2 : _mod.s2)];
	color[2].s0 = _color.s2 & 0x0000ff ;
    color[2].s1 = (_color.s2 & 0x00ff00) >> 8  ;
    color[2].s2 = (_color.s2 & 0xff0000) >> 16 ;
    color[2].s3 = 0xff;
    if (ccount.s2 == maxIterations)
    {
        color[2].s0 = 0;
        color[2].s1 = 0;
        color[2].s2 = 0;
    }
    if (bench)
    {
        color[2].s0 = ccount.s2 & 0xff;
        color[2].s1 = (ccount.s2 & 0xff00) >> 8;
        color[2].s2 = (ccount.s2 & 0xff0000) >> 16;
        color[2].s3 = (ccount.s2 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 2] = color[2];    
    
	// _mod = zmod(ccount.s3);
	// _color = _color_map[((_mod.s3<0)? __COLOR_MAP_SIZE + _mod.s3 : _mod.s3)];
	color[3].s0 = _color.s3 & 0x0000ff ;
    color[3].s1 = (_color.s3 & 0x00ff00) >> 8  ;
    color[3].s2 = (_color.s3 & 0xff0000) >> 16 ;
    color[3].s3 = 0xff;
    if (ccount.s3 == maxIterations)
    {
        color[3].s0 = 0;
        color[3].s1 = 0;
        color[3].s2 = 0;
    }
    if (bench)
    {
        color[3].s0 = ccount.s3 & 0xff;
        color[3].s1 = (ccount.s3 & 0xff00) >> 8;
        color[3].s2 = (ccount.s3 & 0xff0000) >> 16;
        color[3].s3 = (ccount.s3 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 3] = color[3];
}


/////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



__kernel void ztof_vector_Complexf4(
				__global uchar4 * mandelbrotImage,
		        const    float pow_f,
				const    float posx, 
                const    float posy, 
                const    float stepSizeX,
                const    float stepSizeY,
                const    uint maxIterations,
                const    int width,
                const    int bench)
{
    int tid = get_global_id(0);

    int i = tid % (width / 4);
    int j = tid / (width / 4);
    
    int4 veci = {4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3};
    int4 vecj = {j, j, j, j};
    
    float4 x0 ;
    x0.s0 = (float)(posx + stepSizeX * (float)veci.s0);
    x0.s1 = (float)(posx + stepSizeX * (float)veci.s1);
    x0.s2 = (float)(posx + stepSizeX * (float)veci.s2);
    x0.s3 = (float)(posx + stepSizeX * (float)veci.s3);
    float4 y0 ;
    y0.s0 = (float)(posy + stepSizeY * (float)vecj.s0);
    y0.s1 = (float)(posy + stepSizeY * (float)vecj.s1);
    y0.s2 = (float)(posy + stepSizeY * (float)vecj.s2);
    y0.s3 = (float)(posy + stepSizeY * (float)vecj.s3);

    float4 x = x0;
    float4 y = y0;
    
    uint iter=0;
    float4 tmp;
    int4 stay;
    int4 ccount = 0;
	// float pow_f2 = pow_f ;
	// foo haha;
	// pow_f2 = haha.eat_my_float( pow_f );
	float4  LOGpowf = (float)log(pow_f) ;
	int ipow_f = (int)pow_f ;

    stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimit;
    stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimit;
    stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimit;
    stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimit;
    float4 savx = x;
    float4 savy = y;
	float4 Rx = ONEf4, Ry = ZEROf4;
	// Complexf4 savC = C, Z(ZEROc4), R ;
	float4 t;

    for(iter=0; (stay.s0 | stay.s1 | stay.s2 | stay.s3) && (iter < maxIterations); iter+= 16)
    {
        x = savx;
        y = savy;

		switch ( ipow_f ) 
		{
		default :
		case 2:   // handle mandel special  :-)  around 3.8 ms to 4.2 ms 
			for(int wtf=0; wtf < 16;  wtf++) {
				t = x;
				x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
				y = MUL_ADD(2.0f * t, y, y0);
			} // end for 16 iter
			break;
		
		// odd powers are slightly different than even powers ... 
		case 3:
			for(int wtf=0; wtf < 16;  wtf++) {   // around 7.2 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 5:
			for(int wtf=0; wtf < 16;  wtf++) { // around 9.7 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 7:
			for(int wtf=0; wtf < 16;  wtf++) { // around 14.6 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 9:
			for(int wtf=0; wtf < 16;  wtf++) {  // around 18.6 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		
		// odd - Even power div ================= 
		case 4:
			for(int wtf=0; wtf < 16;  wtf++) {  // Z^4 == 8.2 ms !!  // around 8.1 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 6:
			for(int wtf=0; wtf < 16;  wtf++) {  // around 12.3 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 8:
			for(int wtf=0; wtf < 16;  wtf++) {   // around 16.3 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		} // end switch  
		
		stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimit;
        stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimit;
        stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimit;
        stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimit;

        savx.s0 = (stay.s0 ? x.s0 : savx.s0);
        savx.s1 = (stay.s1 ? x.s1 : savx.s1);
        savx.s2 = (stay.s2 ? x.s2 : savx.s2);
        savx.s3 = (stay.s3 ? x.s3 : savx.s3);
        
		savy.s0 = (stay.s0 ? y.s0 : savy.s0);
        savy.s1 = (stay.s1 ? y.s1 : savy.s1);
        savy.s2 = (stay.s2 ? y.s2 : savy.s2);
        savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        
		ccount += stay * 16 ; // was : *32;
    }
    
	// Handle remainder
    if (!(stay.s0 & stay.s1 & stay.s2 & stay.s3))
    {
        iter = 32;
        do
        {
            x = savx;
            y = savy;
            stay.s0 = ((x.s0 * x.s0 + y.s0 * y.s0) <= maglimit) && 
						   (ccount.s0 < maxIterations);
            stay.s1 = ((x.s1 * x.s1 + y.s1 * y.s1) <= maglimit) && 
				       (ccount.s1 < maxIterations);
            stay.s2 = ((x.s2 * x.s2 + y.s2 * y.s2) <= maglimit) && 
				       (ccount.s2 < maxIterations);
            stay.s3 = ((x.s3 * x.s3 + y.s3 * y.s3) <= maglimit) && 
				       (ccount.s3 < maxIterations);
            Rx = 1.0;  Ry = 0.0;
			for (int n=0; n < ipow_f ; n++) {
				// R *= Z;
				//  y = MUL_ADD(2.0 * tmp, y, y0); 
				t = MUL_ADD( Rx, x, - Ry * y );
				Ry = MUL_ADD( Rx, y, Ry * x );
				Rx = t;
			}
			x = Rx + x0 ;
			y = Ry + y0 ;
			
			ccount += stay;
            iter--;
            savx.s0 = (stay.s0 ? x.s0 : savx.s0);
            savx.s1 = (stay.s1 ? x.s1 : savx.s1);
            savx.s2 = (stay.s2 ? x.s2 : savx.s2);
            savx.s3 = (stay.s3 ? x.s3 : savx.s3);
            savy.s0 = (stay.s0 ? y.s0 : savy.s0);
            savy.s1 = (stay.s1 ? y.s1 : savy.s1);
            savy.s2 = (stay.s2 ? y.s2 : savy.s2);
            savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        } while ((stay.s0 | stay.s1 | stay.s2 | stay.s3) && iter);
    } 
	
    
	x = savx;
    y = savy;
    
	
	float4 fMU   = convert_float4(ccount) + (( fLOGLOGml - log(log( sqrt(convert_float4(x*x + y*y))   )) ) / LOGpowf ) ;  // fLOGpow4 ) ;   //  TWOf4 - ( (log( /* sqrt */ (convert_float4(x*x + y*y))   )) / log(FOURf4) )  ;   
	int4 _color = GetGradColor( max(ZEROf4, fMU) );   
	
	uchar4 color[4];   // this has switched RED AND BLUE 
	
	color[0].s0 =   (_color.s0 & 0xff0000) >> 16 ;
    color[0].s1 = (_color.s0 & 0x00ff00) >> 8  ;
    color[0].s2 = _color.s0 & 0x0000ff ;
    color[0].s3 = 0xff;
    if (ccount.s0 >= maxIterations)
    {
        color[0].s0 = 0;
        color[0].s1 = 0;
        color[0].s2 = 0;
    }
    if (bench)
    {
        color[0].s0 = ccount.s0 & 0xff;
        color[0].s1 = (ccount.s0 & 0xff00) >> 8;
        color[0].s2 = (ccount.s0 & 0xff0000) >> 16;
        color[0].s3 = (ccount.s0 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid] = color[0];    
    
	color[1].s0 = (_color.s1 & 0xff0000) >> 16 ;
    color[1].s1 = (_color.s1 & 0x00ff00) >> 8  ;
    color[1].s2 = _color.s1 & 0x0000ff ;
    color[1].s3 = 0xff;
    if (ccount.s1 >= maxIterations)
    {
        color[1].s0 = 0;
        color[1].s1 = 0;
        color[1].s2 = 0;
    }
    if (bench)
    {
        color[1].s0 = ccount.s1 & 0xff;
        color[1].s1 = (ccount.s1 & 0xff00) >> 8;
        color[1].s2 = (ccount.s1 & 0xff0000) >> 16;
        color[1].s3 = (ccount.s1 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 1] = color[1];    
    
	color[2].s0 = (_color.s2 & 0xff0000) >> 16 ;
    color[2].s1 = (_color.s2 & 0x00ff00) >> 8  ;
    color[2].s2 = _color.s2 & 0x0000ff ;
    color[2].s3 = 0xff;
    if (ccount.s2 >= maxIterations)
    {
        color[2].s0 = 0;
        color[2].s1 = 0;
        color[2].s2 = 0;
    }
    if (bench)
    {
        color[2].s0 = ccount.s2 & 0xff;
        color[2].s1 = (ccount.s2 & 0xff00) >> 8;
        color[2].s2 = (ccount.s2 & 0xff0000) >> 16;
        color[2].s3 = (ccount.s2 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 2] = color[2];    
    
	color[3].s0 = (_color.s3 & 0xff0000) >> 16 ;
    color[3].s1 = (_color.s3 & 0x00ff00) >> 8  ;
    color[3].s2 = _color.s3 & 0x0000ff ;
    color[3].s3 = 0xff;
    if (ccount.s3 >= maxIterations)   // fgz: was == max ... now >= max
    {
        color[3].s0 = 0;
        color[3].s1 = 0;
        color[3].s2 = 0;
    }
    if (bench)
    {
        color[3].s0 = ccount.s3 & 0xff;
        color[3].s1 = (ccount.s3 & 0xff00) >> 8;
        color[3].s2 = (ccount.s3 & 0xff0000) >> 16;
        color[3].s3 = (ccount.s3 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 3] = color[3];
}

#endif  // end of !enable-double




/////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#define native_log2 log2
#define native_cos  cos

#ifdef ENABLE_DOUBLE
#ifdef KHR_DP_EXTENSION
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#else
#pragma OPENCL EXTENSION cl_amd_fp64 : enable
#endif

#define maglimitd  16.0 
const double4  ZEROd4 = {0.0,0.0,0.0,0.0} ;
const double4  ONEd4  = {1.0,1.0,1.0,1.0} ;

const double2  ZEROd2 = {0.0, 0.0} ;
const double2  ONEd2  = {1.0, 1.0} ;


 #define dFACTOR   256.0

  const int2     iFACTOR2 = {iFACTOR, iFACTOR} ;
  const double2   dFACTOR2 = {dFACTOR, dFACTOR} ;


  const double2 d2LOGLOGml = {1.01978144053822, 1.01978144053822} ;     //  ln(ln(16)) = 1.01978144053822        

		
	
	inline int2 GetGradColord2(double2 fMU)
	{
			  int2 bigMU = convert_int2(  max(ZEROd2, fMU) * dFACTOR2) ; 
		//	bigMU.s0 = (max(1.0f, fMU.s0 + 1.0f)*fFACTOR); 
		//	bigMU.s1 = (max(1.0f, fMU.s1 + 1.0f)*fFACTOR); 
		//	bigMU.s2 = (max(1.0f, fMU.s2 + 1.0f)*fFACTOR);
		//	bigMU.s3 = (max(1.0f, fMU.s3 + 1.0f)*fFACTOR);
			 
			  int2 iMU = (bigMU / iFACTOR2) ;
			  int2 iMan = bigMU % iFACTOR2  ; // bigMU - (iMU * iFACTOR4);
							
			// iMU-- ; 
			 
			  int2 clue, clue2, ecgs = {__COLOR_MAP_SIZE, __COLOR_MAP_SIZE};
			clue = iMU % ecgs;
			clue2.s0 = ((clue.s0+1>=__COLOR_MAP_SIZE)? 0 : clue.s0+1); 
			clue2.s1 = ((clue.s1+1>=__COLOR_MAP_SIZE)? 0 : clue.s1+1); 
			  int2 retc ;
			retc.s0 = MakeGradientColor( _color_map[clue.s0], _color_map[clue2.s0], iMan.s0 );
			retc.s1 = MakeGradientColor( _color_map[clue.s1], _color_map[clue2.s1], iMan.s1 );
			 
			return retc; 
	}



////////////////////////////////////////////////


__kernel void mandelbrot_vector_double(
				__global uchar4 * mandelbrotImage,
			//  	const    UINT *   _color_map,
                const    double posx, 
				const    double posy, 
                const    double stepSizeX,
                const    double stepSizeY,
                const    uint maxIterations,
                const    int width,
                const    int bench)
{
	// static int scnt = -1;
	// scnt++;
    int tid = get_global_id(0);
	// printf("global-id = %d    cnt = %d \n", tid, scnt);
    int i = tid % (width / 4);
    int j = tid / (width / 4);
    
    int4 veci = {4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3};
    int4 vecj = {j, j, j, j};
    
    double4 x0;
    x0.s0 = (double)(posx + stepSizeX * (double)veci.s0);
    x0.s1 = (double)(posx + stepSizeX * (double)veci.s1);
    x0.s2 = (double)(posx + stepSizeX * (double)veci.s2);
    x0.s3 = (double)(posx + stepSizeX * (double)veci.s3);
    double4 y0;
    y0.s0 = (double)(posy + stepSizeY * (double)vecj.s0);
    y0.s1 = (double)(posy + stepSizeY * (double)vecj.s1);
    y0.s2 = (double)(posy + stepSizeY * (double)vecj.s2);
    y0.s3 = (double)(posy + stepSizeY * (double)vecj.s3);

    double4 x = x0;
    double4 y = y0;
    
    uint iter=0;
    double4 tmp;
    int4 stay;
    int4 ccount = 0;

    stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
    stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
    stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd;
    stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd;
    double4 savx = x;
    double4 savy = y;
    for(iter=0; (stay.s0 | stay.s1 | stay.s2 | stay.s3) && (iter < maxIterations); iter+= 32)
    {
        x = savx;
        y = savy;

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); // tmp = x * x + x0 - y * y;
        y = MUL_ADD(2.0 * x, y, y0); //y = 2.0 * x * y + y0;
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));// x = tmp * tmp + x0 - y * y;
        y = MUL_ADD(2.0 * tmp, y, y0); //y = 2.0 * tmp * y + y0;

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0); 

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        ///////

		// Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); // tmp = x * x + x0 - y * y;
        y = MUL_ADD(2.0 * x, y, y0); //y = 2.0 * x * y + y0;
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));// x = tmp * tmp + x0 - y * y;
        y = MUL_ADD(2.0 * tmp, y, y0); //y = 2.0 * tmp * y + y0;

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0); 

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

        // Two iterations
        tmp = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
        y = MUL_ADD(2.0 * x, y, y0); 
		x = MUL_ADD(-y, y, MUL_ADD(tmp, tmp, x0));
        y = MUL_ADD(2.0 * tmp, y, y0);

		
		stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
        stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
        stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd;
        stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd;

        savx.s0 = (stay.s0 ? x.s0 : savx.s0);
        savx.s1 = (stay.s1 ? x.s1 : savx.s1);
        savx.s2 = (stay.s2 ? x.s2 : savx.s2);
        savx.s3 = (stay.s3 ? x.s3 : savx.s3);
        savy.s0 = (stay.s0 ? y.s0 : savy.s0);
        savy.s1 = (stay.s1 ? y.s1 : savy.s1);
        savy.s2 = (stay.s2 ? y.s2 : savy.s2);
        savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        ccount += stay*32;
    }
    // Handle remainder
    if (!(stay.s0 & stay.s1 & stay.s2 & stay.s3))
    {
        iter = 32;
        do
        {
            x = savx;
            y = savy;
            stay.s0 = ((x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd) && 
						   (ccount.s0 < maxIterations);
            stay.s1 = ((x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd) && 
				       (ccount.s1 < maxIterations);
            stay.s2 = ((x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd) && 
				       (ccount.s2 < maxIterations);
            stay.s3 = ((x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd) && 
				       (ccount.s3 < maxIterations);
            tmp = x;
            x = MUL_ADD(-y, y, MUL_ADD(x, x, x0)); 
            y = MUL_ADD(2.0 * tmp, y, y0); //y = 2.0 * tmp * y + y0;
            ccount += stay;
            iter--;
            savx.s0 = (stay.s0 ? x.s0 : savx.s0);
            savx.s1 = (stay.s1 ? x.s1 : savx.s1);
            savx.s2 = (stay.s2 ? x.s2 : savx.s2);
            savx.s3 = (stay.s3 ? x.s3 : savx.s3);
            savy.s0 = (stay.s0 ? y.s0 : savy.s0);
            savy.s1 = (stay.s1 ? y.s1 : savy.s1);
            savy.s2 = (stay.s2 ? y.s2 : savy.s2);
            savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        } while ((stay.s0 | stay.s1 | stay.s2 | stay.s3) && iter);
    }
    x = savx;
    y = savy;
    
	/* fgz -- change out color scheme ... is it done by cpu or gpu ??  

	double4 fc = convert_double4(ccount);
    fc.s0 = (double)ccount.s0 + 1 - 
			           native_log2(native_log2(x.s0 * x.s0 + y.s0 * y.s0));
    fc.s1 = (double)ccount.s1 + 1 - 
		           native_log2(native_log2(x.s1 * x.s1 + y.s1 * y.s1));
    fc.s2 = (double)ccount.s2 + 1 - 
				   native_log2(native_log2(x.s2 * x.s2 + y.s2 * y.s2));
    fc.s3 = (double)ccount.s3 + 1 - 
				   native_log2(native_log2(x.s3 * x.s3 + y.s3 * y.s3));
				   
    double c = fc.s0 * 2.0 * 3.1416 / 256.0;
    
	*/  

	float4 fMU   = convert_float4(ccount) + (( fLOGLOGml - log(log( sqrt(convert_float4(x*x + y*y))   )) ) / fLOGpow ) ;   //  TWOf4 - ( (log( /* sqrt */ (convert_float4(x*x + y*y))   )) / log(FOURf4) )  ;   
	int4 _color = GetGradColor( max(ZEROf4, fMU) );   
	
	uchar4 color[4];
	
	// int4 _mod = convert_int4(zmod(convert_float4(ccount)));
	// uint _color = _color_map[_mod.s0];
    
	color[0].s0 =  _color.s0 & 0x0000ff ;
    color[0].s1 = (_color.s0 & 0x00ff00) >> 8  ;
    color[0].s2 = (_color.s0 & 0xff0000) >> 16 ;
    color[0].s3 = 0xff;
    if (ccount.s0 == maxIterations)
    {
        color[0].s0 = 0;
        color[0].s1 = 0;
        color[0].s2 = 0;
    }
    if (bench)
    {
        color[0].s0 = ccount.s0 & 0xff;
        color[0].s1 = (ccount.s0 & 0xff00) >> 8;
        color[0].s2 = (ccount.s0 & 0xff0000) >> 16;
        color[0].s3 = (ccount.s0 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid] = color[0];    
    
	// _mod = zmod(ccount.s1);
	// _color = _color_map[_mod.s1];
	color[1].s0 = _color.s1 & 0x0000ff ;
    color[1].s1 = (_color.s1 & 0x00ff00) >> 8  ;
    color[1].s2 = (_color.s1 & 0xff0000) >> 16 ;
    color[1].s3 = 0xff;
    if (ccount.s1 == maxIterations)
    {
        color[1].s0 = 0;
        color[1].s1 = 0;
        color[1].s2 = 0;
    }
    if (bench)
    {
        color[1].s0 = ccount.s1 & 0xff;
        color[1].s1 = (ccount.s1 & 0xff00) >> 8;
        color[1].s2 = (ccount.s1 & 0xff0000) >> 16;
        color[1].s3 = (ccount.s1 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 1] = color[1];    
    
	// _mod = zmod(ccount.s2);
	// _color = _color_map[_mod.s2];
	color[2].s0 = _color.s2 & 0x0000ff ;
    color[2].s1 = (_color.s2 & 0x00ff00) >> 8  ;
    color[2].s2 = (_color.s2 & 0xff0000) >> 16 ;
    color[2].s3 = 0xff;
    if (ccount.s2 == maxIterations)
    {
        color[2].s0 = 0;
        color[2].s1 = 0;
        color[2].s2 = 0;
    }
    if (bench)
    {
        color[2].s0 = ccount.s2 & 0xff;
        color[2].s1 = (ccount.s2 & 0xff00) >> 8;
        color[2].s2 = (ccount.s2 & 0xff0000) >> 16;
        color[2].s3 = (ccount.s2 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 2] = color[2];    
    
	// _mod = zmod(ccount.s3);
	// _color = _color_map[_mod.s3];
	color[3].s0 = _color.s3 & 0x0000ff ;
    color[3].s1 = (_color.s3 & 0x00ff00) >> 8  ;
    color[3].s2 = (_color.s3 & 0xff0000) >> 16 ;
    color[3].s3 = 0xff;
    if (ccount.s3 == maxIterations)
    {
        color[3].s0 = 0;
        color[3].s1 = 0;
        color[3].s2 = 0;
    }
    if (bench)
    {
        color[3].s0 = ccount.s3 & 0xff;
        color[3].s1 = (ccount.s3 & 0xff00) >> 8;
        color[3].s2 = (ccount.s3 & 0xff0000) >> 16;
        color[3].s3 = (ccount.s3 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 3] = color[3];
}


__kernel void ztof_vector_Complexd4(
				__global uchar4 * mandelbrotImage,
		        const    double pow_f,
				const    double posx, 
                const    double posy, 
                const    double stepSizeX,
                const    double stepSizeY,
                const    uint maxIterations,
                const    int width,
                const    int bench)
{
    int tid = get_global_id(0);

    int i = tid % (width / 4);
    int j = tid / (width / 4);
    
    int4 veci = {4 * i, 4 * i + 1, 4 * i + 2, 4 * i + 3};
    int4 vecj = {j, j, j, j};
    
    double4 x0 ;
    x0.s0 = (double)(posx + stepSizeX * (double)veci.s0);
    x0.s1 = (double)(posx + stepSizeX * (double)veci.s1);
    x0.s2 = (double)(posx + stepSizeX * (double)veci.s2);
    x0.s3 = (double)(posx + stepSizeX * (double)veci.s3);
    double4 y0 ;
    y0.s0 = (double)(posy + stepSizeY * (double)vecj.s0);
    y0.s1 = (double)(posy + stepSizeY * (double)vecj.s1);
    y0.s2 = (double)(posy + stepSizeY * (double)vecj.s2);
    y0.s3 = (double)(posy + stepSizeY * (double)vecj.s3);

    double4 x = x0;
    double4 y = y0;
    
    uint iter=0;
    // double4 tmp;
    int4 stay;
    int4 ccount = 0;
	// double pow_f2 = pow_f ;
	// foo haha;
	// pow_f2 = haha.eat_my_float( pow_f );
	float4  LOGpowf = (float)log(pow_f) ;
	int ipow_f = (int)pow_f ;

    stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
    stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
    stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd;
    stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd;
    double4 savx = x;
    double4 savy = y;
	double4 Rx = ONEd4, Ry = ZEROd4;
	// Complexf4 savC = C, Z(ZEROc4), R ;
	double4 t;

    for(iter=0; (stay.s0 | stay.s1 | stay.s2 | stay.s3) && (iter < maxIterations); iter+= 16)
    {
        x = savx;
        y = savy;

		switch ( ipow_f ) 
		{
		default :
		case 2:   // handle mandel special  :-)  around 3.8 ms to 4.2 ms 
			for(int wtf=0; wtf < 16;  wtf++) {
				t = x;
				x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
				y = MUL_ADD(2.0f * t, y, y0);
			} // end for 16 iter
			break;
		
		// odd powers are slightly different than even powers ... 
		case 3:
			for(int wtf=0; wtf < 16;  wtf++) {   // around 7.2 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 5:
			for(int wtf=0; wtf < 16;  wtf++) { // around 9.7 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 7:
			for(int wtf=0; wtf < 16;  wtf++) { // around 14.6 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 9:
			for(int wtf=0; wtf < 16;  wtf++) {  // around 18.6 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		
		// odd - Even power div ================= 
		case 4:
			for(int wtf=0; wtf < 16;  wtf++) {  // Z^4 == 8.2 ms !!  // around 8.1 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 6:
			for(int wtf=0; wtf < 16;  wtf++) {  // around 12.3 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 8:
			for(int wtf=0; wtf < 16;  wtf++) {   // around 16.3 ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		} // end switch  
		
		stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
        stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
        stay.s2 = (x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd;
        stay.s3 = (x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd;

        savx.s0 = (stay.s0 ? x.s0 : savx.s0);
        savx.s1 = (stay.s1 ? x.s1 : savx.s1);
        savx.s2 = (stay.s2 ? x.s2 : savx.s2);
        savx.s3 = (stay.s3 ? x.s3 : savx.s3);
        
		savy.s0 = (stay.s0 ? y.s0 : savy.s0);
        savy.s1 = (stay.s1 ? y.s1 : savy.s1);
        savy.s2 = (stay.s2 ? y.s2 : savy.s2);
        savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        
		ccount += stay * 16 ; // was : *32;
    }
    
	// Handle remainder
    if (!(stay.s0 & stay.s1 & stay.s2 & stay.s3))
    {
        iter = 32;
        do
        {
            x = savx;
            y = savy;
            stay.s0 = ((x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd) && 
						   (ccount.s0 < maxIterations);
            stay.s1 = ((x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd) && 
				       (ccount.s1 < maxIterations);
            stay.s2 = ((x.s2 * x.s2 + y.s2 * y.s2) <= maglimitd) && 
				       (ccount.s2 < maxIterations);
            stay.s3 = ((x.s3 * x.s3 + y.s3 * y.s3) <= maglimitd) && 
				       (ccount.s3 < maxIterations);
            Rx = 1.0;  Ry = 0.0;
			for (int n=0; n < ipow_f ; n++) {
				// R *= Z;
				//  y = MUL_ADD(2.0 * tmp, y, y0); 
				t = MUL_ADD( Rx, x, - Ry * y );
				Ry = MUL_ADD( Rx, y, Ry * x );
				Rx = t;
			}
			x = Rx + x0 ;
			y = Ry + y0 ;
			
			ccount += stay;
            iter--;
            savx.s0 = (stay.s0 ? x.s0 : savx.s0);
            savx.s1 = (stay.s1 ? x.s1 : savx.s1);
            savx.s2 = (stay.s2 ? x.s2 : savx.s2);
            savx.s3 = (stay.s3 ? x.s3 : savx.s3);
            savy.s0 = (stay.s0 ? y.s0 : savy.s0);
            savy.s1 = (stay.s1 ? y.s1 : savy.s1);
            savy.s2 = (stay.s2 ? y.s2 : savy.s2);
            savy.s3 = (stay.s3 ? y.s3 : savy.s3);
        } while ((stay.s0 | stay.s1 | stay.s2 | stay.s3) && iter);
    } 
	
    
	x = savx;
    y = savy;
    
	
	float4 fMU   = convert_float4(ccount) + (( fLOGLOGml - log(log( sqrt(convert_float4(x*x + y*y))   )) ) / LOGpowf ) ;  // fLOGpow4 ) ;   //  TWOf4 - ( (log( /* sqrt */ (convert_float4(x*x + y*y))   )) / log(FOURf4) )  ;   
	int4 _color = GetGradColor( max(ZEROf4, fMU) );   
	
	uchar4 color[4];
	
	color[0].s0 =  _color.s0 & 0x0000ff ;
    color[0].s1 = (_color.s0 & 0x00ff00) >> 8  ;
    color[0].s2 = (_color.s0 & 0xff0000) >> 16 ;
    color[0].s3 = 0xff;
    if (ccount.s0 == maxIterations)
    {
        color[0].s0 = 0;
        color[0].s1 = 0;
        color[0].s2 = 0;
    }
    if (bench)
    {
        color[0].s0 = ccount.s0 & 0xff;
        color[0].s1 = (ccount.s0 & 0xff00) >> 8;
        color[0].s2 = (ccount.s0 & 0xff0000) >> 16;
        color[0].s3 = (ccount.s0 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid] = color[0];    
    
	color[1].s0 = _color.s1 & 0x0000ff ;
    color[1].s1 = (_color.s1 & 0x00ff00) >> 8  ;
    color[1].s2 = (_color.s1 & 0xff0000) >> 16 ;
    color[1].s3 = 0xff;
    if (ccount.s1 == maxIterations)
    {
        color[1].s0 = 0;
        color[1].s1 = 0;
        color[1].s2 = 0;
    }
    if (bench)
    {
        color[1].s0 = ccount.s1 & 0xff;
        color[1].s1 = (ccount.s1 & 0xff00) >> 8;
        color[1].s2 = (ccount.s1 & 0xff0000) >> 16;
        color[1].s3 = (ccount.s1 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 1] = color[1];    
    
	color[2].s0 = _color.s2 & 0x0000ff ;
    color[2].s1 = (_color.s2 & 0x00ff00) >> 8  ;
    color[2].s2 = (_color.s2 & 0xff0000) >> 16 ;
    color[2].s3 = 0xff;
    if (ccount.s2 == maxIterations)
    {
        color[2].s0 = 0;
        color[2].s1 = 0;
        color[2].s2 = 0;
    }
    if (bench)
    {
        color[2].s0 = ccount.s2 & 0xff;
        color[2].s1 = (ccount.s2 & 0xff00) >> 8;
        color[2].s2 = (ccount.s2 & 0xff0000) >> 16;
        color[2].s3 = (ccount.s2 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 2] = color[2];    
    
	color[3].s0 = _color.s3 & 0x0000ff ;
    color[3].s1 = (_color.s3 & 0x00ff00) >> 8  ;
    color[3].s2 = (_color.s3 & 0xff0000) >> 16 ;
    color[3].s3 = 0xff;
    if (ccount.s3 == maxIterations)
    {
        color[3].s0 = 0;
        color[3].s1 = 0;
        color[3].s2 = 0;
    }
    if (bench)
    {
        color[3].s0 = ccount.s3 & 0xff;
        color[3].s1 = (ccount.s3 & 0xff00) >> 8;
        color[3].s2 = (ccount.s3 & 0xff0000) >> 16;
        color[3].s3 = (ccount.s3 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 3] = color[3];
}

/// ///////////








__kernel void ztof_vector_Complexd2(
				__global uchar4 * mandelbrotImage,
		        const    double pow_f,
				const    double posx, 
                const    double posy, 
                const    double stepSizeX,
                const    double stepSizeY,
                const    uint maxIterations,
                const    int width,
                const    int bench)
{
    int tid = get_global_id(0);

    int i = tid % (width / 2);
    int j = tid / (width / 2);
    
    int2 veci = {2 * i, 2 * i + 1};
    int2 vecj = {j, j};
    
    double2 x0 ;
    x0.s0 = (double)(posx + stepSizeX * (double)veci.s0);
    x0.s1 = (double)(posx + stepSizeX * (double)veci.s1);
    
    double2 y0 ;
    y0.s0 = (double)(posy + stepSizeY * (double)vecj.s0);
    y0.s1 = (double)(posy + stepSizeY * (double)vecj.s1);
   

    double2 x = x0;
    double2 y = y0;
    
    uint iter=0;
    // double2 tmp;
    int2 stay;
    int2 ccount = 0;
	// double pow_f2 = pow_f ;
	// foo haha;
	// pow_f2 = haha.eat_my_float( pow_f );
	double2  LOGpowd2 = (double)log(pow_f) ;
	int ipow_f = (int)pow_f ;

    stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
    stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
    
    double2 savx = x;
    double2 savy = y;
	double2 Rx = ONEd2, Ry = ZEROd2;
	// Complexf4 savC = C, Z(ZEROc4), R ;
	double2 t;

    for(iter=0; (stay.s0 | stay.s1) && (iter < maxIterations); iter+= 16)
    {
        x = savx;
        y = savy;

		switch ( ipow_f ) 
		{
		default :
		case 2:   // handle mandel special  :-)  around 32.1 ms to 32.6 ms  vs 39.7 ms for double4 
			for(int wtf=0; wtf < 16;  wtf++) {
				t = x;
				x = MUL_ADD(-y, y, MUL_ADD(x, x, x0));
				y = MUL_ADD(2.0f * t, y, y0);
			} // end for 16 iter
			break;
		
		// odd powers are slightly different than even powers ... 
		case 3:
			for(int wtf=0; wtf < 16;  wtf++) {   // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 5:
			for(int wtf=0; wtf < 16;  wtf++) { // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 7:
			for(int wtf=0; wtf < 16;  wtf++) { // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		case 9:
			for(int wtf=0; wtf < 16;  wtf++) {  // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				
				t = MUL_ADD(-Ry, y, MUL_ADD(Rx, x, x0));     
				y = MUL_ADD(Rx, y, MUL_ADD(Ry, x, y0));
				x = t ;
			} // end for 16 iter
			break;
		
		// odd - Even power div ================= 
		case 4:
			for(int wtf=0; wtf < 16;  wtf++) {  // Z^4 == 64 ms to 77.5 ... faster than 126.1 ms for double4   
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 6:
			for(int wtf=0; wtf < 16;  wtf++) {  // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		case 8:
			for(int wtf=0; wtf < 16;  wtf++) {   // around  ms   to   ms
				Rx = x * x - y * y ;
				Ry = x * y + y * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = t * x - Ry * y ;
				Ry = t * y + Ry * x ;
				t = Rx * x - Ry * y ;
				Ry = Rx * y + Ry * x ;
				
				Rx = MUL_ADD(-Ry, y, MUL_ADD(t, x, x0));     
				y = MUL_ADD(t, y, MUL_ADD(Ry, x, y0));
				x = Rx ;
			} // end for 16 iter
			break;
		} // end switch  
		
		stay.s0 = (x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd;
        stay.s1 = (x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd;
        
        savx.s0 = (stay.s0 ? x.s0 : savx.s0);
        savx.s1 = (stay.s1 ? x.s1 : savx.s1);
        
		savy.s0 = (stay.s0 ? y.s0 : savy.s0);
        savy.s1 = (stay.s1 ? y.s1 : savy.s1);
        
		ccount += stay * 16 ; // was : *32;
    }
    
	// Handle remainder
    if (!(stay.s0 & stay.s1 ))
    {
        iter = 32;
        do
        {
            x = savx;
            y = savy;
            stay.s0 = ((x.s0 * x.s0 + y.s0 * y.s0) <= maglimitd) && 
						   (ccount.s0 < maxIterations);
            stay.s1 = ((x.s1 * x.s1 + y.s1 * y.s1) <= maglimitd) && 
				       (ccount.s1 < maxIterations);
            
            Rx = 1.0;  Ry = 0.0;
			for (int n=0; n < ipow_f ; n++) {
				// R *= Z;
				//  y = MUL_ADD(2.0 * tmp, y, y0); 
				t = MUL_ADD( Rx, x, - Ry * y );
				Ry = MUL_ADD( Rx, y, Ry * x );
				Rx = t;
			}
			x = Rx + x0 ;
			y = Ry + y0 ;
			
			ccount += stay;
            iter--;
            savx.s0 = (stay.s0 ? x.s0 : savx.s0);
            savx.s1 = (stay.s1 ? x.s1 : savx.s1);
            
            savy.s0 = (stay.s0 ? y.s0 : savy.s0);
            savy.s1 = (stay.s1 ? y.s1 : savy.s1);
            
        } while ((stay.s0 | stay.s1 ) && iter);
    } 
	
    
	x = savx;
    y = savy;
    
	
	double2 fMU   = convert_double2(ccount) + (( d2LOGLOGml - log(log( sqrt(convert_double2(x*x + y*y))   )) ) / LOGpowd2 ) ;  // fLOGpow4 ) ;   //  TWOf4 - ( (log( /* sqrt */ (convert_float4(x*x + y*y))   )) / log(FOURf4) )  ;   
	int2 _color = GetGradColord2( max(ZEROd2, fMU) );   
	
	uchar4 color[2];
	
	color[0].s0 =  _color.s0 & 0x0000ff ;
    color[0].s1 = (_color.s0 & 0x00ff00) >> 8  ;
    color[0].s2 = (_color.s0 & 0xff0000) >> 16 ;
    color[0].s3 = 0xff;
    if (ccount.s0 == maxIterations)
    {
        color[0].s0 = 0;
        color[0].s1 = 0;
        color[0].s2 = 0;
    }
    if (bench)
    {
        color[0].s0 = ccount.s0 & 0xff;
        color[0].s1 = (ccount.s0 & 0xff00) >> 8;
        color[0].s2 = (ccount.s0 & 0xff0000) >> 16;
        color[0].s3 = (ccount.s0 & 0xff000000) >> 24;
    }
    mandelbrotImage[2 * tid] = color[0];    
    
	color[1].s0 = _color.s1 & 0x0000ff ;
    color[1].s1 = (_color.s1 & 0x00ff00) >> 8  ;
    color[1].s2 = (_color.s1 & 0xff0000) >> 16 ;
    color[1].s3 = 0xff;
    if (ccount.s1 == maxIterations)
    {
        color[1].s0 = 0;
        color[1].s1 = 0;
        color[1].s2 = 0;
    }
    if (bench)
    {
        color[1].s0 = ccount.s1 & 0xff;
        color[1].s1 = (ccount.s1 & 0xff00) >> 8;
        color[1].s2 = (ccount.s1 & 0xff0000) >> 16;
        color[1].s3 = (ccount.s1 & 0xff000000) >> 24;
    }
    mandelbrotImage[2 * tid + 1] = color[1];    
    /*
	color[2].s0 = _color.s2 & 0x0000ff ;
    color[2].s1 = (_color.s2 & 0x00ff00) >> 8  ;
    color[2].s2 = (_color.s2 & 0xff0000) >> 16 ;
    color[2].s3 = 0xff;
    if (ccount.s2 == maxIterations)
    {
        color[2].s0 = 0;
        color[2].s1 = 0;
        color[2].s2 = 0;
    }
    if (bench)
    {
        color[2].s0 = ccount.s2 & 0xff;
        color[2].s1 = (ccount.s2 & 0xff00) >> 8;
        color[2].s2 = (ccount.s2 & 0xff0000) >> 16;
        color[2].s3 = (ccount.s2 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 2] = color[2];    
    
	color[3].s0 = _color.s3 & 0x0000ff ;
    color[3].s1 = (_color.s3 & 0x00ff00) >> 8  ;
    color[3].s2 = (_color.s3 & 0xff0000) >> 16 ;
    color[3].s3 = 0xff;
    if (ccount.s3 == maxIterations)
    {
        color[3].s0 = 0;
        color[3].s1 = 0;
        color[3].s2 = 0;
    }
    if (bench)
    {
        color[3].s0 = ccount.s3 & 0xff;
        color[3].s1 = (ccount.s3 & 0xff00) >> 8;
        color[3].s2 = (ccount.s3 & 0xff0000) >> 16;
        color[3].s3 = (ccount.s3 & 0xff000000) >> 24;
    }
    mandelbrotImage[4 * tid + 3] = color[3];
	*/ 
}


#endif // ENABLE_DOUBLE
