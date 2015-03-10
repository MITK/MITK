/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPickingToolGUI_h_Included
#define QmitkPickingToolGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "mitkPickingTool.h"
#include "ui_QmitkPickingToolGUIControls.h"

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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnConfirmSegmentation();



protected:

  QmitkPickingToolGUI();
  virtual ~QmitkPickingToolGUI();

  Ui::QmitkPickingToolGUIControls m_Controls;

  mitk::PickingTool::Pointer m_PickingTool;
};

#endif
