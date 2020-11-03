/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKGL_H_HEADER_INCLUDED_C1C53722
#define MITKGL_H_HEADER_INCLUDED_C1C53722

#ifdef WIN32
#include <windows.h>
#endif

#ifndef __APPLE__
//#include "GL/gl.h"
#include "vtk_glew.h"
#else
#include "OpenGL/gl.h"
#endif

#endif /* MITKGL_H_HEADER_INCLUDED_C1C53722 */
