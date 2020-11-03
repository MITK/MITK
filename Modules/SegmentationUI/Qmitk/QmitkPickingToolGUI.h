/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPickingToolGUI_h_Included
#define QmitkPickingToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkPickingTool.h"
#include "ui_QmitkPickingToolGUIControls.h"
#include <MitkSegmentationUIExports.h>

class QSlider;
class QLabel;
class QFrame;
class QPushButton;
#include <QCheckBox>

#include "QmitkStepperAdapter.h"

class QmitkPickingToolGUIControls;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::LiveWireTool.
\sa mitk::PickingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkPickingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkPickingToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    protected slots :

    void OnNewToolAssociated(mitk::Tool *);

  void OnConfirmSegmentation();

protected:
  QmitkPickingToolGUI();
  ~QmitkPickingToolGUI() override;

  Ui::QmitkPickingToolGUIControls m_Controls;

  mitk::PickingTool::Pointer m_PickingTool;
};

#endif
