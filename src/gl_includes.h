// Inclui OpenGL, GLU e GLUT em qualquer sistema (como na aula 06).
#ifndef GL_INCLUDES_H
#define GL_INCLUDES_H

#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #ifdef _WIN32
        #include <windows.h>
    #endif
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#endif
