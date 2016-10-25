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
#ifndef __mitkLabelSetImageSurfaceStampFilter_h
#define __mitkLabelSetImageSurfaceStampFilter_h

#include "MitkMultilabelExports.h"

// MITK
#include "mitkImageToImageFilter.h"
#include <itkImage.h>
#include <mitkImage.h>
#include <mitkSurface.h>

namespace mitk
{
  class MITKMULTILABEL_EXPORT LabelSetImageSurfaceStampFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(LabelSetImageSurfaceStampFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkGetConstMacro(Surface, Surface::Pointer);
    itkSetMacro(Surface, Surface::Pointer);
    itkGetConstMacro(ForceOverwrite, bool);
    itkSetMacro(ForceOverwrite, bool);

  private:
    /*!
    \brief standard constructor
    */
    LabelSetImageSurfaceStampFilter();
    /*!
    \brief standard destructor
    */
    ~LabelSetImageSurfaceStampFilter();
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    virtual void GenerateData() override;

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing(itk::Image<TPixel, VImageDimension> *itkImage, mitk::Image::Pointer resultImage);

    Surface::Pointer m_Surface;
    bool m_ForceOverwrite;
  };
} // END mitk namespace
#endif
