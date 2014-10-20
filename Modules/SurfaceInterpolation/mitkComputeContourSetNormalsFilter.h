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

#ifndef mitkComputeContourSetNormalsFilter_h_Included
#define mitkComputeContourSetNormalsFilter_h_Included

#include <MitkSurfaceInterpolationExports.h>
#include "mitkSurfaceToSurfaceFilter.h"
#include "mitkProgressBar.h"
#include "mitkSurface.h"

#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
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
class MitkSurfaceInterpolation_EXPORT ComputeContourSetNormalsFilter : public SurfaceToSurfaceFilter
{
public:

  mitkClassMacro(ComputeContourSetNormalsFilter,SurfaceToSurfaceFilter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)


   /*
      \brief Returns the computed normals as a surface
  */
  mitk::Surface::Pointer GetNormalsAsSurface();

  //Resets the filter, i.e. removes all inputs and outputs
  void Reset();

  void SetMaxSpacing(double);

  /**
    \brief Set whether the mitkProgressBar should be used

    \a Parameter true for using the progress bar, false otherwise
  */
  void SetUseProgressBar(bool);

  /**
    \brief Set the stepsize which the progress bar should proceed

    \a Parameter The stepsize for progressing
  */
  void SetProgressStepSize(unsigned int stepSize);

  void SetSegmentationBinaryImage(mitk::Image* segmentationImage)
  {
    m_SegmentationBinaryImage = segmentationImage;
  }

protected:
  ComputeContourSetNormalsFilter();
  virtual ~ComputeContourSetNormalsFilter();
  virtual void GenerateData();
  virtual void GenerateOutputInformation();

private:

  //The segmentation out of which the contours were extracted. Can be used to determine the direction of the normals
  mitk::Image* m_SegmentationBinaryImage;
  double m_MaxSpacing;

  unsigned int m_NegativeNormalCounter;
  unsigned int m_PositiveNormalCounter;

  bool m_UseProgressBar;
  unsigned int m_ProgressStepSize;

};//class

}//namespace
#endif
