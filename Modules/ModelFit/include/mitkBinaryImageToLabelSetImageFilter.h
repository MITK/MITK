/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

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
    void VerifyInputInformation() const override;
    void VerifyInputImage(const mitk::Image* inputImage) const;

    bool m_FullyConnected = true;
    unsigned int m_ForegroundValue = 1;
    bool m_OutputIsLabelSetImage = false;
  };
}

#endif
