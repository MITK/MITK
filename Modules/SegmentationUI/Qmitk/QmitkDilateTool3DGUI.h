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

#ifndef QmitkDilateTool3DGUI_h_Included
#define QmitkDilateTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "ui_QmitkDilateTool3DGUIControls.h"


namespace mitk {
  class DilateTool3D;
}

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::DilateTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkDilateTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkDilateTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkDilateTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkDilateTool3DGUI();
    virtual ~QmitkDilateTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkDilateTool3DGUIControls m_Controls;

    mitk::DilateTool3D* m_DilateTool3D;
};

#endif
