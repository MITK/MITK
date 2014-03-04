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

#ifndef _mitkImageToContourModelSetFilter_h__
#define _mitkImageToContourModelSetFilter_h__

#include "mitkCommon.h"
#include "MitkContourModelExports.h"
#include "mitkContourModelSet.h"
#include "mitkContourModelSetSource.h"
#include <mitkImage.h>

#include "itkPolyLineParametricPath.h"

namespace mitk {

  /**
  *
  * \brief Base class for all filters that generate a mitk::ContourModel from a mitk::Image
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class MitkContourModel_EXPORT ImageToContourModelSetFilter : public ContourModelSetSource
  {

  public:

    mitkClassMacro(ImageToContourModelSetFilter, ContourModelSetSource);
    itkNewMacro(Self);

    typedef mitk::Image InputType;
    typedef itk::PolyLineParametricPath<2> PolyLineParametricPath2D;
    typedef PolyLineParametricPath2D::VertexListType ContourPath;

    virtual void SetInput( const InputType *input);

    const InputType* GetInput(void);

   /**
     \brief Set macro to apply a geometry to the extracted contours.
   */
   itkSetMacro(SliceGeometry, Geometry3D*);

   itkSetMacro(ContourValue, double);
   /**
     \brief Set whether the mitkProgressBar should be used

     \a Parameter true for using the progress bar, false otherwise
   */
   void SetUseProgressBar(bool);

  protected:
    ImageToContourModelSetFilter();
    virtual ~ImageToContourModelSetFilter();

   virtual void GenerateData();
   virtual void GenerateOutputInformation();

  private:
   const Geometry3D* m_SliceGeometry;
   bool m_UseProgressBar;
   unsigned int m_ProgressStepSize;

   double m_ContourValue;

   template<typename TPixel, unsigned int VImageDimension>
   void ExtractContoursITKProcessing (itk::Image<TPixel, VImageDimension>* sliceImage);

  };

}

#endif
