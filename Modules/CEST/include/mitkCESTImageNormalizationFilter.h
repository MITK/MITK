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
#ifndef __mitkCESTImageNormalizationFilter_h
#define __mitkCESTImageNormalizationFilter_h

#include <MitkCESTExports.h>

// MITK
#include "mitkImageToImageFilter.h"

namespace mitk
{
  class MITKCEST_EXPORT CESTImageNormalizationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CESTImageNormalizationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

  protected:
    /*!
    \brief standard constructor
    */
    CESTImageNormalizationFilter();
    /*!
    \brief standard destructor
    */
    ~CESTImageNormalizationFilter();
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the normalized output image.
    */
    virtual void GenerateData() override;

    /** Internal templated method that normalizes across timesteps
    */
    template <typename TPixel, unsigned int VImageDimension>
    void NormalizeTimeSteps(const itk::Image<TPixel, VImageDimension>* image);

  };
} // END mitk namespace
#endif
