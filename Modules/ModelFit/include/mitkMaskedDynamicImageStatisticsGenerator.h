/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaskedDynamicImageStatisticsGenerator_h
#define mitkMaskedDynamicImageStatisticsGenerator_h

#include <mitkImage.h>

#include "MitkModelFitExports.h"

namespace mitk
{
/** Simple mitk based wrapper for the itk::MaskedNaryStatisticsImageFilter.
 * takes an input image and a mask image (both mitk::Images) and calculates the statistic
 * of the input image within the given mask (every pixel != 0).\n
 * The class assumes that the mask image is 3D (only one time step), if this is not the case
 * *only* the first time step will be used as mask.\n
 * If the input image has multiple time steps, the statistics will be calculated for each time
 * step. This the result arrays will always have as many values as the input image
 * has time steps.*/
class MITKMODELFIT_EXPORT MaskedDynamicImageStatisticsGenerator : public itk::Object
{
public:
  mitkClassMacroItkParent(MaskedDynamicImageStatisticsGenerator, itk::Object);

  itkNewMacro(Self);

    typedef itk::Array<double> ResultType;

    itkSetConstObjectMacro(DynamicImage,Image);
    itkGetConstObjectMacro(DynamicImage,Image);

    itkSetConstObjectMacro(Mask, Image);
    itkGetConstObjectMacro(Mask, Image);

    const ResultType& GetMaximum();
    const ResultType& GetMinimum();
    const ResultType& GetMean();
    const ResultType& GetSigma();
    const ResultType& GetVariance();
    const ResultType& GetSum();

    void Generate();

protected:
    MaskedDynamicImageStatisticsGenerator();
    ~MaskedDynamicImageStatisticsGenerator() override;

    template <typename TPixel, unsigned int VDim>
    void DoCalculateStatistics(const itk::Image<TPixel, VDim>* image);

    virtual void CheckValidInputs() const;

    bool HasOutdatedResults() const;

    itk::TimeStamp m_GenerationTimeStamp;

private:
    Image::ConstPointer m_DynamicImage;
    Image::ConstPointer m_Mask;

    typedef itk::Image<unsigned short, 3> InternalMaskType;
    InternalMaskType::ConstPointer m_InternalMask;

    ResultType m_Maximum;
    ResultType m_Minimum;
    ResultType m_Mean;
    ResultType m_Sigma;
    ResultType m_Variance;
    ResultType m_Sum;
};

}

#endif
