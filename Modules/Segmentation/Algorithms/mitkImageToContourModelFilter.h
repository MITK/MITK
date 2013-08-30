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

#ifndef _mitkImageToContourModelFilter_h__
#define _mitkImageToContourModelFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"
#include <mitkImage.h>

#include "itkContourExtractor2DImageFilter.h"

namespace mitk {

  /**
  *
  * \brief Base class for all filters that generate a mitk::ContourModel from a mitk::Image
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class Segmentation_EXPORT ImageToContourModelFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ImageToContourModelFilter, ContourModelSource);
    itkNewMacro(Self);

    typedef mitk::Image InputType;
    typedef itk::PolyLineParametricPath<2> PolyLineParametricPath2D;
    typedef PolyLineParametricPath2D::VertexListType ContourPath;

    virtual void SetInput( const InputType *input);
    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);
    const InputType* GetInput(unsigned int idx);

    /**
      \brief Set macro for the geometry of the slice. If it is not set explicitly the geometry will be taken from the slice

      \a Parameter The slice`s geometry
    */
   itkSetMacro(SliceGeometry, Geometry3D*);

    /**
     \brief Set whether the mitkProgressBar should be used

     \a Parameter true for using the progress bar, false otherwise
   */
   void SetUseProgressBar(bool);

  protected:
    ImageToContourModelFilter();
    virtual ~ImageToContourModelFilter();

   virtual void GenerateData();
   virtual void GenerateOutputInformation();

  private:
   const Geometry3D* m_SliceGeometry;
   bool m_UseProgressBar;
   unsigned int m_ProgressStepSize;

   template<typename TPixel, unsigned int VImageDimension>
   void ExtractContoursITKProcessing (itk::Image<TPixel, VImageDimension>* sliceImage);

  };

}

#endif
