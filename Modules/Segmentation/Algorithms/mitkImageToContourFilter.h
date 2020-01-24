/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToContourFilter_h_Included
#define mitkImageToContourFilter_h_Included

//#include "MitkSBExports.h"
#include "itkContourExtractor2DImageFilter.h"
#include "itkImage.h"
#include "mitkImage.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkSurface.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"
#include <MitkSegmentationExports.h>

#include "mitkProgressBar.h"

namespace mitk
{
  /**
    \brief A filter that can extract contours out of a 2D binary image

    This class takes an 2D mitk::Image as input and extracts all contours which are drawn it. The contour
    extraction is done by using the itk::ContourExtractor2DImageFilter.

    The output is a mitk::Surface.

    $Author: fetzer$
  */
  class MITKSEGMENTATION_EXPORT ImageToContourFilter : public ImageToSurfaceFilter
  {
  public:
    mitkClassMacro(ImageToContourFilter, ImageToSurfaceFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
        \brief Set macro for the geometry of the slice. If it is not set explicitly the geometry will be taken from the
        slice

        \a Parameter The slice`s geometry
      */
      itkSetMacro(SliceGeometry, BaseGeometry *);

    // typedef unsigned int VDimension;
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
    ~ImageToContourFilter() override;
    void GenerateData() override;
    void GenerateOutputInformation() override;

  private:
    const BaseGeometry *m_SliceGeometry;
    bool m_UseProgressBar;
    unsigned int m_ProgressStepSize;

    template <typename TPixel, unsigned int VImageDimension>
    void Itk2DContourExtraction(const itk::Image<TPixel, VImageDimension> *sliceImage);

  }; // class

} // namespace
#endif
