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

#ifndef QmitkConnectedRegionSelectorTool3DGUI_h_Included
#define QmitkConnectedRegionSelectorTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"

namespace mitk {
  class ConnectedRegionSelectorTool3D;
}

#include "ui_QmitkConnectedRegionSelectorTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::ConnectedRegionSelectorTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkConnectedRegionSelectorTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkConnectedRegionSelectorTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkConnectedRegionSelectorTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkConnectedRegionSelectorTool3DGUI();
    virtual ~QmitkConnectedRegionSelectorTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkConnectedRegionSelectorTool3DGUIControls m_Controls;

    mitk::ConnectedRegionSelectorTool3D* m_ConnectedRegionSelectorTool3D;
};

#endif
