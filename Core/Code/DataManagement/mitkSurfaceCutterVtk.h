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

#ifndef mitkSurfaceCutterVtk_h
#define mitkSurfaceCutterVtk_h

#include <memory>

#include "mitkISurfaceCutter.h"

namespace mitk
{
  /**
    * \brief Class for cutting surfaces using VTK
    * \ingroup Data
    */
      class SurfaceCutterVtkPrivate;
      class MITK_CORE_EXPORT SurfaceCutterVtk : public ISurfaceCutter
      {
      public:
          SurfaceCutterVtk();
          virtual ~SurfaceCutterVtk();

          virtual vtkSmartPointer<vtkPolyData> cutWithPlane(const mitk::Point3D planePoints[4]) const;
          virtual void setPolyData(vtkPolyData* surface);

      private:
          std::unique_ptr<SurfaceCutterVtkPrivate> p;
      };
} // namespace mitk

#endif // mitkSurfaceCutterVtk_h
