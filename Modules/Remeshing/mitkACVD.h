/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkACVD_h
#define mitkACVD_h

#include <mitkSurface.h>
#include <RemeshingExports.h>

namespace mitk
{
  namespace ACVD
  {
    Remeshing_EXPORT Surface::Pointer Remesh(Surface::Pointer surface, int numVertices, double gradation, int subsampling = 10, double edgeSplitting = 0.0, bool forceManifold = false, bool boundaryFixing = false);
  }
}

#endif
