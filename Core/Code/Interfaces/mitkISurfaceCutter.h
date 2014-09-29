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

#ifndef mitkISurfaceCutter_h
#define mitkISurfaceCutter_h

#include "MitkCoreExports.h"

#include <mitkPoint.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

namespace mitk
{
  /**
    * \brief Interface for surface cutting algorithms.
    * \ingroup Data
    */
    class MITK_CORE_EXPORT ISurfaceCutter
    {
    public:
        virtual ~ISurfaceCutter() {}

        /** Cut the polydata associated with this cutter with a rectangle
         *  \param planePoints the points defining the cutting rectangle.  
         *  \return Result of the cut. 
         */
        virtual vtkSmartPointer<vtkPolyData> cutWithPlane(const mitk::Point3D planePoints[4]) const = 0;
        
        /** Rebuild the cutter's internal data structures for a new polyData
         *  \param surface the polyData to cut through.  
         */
        virtual void setPolyData(vtkPolyData* surface) = 0;
    };
}

#endif
