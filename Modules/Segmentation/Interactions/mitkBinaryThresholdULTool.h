/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryThresholdULTool_h
#define mitkBinaryThresholdULTool_h

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
  class MITKSEGMENTATION_EXPORT BinaryThresholdULTool : public BinaryThresholdBaseTool
  {
  public:
    mitkClassMacro(BinaryThresholdULTool, BinaryThresholdBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    const char *GetName() const override;

  protected:
    BinaryThresholdULTool(); // purposely hidden
    ~BinaryThresholdULTool() override;
  };

} // namespace

#endif
