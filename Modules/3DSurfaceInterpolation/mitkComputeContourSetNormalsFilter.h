/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#ifndef mitkComputeContourSetNormalsFilter_h_Included
#define mitkComputeContourSetNormalsFilter_h_Included

#include <mitk3DSurfaceInterpolationExports.h>
#include "mitkSurfaceToSurfaceFilter.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "mitkSurface.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkCellData.h"
#include "vtkLine.h"

namespace mitk {

  /**
  \brief This filter takes a number of extracted contours and computes the normals for each
         contour edge point. The normals can be accessed by calling:

         filter->GetOutput(i)->GetVtkPolyData()->GetCellData()->GetNormals();

         See also the method GetNormalsAsSurface()

   \ingroup Process

   $Author: fetzer$
*/
class mitk3DSurfaceInterpolation_EXPORT ComputeContourSetNormalsFilter : public SurfaceToSurfaceFilter
{
public:

  mitkClassMacro(ComputeContourSetNormalsFilter,SurfaceToSurfaceFilter);
  itkNewMacro(Self);

   /*
      \brief Returns the computed normals as a surface 
  */
  mitk::Surface::Pointer GetNormalsAsSurface();

protected:
  ComputeContourSetNormalsFilter();
  virtual ~ComputeContourSetNormalsFilter();
  virtual void GenerateData();
  virtual void GenerateOutputInformation();

};//class

}//namespace
#endif