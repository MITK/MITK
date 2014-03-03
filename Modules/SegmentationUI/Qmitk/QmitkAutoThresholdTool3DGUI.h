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

#ifndef QmitkAutoThresholdTool3DGUI_h_Included
#define QmitkAutoThresholdTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "ui_QmitkAutoThresholdTool3DGUIControls.h"

namespace mitk {
  class AutoThresholdTool3D;
}

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::AutoThresholdTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkAutoThresholdTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkAutoThresholdTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkAutoThresholdTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnRun();

    void OnCancel();

    void OnAcceptPreview();

    void OnInvertPreview();

    void OnShowInformation(bool);

    void OnShowAdvancedControls(bool);

    void OnMethodChanged(int);

  protected:

    QmitkAutoThresholdTool3DGUI();
    virtual ~QmitkAutoThresholdTool3DGUI();

    void BusyStateChanged(bool);

    mitk::AutoThresholdTool3D* m_AutoThresholdTool3D;

    Ui_QmitkAutoThresholdTool3DGUIControls m_Controls;
};

#endif
