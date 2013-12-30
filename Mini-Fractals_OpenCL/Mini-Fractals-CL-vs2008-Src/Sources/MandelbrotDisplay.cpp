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

#ifdef linux
# define GL_GLEXT_PROTOTYPES
#endif // !linux
#include <glew.h>
#include <glut.h>
#include <cstdlib>
#include <cstdio>

#include "..\headers\MandelbrotDisplay.hpp"
#include "..\headers\Mandelbrot.hpp"

// An instance of the Mandelbrot Class 
Mandelbrot clMandelbrot("OpenCL Mandelbrot fractal");

// Window height, Window Width and the pixels to be displayed 
int width;
int height;
unsigned char * output;

int mouseX = 0;
int mouseY = 0;
bool panning = false;

bool zoomIn = false;
bool zoomOut = false;

// display function 
void
displayFunc()
{
    if (!clMandelbrot.getBenched())
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, output);
        glFlush();
        glutSwapBuffers();
    }
}

// idle function 
void
idleFunc(void)
{
    if (panning)
    {
        if (mouseX < (width / 4))
        {
            clMandelbrot.setXPos(clMandelbrot.getXPos() - clMandelbrot.getXStep());
        }
        else if (mouseX > (3 * width / 4))
        {
            clMandelbrot.setXPos(clMandelbrot.getXPos() + clMandelbrot.getXStep());
        }
        if (mouseY < (height / 4))
        {
            clMandelbrot.setYPos(clMandelbrot.getYPos() + clMandelbrot.getYStep());
        }
        else if (mouseY > (3 * height / 4))
        {
            clMandelbrot.setYPos(clMandelbrot.getYPos() - clMandelbrot.getYStep());
        }
        if (zoomIn)
        {
            clMandelbrot.setXSize(clMandelbrot.getXSize() * 0.99);
        }
        else if (zoomOut)
        {
            clMandelbrot.setXSize(clMandelbrot.getXSize() / 0.99);
        }
    }
    clMandelbrot.run();
    //clMandelbrot.verifyResults();

    glutPostRedisplay();
}

// keyboard function 
void
keyboardFunc(unsigned char key, int mouseX, int mouseY)
{
    switch(key)
    {
        // If the user hits escape or Q, then exit 
        case GLUT_ESCAPE_KEY:
        case 'q':
        case 'Q':
            {
                cleanup();
                exit(0);
                break;
            }
        case 'c':
            {
                printf("center (%.13f, %.13f), window width %.13f\n", clMandelbrot.getXPos(), clMandelbrot.getYPos(),
                                                                      clMandelbrot.getXStep());
                break;
            }
		case 'n' :
			{
				printf("number of kernel WorkGroup Size (Cores) = %d \n\n", clMandelbrot.GetNumDevices() ) ;
				clMandelbrot.printStats();
				break;
			}
		case 'T' :
		case 't' :
			{
				printf("Calc Time = %s Seconds \n", clMandelbrot.ConvertTimeSpan( clMandelbrot.m_dRtime ).c_str() );
				break;
			}
        case 'i':
            {
                cl_uint maxIterations = clMandelbrot.getMaxIterations();
                maxIterations = ((maxIterations * 2) < MAX_ITER) ? maxIterations * 2 : MAX_ITER;
                printf("Setting maxIterations to %d\n", maxIterations);
                clMandelbrot.setMaxIterations(maxIterations);
                break;
            }
        case 'I':
            {
                cl_uint maxIterations = clMandelbrot.getMaxIterations();
                maxIterations = ((maxIterations / 2) > MIN_ITER) ? maxIterations / 2 : MIN_ITER;
                printf("Setting maxIterations to %d\n", maxIterations);
                clMandelbrot.setMaxIterations(maxIterations);
                break;
            }
        case 'b':
            {
                if (clMandelbrot.getTiming())
                {
                    clMandelbrot.setBench(1);
                }
				break;
            }
        case 'p':
            {
                panning = (panning == false)? true : false;
                break;
            }
		case '2' :
			{
				clMandelbrot.pow_F = 2.0 ; 
				break;
			}
		case '3' :
			{
				clMandelbrot.pow_F = 3.0 ; 
				break;
			}
		case '4' :
			{
				clMandelbrot.pow_F = 4.0 ; 
				break;
			}
		case '5' :
			{
				clMandelbrot.pow_F = 5.0 ; 
				break;
			}
		case '6' :
			{
				clMandelbrot.pow_F = 6.0 ; 
				break;
			}
		case '7' :
			{
				clMandelbrot.pow_F = 7.0 ; 
				break;
			}
		case '8' :
			{
				clMandelbrot.pow_F = 8.0 ; 
				break;
			}
		case '9' :
			{
				clMandelbrot.pow_F = 9.0 ; 
				break;
			}
        default:
            break;
    }
}

void mouseEntry(int state)
{
    if (state == GLUT_LEFT)
    {
        panning = false;
    }
    else
    {
        panning = true;
    }
}

void
mouseFunc(int button, int state, int x, int y)
{
    switch (button)
    {
        case GLUT_LEFT_BUTTON:
            {
                if ((state == GLUT_DOWN) && !zoomOut)
                {
                    zoomIn = true;
                }
                else
                {
                    zoomIn = false;
                }
                break;
            }
        case GLUT_RIGHT_BUTTON:
            {
                if ((state == GLUT_DOWN) && !zoomIn)
                {
                    zoomOut = true;
                }
                else
                {
                    zoomOut = false;
                }
                break;
            }
        default:
            break;
    }
    mouseX = x;
    mouseY = y;
}

void
motionFunc(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

void
passiveMotionFunc(int x, int y)
{
    mouseX = x;
    mouseY = y;
}

// initalise display 
void
initDisplay(int argc, char *argv[])
{
    initGlut(argc, argv);
    initGL();
}

// initalise glut 
void
initGlut(int argc, char *argv[])
{
    /* initialising the window */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    
    // Print available keyboard and mouse options.
    printf("\n\tMouse Options :\n"
           "\t  Move mouse to pan image\n"
           "\t  Left click to zoom in\n"
           "\t  Right click to zoom out\n"
		   "\n\tKeyboard Options :\n"
           "\t  'i'  double the number of iterations\n"
           "\t  'I'  halve the number of iterations\n"
           "\t  'b'  benchmark the current frame (when -t is enabled)\n"
           "\t  'c'  print the current center position\n"
           "\t  'p'  toggle panning\n"
		   "\t  't'  or 'T'  for Calc Time in seconds\n"
		   "\t  'n'  GPU Stats and # cores\n"
		   "\t  2-9  F  in  Z^F + C  \n"
		   "\t  'q'  or 'Q' to Quit\n"
           );

    printf("width %d, height %d\n", width, height);
	printf("iterations are set at 1024\n");
    glutInitWindowSize(width, height);
    mouseX = width / 2;
    mouseY = height / 2;
    glutInitWindowPosition(0,0);
    glutCreateWindow("Mandelbrot Fractal Generator");

    // the various glut callbacks 
    glutDisplayFunc(displayFunc);
    glutIdleFunc(idleFunc);
    glutKeyboardFunc(keyboardFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutPassiveMotionFunc(passiveMotionFunc);
    glutEntryFunc(mouseEntry);
}

// initalise OpenGL 
void
initGL(void)
{
    glewInit();
}

void
mainLoopGL(void)
{
    glutMainLoop();
}

// free any allocated resources 
void
cleanup(void)
{
    clMandelbrot.cleanup();
    clMandelbrot.printStats();

}

int 
main(int argc, char * argv[])
{
    // initalise and run the mandelbrot kernel 
	// char* my_argv[] = {"--fma"}; // fma is slower // 
	char* my_argv[] = {"--double"} ;  // {"--double", "-t"} ; // this works, but do not force a call to float functions! It WILL crash.
	argc = 1 ;   // also change mandelbrot.hpp constructor ... 
	// argc = 1 ;
	// printf("START APP !! \n"); 
	clMandelbrot.initialize();
	// printf("initialized ... \n");
	if(clMandelbrot.parseCommandLine(argc, argv))
        return SDK_FAILURE;
	//  printf("parsed command line ... \n");
    if(clMandelbrot.isDumpBinaryEnabled())
    {
        return clMandelbrot.genBinaryImage();
    }
    else
    {
        int returnVal = clMandelbrot.setup();
        // printf("setup ... \n");
		if(returnVal != SDK_SUCCESS)
            return (returnVal == SDK_EXPECTED_FAILURE)? SDK_SUCCESS : SDK_FAILURE;
		// fgz - devide here ...
        if(clMandelbrot.run()!=SDK_SUCCESS)
            return SDK_FAILURE;
        if(clMandelbrot.verifyResults()!=SDK_SUCCESS)
            return SDK_FAILURE;

        // show window if it is not running in quiet mode 
        if(clMandelbrot.showWindow())
        {
            width = clMandelbrot.getWidth();
            height = clMandelbrot.getHeight();
            output = (unsigned char *)clMandelbrot.getPixels();

            initDisplay(argc, argv);
            mainLoopGL();
        }
        cleanup();
    }
    return SDK_SUCCESS;
}
