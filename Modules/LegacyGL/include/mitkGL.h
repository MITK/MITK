/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkGL.h
 * \brief Platform-specific include header for OpenGL.
 *
 * On Windows, includes \c windows.h first. On Apple, includes
 * \c OpenGL/gl.h; on other platforms, uses \c vtk_glad.h.
 */
#ifndef mitkGL_h
#define mitkGL_h

#ifdef WIN32
#include <windows.h>
#endif

#ifndef __APPLE__
//#include "GL/gl.h"
#include <vtk_glad.h>
#else
#include <OpenGL/gl.h>
#endif

#endif
