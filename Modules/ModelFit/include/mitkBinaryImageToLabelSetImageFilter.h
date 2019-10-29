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

#ifndef mitkBinaryImageToLabelSetImageFilter_h
#define mitkBinaryImageToLabelSetImageFilter_h

#include <mitkImageToImageFilter.h>

#include "mitkCommon.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** \brief Converts an binary image to a LabelSetImage. The amount of labels equals the connected components.
   */
  class MITKMODELFIT_EXPORT BinaryImageToLabelSetImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BinaryImageToLabelSetImageFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    void SetInput(const InputImageType* image) override;
    void SetInput(unsigned int index, const InputImageType* image) override;

    itkSetMacro(FullyConnected, bool);
    itkGetConstMacro(FullyConnected, bool);

    itkSetMacro(ForegroundValue, unsigned int);
    itkGetConstMacro(ForegroundValue, unsigned int);

    itkSetMacro(OutputIsLabelSetImage, bool);
    itkGetConstMacro(OutputIsLabelSetImage, bool);

  private:
    using Superclass::SetInput;

    BinaryImageToLabelSetImageFilter() = default;
    ~BinaryImageToLabelSetImageFilter() override = default;

    template <typename TPixel, unsigned int VImageDimension>
    void ApplyBinaryImageToLabelMapFilter(const itk::Image<TPixel, VImageDimension>* inputImage);

    void GenerateData() override;
    void VerifyInputInformation() override;
    void VerifyInputImage(const mitk::Image* inputImage);

    bool m_FullyConnected = true;
    unsigned int m_ForegroundValue = 1;
    bool m_OutputIsLabelSetImage = false;
  };
}

#endif
