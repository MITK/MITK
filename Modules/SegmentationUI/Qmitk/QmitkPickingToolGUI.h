/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPickingToolGUI_h
#define QmitkPickingToolGUI_h

#include "QmitkSegWithPreviewToolGUIBase.h"
#include <MitkSegmentationUIExports.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::PickingTool.
\sa mitk::PickingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkPickingToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkPickingToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :
  void OnResetPicksClicked();
  void OnRadioPickClicked(bool checked);

protected:
  QmitkPickingToolGUI();
  ~QmitkPickingToolGUI() override;

  void InitializeUI(QBoxLayout* mainLayout) override;
  void EnableWidgets(bool enabled) override;

private:
  QWidget* m_ClearPicksBtn = nullptr;
  QWidget* m_RadioPick = nullptr;
  QWidget* m_RadioRelabel = nullptr;
};

#endif
