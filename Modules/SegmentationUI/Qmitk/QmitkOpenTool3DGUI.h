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
#include "SegmentationUIExports.h"
#include "mitkOpenTool3D.h"

#include "ui_QmitkOpenTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::OpenTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkOpenTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkOpenTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkOpenTool3DGUI);

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
    QmitkOpenTool3DGUI();
    virtual ~QmitkOpenTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkOpenTool3DGUIControls m_Controls;

    mitk::OpenTool3D::Pointer m_OpenTool3D;
};

#endif
