/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkMonaiLabel3DTool_h
#define mitkMonaiLabel3DTool_h

#include "mitkMonaiLabelTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief MonaiLabel segmentation 3D tool.

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT MonaiLabel3DTool : public MonaiLabelTool
  {
  public:
    mitkClassMacro(MonaiLabel3DTool, MonaiLabelTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char *GetName() const override;
    const char **GetXPM() const override;
    us::ModuleResource GetIconResource() const override;
    void Activated() override;
    void OnAddPositivePoint(StateMachineAction *, InteractionEvent *) override;
    void OnAddNegativePoint(StateMachineAction *, InteractionEvent *) override;
    std::stringstream GetPointsAsListString(const mitk::BaseGeometry *, const PointSet::Pointer) override;
    void WriteImage(const Image *, const std::string &) override;
    void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) override;

  protected:
    MonaiLabel3DTool() = default;
    ~MonaiLabel3DTool() = default;
  };
} // namespace mitk
#endif
