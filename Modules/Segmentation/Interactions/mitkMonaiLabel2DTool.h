/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabel2DTool_h
#define mitkMonaiLabel2DTool_h

#include "mitkMonaiLabelTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
  \brief MonaiLabel segmentation 2D tool.

  \ingroup ToolManagerEtAl
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  */
  class MITKSEGMENTATION_EXPORT MonaiLabel2DTool : public MonaiLabelTool
  {
  public:
    mitkClassMacro(MonaiLabel2DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;

    void Activated() override;
    void WriteImage(const Image *inputAtTimeStep, const std::string &inputImagePath) const override;
    void WriteBackResults(MultiLabelSegmentation *previewImage, MultiLabelSegmentation *segResults, TimeStepType timeStep) const override;
    void ConfirmCleanUp() override;

  protected:
    MonaiLabel2DTool() = default;
    ~MonaiLabel2DTool() = default;
  };
}
#endif
