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
#include <mitkImage.h>
#include <mitkLabelSetImage.h>


namespace mitk
{
/**
 * @brief The MultiLabelMaskGenerator class NOT IMPLEMENTED YET!
 */
class MITKIMAGESTATISTICS_EXPORT MultiLabelMaskGenerator: public MaskGenerator
{
public:
   /* void setLabelSetImage(mitk::LabelSetImage::Pointer labelSetImage);

    void addLabel(LabelSetImage::PixelType, std::vector<mitk::Image::Pointer>::size_type layer=0);
    void removeLabel(LabelSetImage::PixelType, std::vector<mitk::Image::Pointer>::size_type layer=0);

    void addLabels(std::pair<std::vector<mitk::Image::Pointer>::size_type, std::vector<unsigned int>> labelsToAdd);
    void removeLabels(std::pair<std::vector<mitk::Image::Pointer>::size_type, std::vector<unsigned int>> labelsToAdd);

    void addLabels(std::vector<LabelSetImage::PixelType> labels, std::vector<mitk::Image::Pointer>::size_type layer=0);
    void removeLabels(std::vector<LabelSetImage::PixelType> labels, std::vector<mitk::Image::Pointer>::size_type layer=0);

    void removeLayer(std::vector<mitk::Image::Pointer>::size_type layer);

    mitk::Image::Pointer GetMask();

protected:

private:
    mitk::LabelSetImage::Pointer m_LabelSetImage;
    std::vector<std::vector<LabelSetImage::PixelType>> m_selectedLabels;*/

};

}

#endif
