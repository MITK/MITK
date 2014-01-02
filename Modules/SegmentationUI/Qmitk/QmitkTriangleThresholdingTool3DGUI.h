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

#ifndef QmitkTriangleThresholdingTool3DGUI_h_Included
#define QmitkTriangleThresholdingTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "ui_QmitkTriangleThresholdingTool3DGUIControls.h"

namespace mitk {
  class TriangleThresholdingTool3D;
}

//class QListWidgetItem;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::TriangleThresholdingTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkTriangleThresholdingTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkTriangleThresholdingTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkTriangleThresholdingTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnRun();

    void OnCancel();

    void OnAcceptPreview();

    void OnInvertPreview();

    void OnShowInformation(bool);

    void OnShowAdvancedControls(bool);

  protected:

    QmitkTriangleThresholdingTool3DGUI();
    virtual ~QmitkTriangleThresholdingTool3DGUI();

    void BusyStateChanged(bool);

    mitk::TriangleThresholdingTool3D* m_TriangleThresholdingTool3D;

    Ui_QmitkTriangleThresholdingTool3DGUIControls m_Controls;
};

#endif
