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

#ifndef QmitkMedianTool3DGUI_h_Included
#define QmitkMedianTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"

namespace mitk {
  class MedianTool3D;
}

#include "ui_QmitkMedianTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::MedianTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkMedianTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkMedianTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkMedianTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnCancel();
    void OnNewLabel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnCalculateUnion();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);
    void OnKernelSizeChanged(int);

  protected:
    QmitkMedianTool3DGUI();
    virtual ~QmitkMedianTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkMedianTool3DGUIControls m_Controls;

    mitk::MedianTool3D* m_MedianTool3D;
};

#endif
