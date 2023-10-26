/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKMONAILABEL2DTOOL_H
#define MITKMONAILABEL2DTOOL_H

#include "mitkSegWithPreviewTool.h"
#include "mitkMonaiLabelTool.h"
#include <MitkSegmentationExports.h>
#include <memory>
#include <unordered_map>
#include <set>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "mitkMonaiLabelTool.h"


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
    void WriteImage(const Image *, std::string &) override;
    std::stringstream GetPointsAsListString(const mitk::BaseGeometry *, PointSet::Pointer) override;
    void OnAddPositivePoint(StateMachineAction *, InteractionEvent *) override;
    void OnAddNegativePoint(StateMachineAction *, InteractionEvent *) override;
    void WriteBackResults(LabelSetImage *, LabelSetImage *, TimeStepType) override;

  protected:
    MonaiLabel2DTool() = default;
    ~MonaiLabel2DTool() = default;
  };
}
#endif
