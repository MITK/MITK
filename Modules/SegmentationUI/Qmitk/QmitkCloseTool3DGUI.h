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

#ifndef QmitkCloseTool3DGUI_h_Included
#define QmitkCloseTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkCloseTool3D.h"

#include "ui_QmitkCloseTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::CloseTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkCloseTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkCloseTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkCloseTool3DGUI);

  signals:

    /// \brief Emitted when button "Run" is pressed
    void Run();

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnAcceptPreview();
    void OnCalculateDifference();
    void OnKernelSizeChanged(int);
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkCloseTool3DGUI();
    virtual ~QmitkCloseTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkCloseTool3DGUIControls m_Controls;

    mitk::CloseTool3D::Pointer m_CloseTool3D;
};

#endif
