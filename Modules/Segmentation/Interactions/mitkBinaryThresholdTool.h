/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdTool_h_Included
#define mitkBinaryThresholdTool_h_Included

#include "mitkBinaryThresholdBaseTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
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
  class MITKSEGMENTATION_EXPORT BinaryThresholdTool : public BinaryThresholdBaseTool
  {
  public:

    mitkClassMacro(BinaryThresholdTool, BinaryThresholdBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    const char *GetName() const override;

    virtual void SetThresholdValue(double value);

  protected:
    BinaryThresholdTool(); // purposely hidden
    ~BinaryThresholdTool() override;
  };

} // namespace

#endif
