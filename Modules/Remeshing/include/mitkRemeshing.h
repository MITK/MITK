/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRemeshing_h
#define mitkRemeshing_h

#include <mitkSurface.h>
#include <MitkRemeshingExports.h>

namespace mitk
{
  namespace Remeshing
  {
    /** \brief Reduce the number of vertices of an mitk::Surface.
      *
      * The decimation is applied separately to all time steps of the input surface.
      * The meshes of the resulting surface are guaranteed to consist of triangles only.
      *
      * \param[in] input Input surface
      * \param[in] percent Relative number of vertices after decimation [0, 1]
      * \param[in] calculateNormals Calculate normals after decimation (\c true by default)
      * \param[in] flipNormals Flip calculated normals (\c false by default)
      *
      * \return Decimated surface
      */
    MITKREMESHING_EXPORT Surface::Pointer Decimate(const Surface* input, double percent, bool calculateNormals = true, bool flipNormals = false);
  }
}

#endif
