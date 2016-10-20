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

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkAutoSegmentationTool.h"
#include "mitkBinaryThresholdULTool.h"
#include "mitkDataNode.h"

#include <itkImage.h>

namespace us {
class ModuleResource;
}

namespace mitk
{

  /**
  \brief Calculates the segmented volumes for binary images.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
  */
  class MITKSEGMENTATION_EXPORT BinaryThresholdTool : public BinaryThresholdULTool
  {
  public:

    Message1<double>     ThresholdingValueChanged;

    mitkClassMacro(BinaryThresholdTool, BinaryThresholdULTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    us::ModuleResource GetIconResource() const override;
    virtual const char* GetName() const override;

    virtual void SetThresholdValue(double value);

  protected:

    BinaryThresholdTool(); // purposely hidden
    virtual ~BinaryThresholdTool();

    virtual void runItkThreshold(Image::Pointer feedbackImage3D, Image::Pointer previewImage, unsigned int timeStep) override;
    virtual void updateThresholdValue() override;
  };

} // namespace

#endif

