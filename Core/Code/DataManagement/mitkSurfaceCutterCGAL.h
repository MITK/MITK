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

#ifndef mitkSurfaceCutterCGAL_h
#define mitkSurfaceCutterCGAL_h

#include <memory>
#include "mitkISurfaceCutter.h"

namespace mitk
{
  /**
    * \brief Class for cutting surfaces using CGAL
    * \ingroup Data
    */
      class SurfaceCutterCGALPrivate;
      class MITK_CORE_EXPORT SurfaceCutterCGAL : public ISurfaceCutter
      {
      public:
          SurfaceCutterCGAL();
          virtual ~SurfaceCutterCGAL();

          virtual vtkSmartPointer<vtkPolyData> cutWithPlane(const mitk::Point3D planePoints[4]) const;
          virtual void setPolyData(vtkPolyData* surface);

      private:
          std::unique_ptr<SurfaceCutterCGALPrivate> p;
      };
} // namespace mitk

#endif // mitkSurfaceCutterCGAL_h
