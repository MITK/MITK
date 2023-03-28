/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkGrowCutToolGUI_h
#define QmitkGrowCutToolGUI_h

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"

#include "ui_QmitkGrowCutToolWidgetControls.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::GrowCutTool.
  \sa mitk::

  This GUI shows ...

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkGrowCutToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkGrowCutToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:

  void OnPreviewBtnClicked();

private slots:

  void OnAdvancedSettingsButtonToggled(bool toggled);

protected:
  QmitkGrowCutToolGUI();
  ~QmitkGrowCutToolGUI() override;

  void ConnectNewTool(mitk::SegWithPreviewTool *newTool) override;

  void InitializeUI(QBoxLayout *mainLayout) override;

  void SetValueOfDistancePenaltySlider(double val);

  void SetValueOfDistancePenaltyDoubleSpinBox(int val);

  void EnableWidgets(bool enabled) override;

  Ui_QmitkGrowCutToolWidgetControls m_Controls;

  bool m_FirstPreviewComputation = false;
};

#endif
