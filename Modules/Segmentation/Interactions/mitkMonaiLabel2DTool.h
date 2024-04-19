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
    void WriteImage(const Image *, const std::string &) override;
    std::stringstream GetPointsAsListString(const mitk::BaseGeometry *, const PointSet::Pointer) override;
    void OnAddPositivePoint(StateMachineAction *, InteractionEvent *) override;
    void OnAddNegativePoint(StateMachineAction *, InteractionEvent *) override;
    void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) override;

  protected:
    MonaiLabel2DTool() = default;
    ~MonaiLabel2DTool() = default;
  };
}
#endif
