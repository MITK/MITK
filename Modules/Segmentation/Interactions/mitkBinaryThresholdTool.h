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

#ifndef mitkBinaryThresholdTool_h_Included
#define mitkBinaryThresholdTool_h_Included

#include "SegmentationExports.h"
#include "mitkSegTool3D.h"

#include <itkImage.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief A tool for image thresholding.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class Segmentation_EXPORT BinaryThresholdTool : public SegTool3D
  {
  public:

    Message3<mitk::ScalarType, mitk::ScalarType, bool> IntervalBordersChanged;
    Message1<mitk::ScalarType>                         ThresholdingValueChanged;

    mitkClassMacro(BinaryThresholdTool, SegTool3D);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const;
    us::ModuleResource GetIconResource() const;
    virtual const char* GetName() const;

    virtual void Activated();
    virtual void Deactivated();

    /// \brief Sets the current threshold value from GUI.
    virtual void SetThresholdValue(mitk::ScalarType value);

    /// \brief Adds the preview image to the current working image.
    virtual void AcceptPreview();

    /// \brief Creates a new label out of the the current preview image.
    virtual void CreateNewLabel(const std::string& name, const mitk::Color& color);

  protected:

    BinaryThresholdTool(); // purposely hidden
    virtual ~BinaryThresholdTool();

    template <typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
    void InternalAcceptPreview( itk::Image<TPixel1, VDimension1>* targetImage, const itk::Image<TPixel2, VDimension2>* sourceImage );

    DataNode::Pointer m_NodeForThresholding;

    mitk::ScalarType m_SensibleMinimumThresholdValue;
    mitk::ScalarType m_SensibleMaximumThresholdValue;
    mitk::ScalarType m_CurrentThresholdValue;
    bool m_IsFloatImage;

  };

} // namespace

#endif

