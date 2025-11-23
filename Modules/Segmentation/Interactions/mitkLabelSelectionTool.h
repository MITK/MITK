/*============================================================================
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSelectionTool_h
#define mitkLabelSelectionTool_h

#include "mitkCommon.h"
#include "mitkSegTool2D.h"
#include <mitkLabelHighlightGuard.h>

#include <MitkSegmentationExports.h>


namespace mitk
{
  /**
    \brief Tools that allows to inspect and select labels via the render window.

    \sa Tool
    \sa ContourModel

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    It is not a seg tool in a classic sense that manipulate label content. It is use to allow via interaction to identify and select labels
    via the render window.
  */
  class MITKSEGMENTATION_EXPORT LabelSelectionTool : public SegTool2D
  {
  public:
    mitkClassMacro(LabelSelectionTool, SegTool2D);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    Message<> IndicatedLabelsChanged;

    us::ModuleResource GetIconResource() const override;
    us::ModuleResource GetCursorIconResource() const override;
    const char** GetXPM() const override;

    const char* GetName() const override;

    MultiLabelSegmentation::LabelValueVectorType GetIndicatedLabels() const;

    itkGetConstMacro(CheckOnlyActiveGroup, bool);
    itkSetMacro(CheckOnlyActiveGroup, bool);
    itkBooleanMacro(CheckOnlyActiveGroup);

    void Activated() override;
    void Deactivated() override;

    void UpdateLabels();

  protected:
    LabelSelectionTool();
    ~LabelSelectionTool() override;

    virtual void OnMouseMoved(StateMachineAction*, InteractionEvent* interactionEvent);
    virtual void OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent);

    void ConnectActionsAndFunctions() override;

  private:
    LabelHighlightGuard m_HighlightGuard;
    Point3D m_LastCheckedIndex;
    Point3D m_LastCheckedPoint;
    bool m_CheckOnlyActiveGroup = false;
  };

} // namespace

#endif
