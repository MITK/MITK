/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKOPENCL_H_HEADER_INCLUDED
#define MITKOPENCL_H_HEADER_INCLUDED

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenCL/cl.h>
#else
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#endif

#endif /* MITKOPENCL_H_HEADER_INCLUDED */
