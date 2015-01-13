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

#ifndef QmitkLiveWireTool2DGUI_h_Included
#define QmitkLiveWireTool2DGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "mitkLiveWireTool2D.h"
#include "ui_QmitkLiveWireTool2DGUIControls.h"

class QSlider;
class QLabel;
class QFrame;
class QPushButton;
#include <QCheckBox>


#include "QmitkStepperAdapter.h"

class QmitkLiveWireTool2DGUIControls;

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::LiveWireTool.
\sa mitk::LiveWireTool2D
*/
class MITKSEGMENTATIONUI_EXPORT QmitkLiveWireTool2DGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkLiveWireTool2DGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnConfirmSegmentation();

    void OnClearSegmentation();

    void OnShowInformation( bool on );

protected:

  QmitkLiveWireTool2DGUI();
  virtual ~QmitkLiveWireTool2DGUI();

  Ui::QmitkLiveWireTool2DGUIControls m_Controls;

  mitk::LiveWireTool2D::Pointer m_LiveWireTool;
};

#endif
