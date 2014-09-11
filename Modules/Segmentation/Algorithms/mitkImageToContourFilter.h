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

#ifndef mitkImageToContourFilter_h_Included
#define mitkImageToContourFilter_h_Included

//#include "MitkSBExports.h"
#include <MitkSegmentationExports.h>
#include "itkImage.h"
#include "mitkImage.h"
#include "itkContourExtractor2DImageFilter.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include "vtkCellArray.h"

#include "mitkProgressBar.h"

namespace mitk {

/**
  \brief A filter that can extract contours out of a 2D binary image

  This class takes an 2D mitk::Image as input and extracts all contours which are drawn it. The contour
  extraction is done by using the itk::ContourExtractor2DImageFilter.

  The output is a mitk::Surface.

  $Author: fetzer$
*/
class MitkSegmentation_EXPORT ImageToContourFilter : public ImageToSurfaceFilter
{
 public:

   mitkClassMacro(ImageToContourFilter,ImageToSurfaceFilter);
   itkFactorylessNewMacro(Self)
   itkCloneMacro(Self)

    /**
      \brief Set macro for the geometry of the slice. If it is not set explicitly the geometry will be taken from the slice

      \a Parameter The slice`s geometry
    */
   itkSetMacro(SliceGeometry, BaseGeometry*);

   //typedef unsigned int VDimension;
   typedef itk::PolyLineParametricPath<2> PolyLineParametricPath2D;
   typedef PolyLineParametricPath2D::VertexListType ContourPath;

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

 protected:
   ImageToContourFilter();
   virtual ~ImageToContourFilter();
   virtual void GenerateData();
   virtual void GenerateOutputInformation();

 private:
   const BaseGeometry* m_SliceGeometry;
   bool m_UseProgressBar;
   unsigned int m_ProgressStepSize;

   template<typename TPixel, unsigned int VImageDimension>
   void Itk2DContourExtraction (const itk::Image<TPixel, VImageDimension>* sliceImage);

};//class

}//namespace
#endif
