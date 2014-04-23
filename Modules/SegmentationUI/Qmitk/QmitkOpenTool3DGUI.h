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

#ifndef QmitkOpenTool3DGUI_h_Included
#define QmitkOpenTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "MitkSegmentationUIExports.h"

namespace mitk {
  class OpenTool3D;
}

#include "ui_QmitkOpenTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::OpenTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkOpenTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkOpenTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkOpenTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnKernelSizeChanged(int);
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkOpenTool3DGUI();
    virtual ~QmitkOpenTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkOpenTool3DGUIControls m_Controls;

    mitk::OpenTool3D* m_OpenTool3D;
};

#endif
