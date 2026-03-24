/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiLabelMaskGenerator_h
#define mitkMultiLabelMaskGenerator_h

#include <MitkImageStatisticsExports.h>
#include <mitkMaskGenerator.h>
#include <mitkLabelSetImage.h>


namespace mitk
{
/**
 * @brief Class that allows to generate masks (for statistic computation) out of multi label segmentations
 */
class MITKIMAGESTATISTICS_EXPORT MultiLabelMaskGenerator: public MaskGenerator
{
public:
  /** Standard Self typedef */
  mitkClassMacro(MultiLabelMaskGenerator, MaskGenerator);
  itkNewMacro(Self);

  unsigned int GetNumberOfMasks() const override;

  itkSetConstObjectMacro(MultiLabelSegmentation, MultiLabelSegmentation);

protected:
  MultiLabelMaskGenerator() = default;
  ~MultiLabelMaskGenerator() = default;

  Image::ConstPointer DoGetMask(unsigned int maskID) override;

private:
    mitk::MultiLabelSegmentation::ConstPointer m_MultiLabelSegmentation;
};

}

#endif
