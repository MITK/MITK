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

#ifndef QmitkOtsuTool3DGUI_h_Included
#define QmitkOtsuTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"
#include "mitkOtsuTool3D.h"
#include "ui_QmitkOtsuTool3DGUIControls.h"

//class QListWidgetItem;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::OtsuTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkOtsuTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkOtsuTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkOtsuTool3DGUI);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnRun();

//    void OnAcceptPreview();

    void OnItemSelectionChanged(QListWidgetItem *item);

  protected:

    QmitkOtsuTool3DGUI();
    virtual ~QmitkOtsuTool3DGUI();

    mitk::OtsuTool3D::Pointer m_OtsuTool3D;

    Ui_QmitkOtsuTool3DGUIControls m_Controls;

    int m_NumberOfRegions;

    QListWidgetItem* m_SelectedItem;
};

#endif
