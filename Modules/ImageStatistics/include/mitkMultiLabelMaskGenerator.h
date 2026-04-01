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
 * \brief Generates masks for statistics computation from multi-label segmentations.
 *
 * This mask generator takes a MultiLabelSegmentation and produces one binary
 * mask per label. Each mask isolates a single label from the segmentation,
 * allowing per-label statistics to be computed via ImageStatisticsCalculator.
 *
 * The number of masks returned by GetNumberOfMasks() equals the number of
 * labels in the segmentation.
 *
 * \sa MaskGenerator
 * \sa ImageStatisticsCalculator
 * \sa MultiLabelSegmentation
 */
class MITKIMAGESTATISTICS_EXPORT MultiLabelMaskGenerator: public MaskGenerator
{
public:
  /** Standard Self typedef */
  mitkClassMacro(MultiLabelMaskGenerator, MaskGenerator);
  itkNewMacro(Self);

  /**
   * \brief Get the number of masks (one per label in the segmentation).
   * \return Number of labels in the multi-label segmentation.
   */
  unsigned int GetNumberOfMasks() const override;

  /**
   * \brief Set the multi-label segmentation from which to generate masks.
   * \param[in] _arg Const pointer to the MultiLabelSegmentation.
   */
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
