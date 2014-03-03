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

#ifndef QmitkKeepNConnectedRegionsTool3DGUI_h_Included
#define QmitkKeepNConnectedRegionsTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkKeepNConnectedRegionsTool3D.h"

#include "ui_QmitkKeepNConnectedRegionsTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::KeepNConnectedRegionsTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkKeepNConnectedRegionsTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkKeepNConnectedRegionsTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkKeepNConnectedRegionsTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnCancel();
    void OnNewLabel();
    void OnNumberOfConnectedRegionsToKeepChanged(int);
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnCalculateUnion();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkKeepNConnectedRegionsTool3DGUI();
    virtual ~QmitkKeepNConnectedRegionsTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkKeepNConnectedRegionsTool3DGUIControls m_Controls;

    mitk::KeepNConnectedRegionsTool3D::Pointer m_KeepNConnectedRegionsTool3D;
};

#endif
