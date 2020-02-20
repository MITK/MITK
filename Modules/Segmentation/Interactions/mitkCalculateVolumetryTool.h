/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCalculateVolumetryTool_h_Included
#define mitkCalculateVolumetryTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
    \brief Calculates the segmented volumes for binary images.

    \ingroup ToolManagerEtAl
    \sa mitk::Tool
    \sa QmitkInteractiveSegmentation

    Last contributor: $Author$
  */
  class MITKSEGMENTATION_EXPORT CalculateVolumetryTool : public SegmentationsProcessingTool
  {
  public:
    mitkClassMacro(CalculateVolumetryTool, SegmentationsProcessingTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char **GetXPM() const override;
    const char *GetName() const override;

  protected:
    bool ProcessOneWorkingData(DataNode *node) override;
    std::string GetErrorMessage() override;

    void FinishProcessingAllData() override;

    CalculateVolumetryTool(); // purposely hidden
    ~CalculateVolumetryTool() override;
  };

} // namespace

#endif
