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

#include "MitkExtExports.h"
#include "mitkSurfaceToSurfaceFilter.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "mitkSurface.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkCellData.h"
#include "vtkLine.h"

#include "mitkImage.h"

namespace mitk {

  /**
  \brief Filter to compute the normales for contours based on vtkPolygons



    This filter takes a number of extracted contours and computes the normals for each
    contour edge point. The normals can be accessed by calling:

         filter->GetOutput(i)->GetVtkPolyData()->GetCellData()->GetNormals();

         See also the method GetNormalsAsSurface()

   Note: If a segmentation binary image is provided this filter assures that the computed normals
         do not point into the segmentation image

   $Author: fetzer$
*/
class MitkExt_EXPORT ComputeContourSetNormalsFilter : public SurfaceToSurfaceFilter
{
public:

  mitkClassMacro(ComputeContourSetNormalsFilter,SurfaceToSurfaceFilter);
  itkNewMacro(Self);

  itkSetMacro(SegmentationBinaryImage, mitk::Image::Pointer);

   /*
      \brief Returns the computed normals as a surface
  */
  mitk::Surface::Pointer GetNormalsAsSurface();

  //Resets the filter, i.e. removes all inputs and outputs
  void Reset();

  void SetMaxSpacing(double);

protected:
  ComputeContourSetNormalsFilter();
  virtual ~ComputeContourSetNormalsFilter();
  virtual void GenerateData();
  virtual void GenerateOutputInformation();

private:

  //The segmentation out of which the contours were extracted. Can be used to determine the direction of the normals
  mitk::Image::Pointer m_SegmentationBinaryImage;
  double m_MaxSpacing;

  unsigned int m_NegativeNormalCounter;
  unsigned int m_PositiveNormalCounter;

};//class

}//namespace
#endif
