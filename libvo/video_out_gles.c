#include "config.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "esUtil.h"

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>


#include "mpeg2.h"
#include "video_out.h"
#include "vo_internal.h"
#include "mpeg2convert.h"

typedef struct {
    void * data;
} gles_frame_t;

typedef struct gles_instance_s {
    vo_instance_t vo;
    gles_frame_t frame[3];
    int index;
    ESContext * esContext;
    int width;
    int height;
    Display * x_display;
    Window window;
} gles_instance_t;

static int open_display (gles_instance_t * instance, int width, int height)
{

	esInitContext ( instance->esContext );
	esCreateWindow( instance->esContext,"mpeg2try",width,height,ES_WINDOW_RGB );
		printf("GLES X11 window create succesfully\n");
//esRegisterDrawFunc ( &esContext, Draw );

        return 0;
}


static void gles_start_fbuf (vo_instance_t * _instance,
			    uint8_t * const * buf, void * id)
{
    gles_instance_t * instance = (gles_instance_t *) _instance;
    gles_frame_t * frame = (gles_frame_t *) id;

    //while (frame->wait_completion)
//	x11_event (instance);
}

static void gles_setup_fbuf (vo_instance_t * _instance,
			    uint8_t ** buf, void ** id)
{
    gles_instance_t * instance = (gles_instance_t *) _instance;

    buf[0] = (uint8_t *) instance->frame[instance->index].data;
    buf[1] = buf[2] = NULL;
    *id = instance->frame + instance->index++;



// Initialize GLES
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		CHECK_GLES();
		
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_BLEND);
		CHECK_GLES();
		glBlendFunc(GL_ONE, GL_SRC_COLOR);
		CHECK_GLES();
		
		glGenTextures(1, &_texId);
		CHECK_GLES();
		glGenBuffers(1, &_vertId);
		CHECK_GLES();
		glGenBuffers(1, &_indId);
		CHECK_GLES();
		
		glEnableVertexAttribArray(0);
		CHECK_GLES();
		glEnableVertexAttribArray(1);
		CHECK_GLES();
		
		/* Create a program */
		_program = glCreateProgram();
		CHECK_GLES();

		/* Load the two shaders */
		_vertex_shader   = load_shader("vertex", GL_VERTEX_SHADER);
		_fragment_shader = load_shader("fragment", GL_FRAGMENT_SHADER);

		/* Attach the shaders */
		glAttachShader( _program, _vertex_shader);
		CHECK_GLES();
		glAttachShader( _program, _fragment_shader);
		CHECK_GLES();

		/* Link the program */
		glLinkProgram( _program);
		CHECK_GLES();

		GLint linked;
		glGetProgramiv( _program, GL_LINK_STATUS, &linked);
		CHECK_GLES();

		/* Print errors if it failed */
		if(GL_FALSE == linked)
		{
			char buffer[256];
			glGetProgramInfoLog(_program, 255, NULL, buffer);
			CHECK_GLES();
			std::cout<<"load_program: Unable to link shader : \n"<<buffer<<"\n";
			assert(0);
		}

		/* Use it */
		glUseProgram(_program);
		CHECK_GLES();

		_pos_position = glGetAttribLocation(_program, "a_position");
		CHECK_GLES();
		_pos_texCoord = glGetAttribLocation(_program, "a_texCoord");
		CHECK_GLES();
		
		glBindBuffer( GL_ARRAY_BUFFER, _vertId);
		CHECK_GLES();
		glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		CHECK_GLES();
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indId );
		CHECK_GLES();
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW );
		CHECK_GLES();

		glFlush();
		CHECK_GLES();
	}




}

static void gles_draw_frame (vo_instance_t * _instance,
			    uint8_t * const * buf, void * id)
{
    gles_frame_t * frame;
    gles_instance_t * instance;

    frame = (gles_frame_t *) id;
    instance = (gles_instance_t *) _instance;
    printf("Call GLES to draw\n"); //(from v4l example)

void Window::render_gles()
{
	glUseProgram(_program);
	CHECK_GLES();

	glDisable( GL_DEPTH_TEST );
	CHECK_GLES();

	glEnable(GL_BLEND);
	CHECK_GLES();
	glBlendFunc(GL_ONE, GL_SRC_COLOR);
	CHECK_GLES();

	glActiveTexture(GL_TEXTURE0);
	CHECK_GLES();
	glBindTexture(GL_TEXTURE_2D, _texId);
	CHECK_GLES();
		
	glBindBuffer( GL_ARRAY_BUFFER, _vertId);
	CHECK_GLES();
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indId );
	CHECK_GLES();

	glEnableVertexAttribArray(0);
	CHECK_GLES();
	glEnableVertexAttribArray(1);
	CHECK_GLES();
	glVertexAttribPointer(_pos_position, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &((Vertex*)NULL)->pos[0] );
	CHECK_GLES();
	glVertexAttribPointer(_pos_texCoord, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), &((Vertex*)NULL)->tex[0] );
	CHECK_GLES();
	
	/*Don't remove this or the texture will go black */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	CHECK_GLES();
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	CHECK_GLES();

	glDrawElements( GL_TRIANGLES, NELEMS(indices), GL_UNSIGNED_SHORT, NULL);
	CHECK_GLES();


}




static int gles_alloc_frames (gles_instance_t * instance, int xshm)
{
    int size;
    char * alloc;
    int i = 0;

    return 1;
    //if (xshm && !instance->xshm_extension)
    //    	    return 1;

    //size = 0;
    //alloc = NULL;
    //while (i < 3) {
    //        instance->frame[i].wait_completion = 0;
    //        instance->frame[i].ximage = xshm ?
    //    	    XShmCreateImage (instance->display, instance->vinfo.visual,
    //    			    instance->vinfo.depth, ZPixmap, NULL /* data */,
    //    			    &(instance->shminfo),
    //    			    instance->width, instance->height) :
    //    	    XCreateImage(instance->display, instance->vinfo.visual,
    //    			    instance->vinfo.depth, ZPixmap, 0, NULL /* data */,
    //    			    instance->width, instance->height, 8, 0);
    //        if (instance->frame[i].ximage == NULL) {
    //    	    fprintf (stderr, "Cannot create ximage\n");
    //    	    return 1;
    //        } else if (xshm) {
    //    	    if (i == 0) {
    //    		    size = (instance->frame[0].ximage->bytes_per_line *
    //    				    instance->frame[0].ximage->height);
    //    		    alloc = (char *) create_shm (instance, 3 * size);
    //    	    } else if (size != (instance->frame[i].ximage->bytes_per_line *
    //    				    instance->frame[i].ximage->height)) {
    //    		    fprintf (stderr, "unexpected ximage data size\n");
    //    		    return 1;
    //    	    }
    //        } else
    //    	    alloc =
    //    		    (char *) malloc (instance->frame[i].ximage->bytes_per_line *
    //    				    instance->frame[i].ximage->height);
    //        instance->frame[i].data = instance->frame[i].ximage->data = alloc;
    //        i++;
    //        if (alloc == NULL) {
    //    	    while (--i >= 0)
    //    		    XDestroyImage (instance->frame[i].ximage);
    //    	    return 1;
    //        }
    //        alloc += size;
    //}

    //instance->xshm = xshm;
    //return 0;
}


static int common_setup (vo_instance_t * _instance, unsigned int width,
			 unsigned int height, unsigned int chroma_width,
			 unsigned int chroma_height,
			 vo_setup_result_t * result)
{
    gles_instance_t * instance = (gles_instance_t *) _instance;

    if (instance->esContext->x_display != NULL) {
	/* Already setup, just adjust to the new size */
        XResizeWindow (instance->esContext->x_display, instance->window, width, height);
    } else {
	/* Not setup yet, do the full monty */
        if (open_display (instance, width, height))
            return 1;
    }
    instance->vo.setup_fbuf = NULL;
    instance->vo.start_fbuf = NULL;
    instance->vo.set_fbuf = NULL;
    instance->vo.draw = NULL;
    instance->vo.discard = NULL;
    //instance->vo.close = gles_close;
    instance->width = width;
    instance->height = height;
    instance->index = 0;

    //if (!gles_alloc_frames (instance, 1) || !gles_alloc_frames (instance, 0)) {
    //    int bpp;

    //    instance->vo.setup_fbuf = gles_setup_fbuf;
    //    instance->vo.start_fbuf = gles_start_fbuf;
    //    instance->vo.draw = gles_draw_frame;
    //}
    //result->convert =  mpeg2convert_rgb (MPEG2CONVERT_RGB, 24);
//	if (result->convert == NULL) {
//	    fprintf (stderr, "%dbpp not supported\n", 24);
//	    return 1;
//	}


    return 0;
}


vo_instance_t * vo_gles_open (void)
{
	gles_instance_t * instance;

	instance = (gles_instance_t *) malloc (sizeof (gles_instance_t));
	instance->esContext = (ESContext *) malloc (sizeof (ESContext));
	if (instance == NULL)
		return NULL;

	instance->vo.setup = common_setup;
	instance->vo.close = (void (*) (vo_instance_t *)) free;
	return (vo_instance_t *) instance;
}


///
// CreateEGLContext()
//
//    Creates an EGL rendering context and all associated elements
//
EGLBoolean CreateEGLContext ( EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
		EGLContext* eglContext, EGLSurface* eglSurface,
		EGLint attribList[],Display* x_display)
{
	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

	// Get Display
	display = eglGetDisplay((EGLNativeDisplayType)x_display);
	if ( display == EGL_NO_DISPLAY )
	{
		return EGL_FALSE;
	}

	// Initialize EGL
	if ( !eglInitialize(display, &majorVersion, &minorVersion) )
	{
		return EGL_FALSE;
	}

	// Get configs
   if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Choose config
   if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
   {
      return EGL_FALSE;
   }

   // Create a surface
   surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return EGL_FALSE;
   }

   // Create a GL context
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return EGL_FALSE;
   }   
   
   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
   {
      return EGL_FALSE;
   }
   
   *eglDisplay = display;
   *eglSurface = surface;
   *eglContext = context;
   return EGL_TRUE;
} 


///
//  WinCreate()
//
//      This function initialized the native X11 display and window for EGL
//
EGLBoolean WinCreate(ESContext *esContext, const char *title)
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
    Window win;
    Display * x_display;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        return EGL_FALSE;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, esContext->width, esContext->height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );

    xattr.override_redirect = FALSE;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", FALSE);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = FALSE;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       FALSE,
       SubstructureNotifyMask,
       &xev );

    esContext->hWnd = (EGLNativeWindowType) win;
    esContext->x_display=x_display;
    return EGL_TRUE;
}


///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keypress, or
//      window close action.
//
GLboolean userInterrupt(ESContext *esContext)
{
    XEvent xev;
    KeySym key;
    GLboolean userinterrupt = GL_FALSE;
    Display * x_display=esContext->x_display;
    char text;

    // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );
        if ( xev.type == KeyPress )
        {
            if (XLookupString(&xev.xkey,&text,1,&key,0)==1)
            {
                if (esContext->keyFunc != NULL)
                    esContext->keyFunc(esContext, text, 0, 0);
            }
        }
        if ( xev.type == DestroyNotify )
            userinterrupt = GL_TRUE;
    }
    return userinterrupt;
}


//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

///
//  esInitContext()
//
//      Initialize ES utility context.  This must be called before calling any other
//      functions.
//
void esInitContext ( ESContext *esContext )
{
   if ( esContext != NULL )
   {
      memset( esContext, 0, sizeof( ESContext) );
   }
}


///
//  esCreateWindow()
//
//      title - name for title bar of window
//      width - width of window to create
//      height - height of window to create
//      flags  - bitwise or of window creation flags 
//          ES_WINDOW_ALPHA       - specifies that the framebuffer should have alpha
//          ES_WINDOW_DEPTH       - specifies that a depth buffer should be created
//          ES_WINDOW_STENCIL     - specifies that a stencil buffer should be created
//          ES_WINDOW_MULTISAMPLE - specifies that a multi-sample buffer should be created
//
GLboolean esCreateWindow ( ESContext *esContext, const char* title, GLint width, GLint height, GLuint flags )
{
   EGLint attribList[] =
   {
       EGL_RED_SIZE,       5,
       EGL_GREEN_SIZE,     6,
       EGL_BLUE_SIZE,      5,
       EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
       EGL_NONE
   };
   
   if ( esContext == NULL )
   {
      return GL_FALSE;
   }

   esContext->width = width;
   esContext->height = height;

   if ( !WinCreate ( esContext, title) )
   {
      return GL_FALSE;
   }

  
   if ( !CreateEGLContext ( esContext->hWnd,
                            &esContext->eglDisplay,
                            &esContext->eglContext,
                            &esContext->eglSurface,
                            attribList,esContext->x_display) )
   {
      return GL_FALSE;
   }
   

   return GL_TRUE;
}


///
//  esMainLoop()
//
//    Start the main loop for the OpenGL ES application
//

void esMainLoop ( ESContext *esContext )
{
    struct timeval t1, t2;
    struct timezone tz;
    float deltatime;
    float totaltime = 0.0f;
    unsigned int frames = 0;

    gettimeofday ( &t1 , &tz );

    while(userInterrupt(esContext) == GL_FALSE)
    {
        gettimeofday(&t2, &tz);
        deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
        t1 = t2;

        if (esContext->updateFunc != NULL)
            esContext->updateFunc(esContext, deltatime);
        if (esContext->drawFunc != NULL)
            esContext->drawFunc(esContext);

        eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);

        totaltime += deltatime;
        frames++;
        if (totaltime >  2.0f)
        {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
            totaltime -= 2.0f;
            frames = 0;
        }
    }
}


///
//  esRegisterDrawFunc()
//
void  esRegisterDrawFunc ( ESContext *esContext, void (ESCALLBACK *drawFunc) (ESContext* ) )
{
   esContext->drawFunc = drawFunc;
}


///
//  esRegisterUpdateFunc()
//
void  esRegisterUpdateFunc ( ESContext *esContext, void (ESCALLBACK *updateFunc) ( ESContext*, float ) )
{
   esContext->updateFunc = updateFunc;
}


///
//  esRegisterKeyFunc()
//
void  esRegisterKeyFunc ( ESContext *esContext,
                                    void (ESCALLBACK *keyFunc) (ESContext*, unsigned char, int, int ) )
{
   esContext->keyFunc = keyFunc;
}


///
// esLogMessage()
//
//    Log an error message to the debug output for the platform
//
void  esLogMessage ( const char *formatStr, ... )
{
    va_list params;
    char buf[BUFSIZ];

    va_start ( params, formatStr );
    vsprintf ( buf, formatStr, params );
    
    printf ( "%s", buf );
    
    va_end ( params );
}


///
// esLoadTGA()
//
//    Loads a 24-bit TGA image from a file. This is probably the simplest TGA loader ever.
//    Does not support loading of compressed TGAs nor TGAa with alpha channel. But for the
//    sake of the examples, this is sufficient.
//

char*  esLoadTGA ( char *fileName, int *width, int *height )
{
    char *buffer = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;

    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;

    if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }

    if(fread(attributes, sizeof(attributes), 1, f) == 0)
    {
        fclose(f);
        return 0;
    }

    *width = attributes[1] * 256 + attributes[0];
    *height = attributes[3] * 256 + attributes[2];
    imagesize = attributes[4] / 8 * *width * *height;
    buffer = malloc(imagesize);
    if (buffer == NULL)
    {
        fclose(f);
        return 0;
    }

    if(fread(buffer, 1, imagesize, f) != imagesize)
    {
        free(buffer);
        return NULL;
    }
    fclose(f);
    return buffer;
}
