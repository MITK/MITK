/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkEditableContourToolGUIBase_h_Included
#define QmitkEditableContourToolGUIBase_h_Included

#include "QmitkToolGUI.h"
#include "mitkEditableContourTool.h"
#include "ui_QmitkEditableContourToolGUIControls.h"
#include <MitkSegmentationUIExports.h>

class QmitkEditableContourToolGUIBaseControls;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::EditableContourTool based classes.
\sa mitk::LassoTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkEditableContourToolGUIBase : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkEditableContourToolGUIBase, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots :

  void OnNewToolAssociated(mitk::Tool *);

  void OnConfirmSegmentation();

  void OnClearContour();

  void OnAutoConfirm(bool on);
  void OnAddModeToogled(bool on);

  void OnShowInformation(bool on);

protected:
  QmitkEditableContourToolGUIBase();
  ~QmitkEditableContourToolGUIBase() override;

  Ui::QmitkEditableContourToolGUIControls m_Controls;

  mitk::EditableContourTool::Pointer m_NewTool;
};

#endif
