/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNewAddTool2DGUI_h_Included
#define QmitkNewAddTool2DGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkNewAddTool2D.h"
#include "ui_QmitkNewAddTool2DGUIControls.h"
#include <MitkSegmentationUIExports.h>

class QSlider;
class QLabel;
class QFrame;
class QPushButton;
#include <QCheckBox>

#include "QmitkStepperAdapter.h"

class QmitkNewAddTool2DGUIControls;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::NewAddTool.
\sa mitk::NewAddTool2D
*/
class MITKSEGMENTATIONUI_EXPORT QmitkNewAddTool2DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkNewAddTool2DGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

    protected slots :

    void OnNewToolAssociated(mitk::Tool *);

  void OnConfirmSegmentation();

  void OnClearSegmentation();

  void OnShowInformation(bool on);

protected:
  QmitkNewAddTool2DGUI();
  ~QmitkNewAddTool2DGUI() override;

  Ui::QmitkNewAddTool2DGUIControls m_Controls;

  mitk::NewAddTool2D::Pointer m_NewAddTool;
};

#endif
