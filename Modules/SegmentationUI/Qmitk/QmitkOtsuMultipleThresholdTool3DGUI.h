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

#ifndef QmitkOtsuMultipleThresholdTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "ui_QmitkOtsuMultipleThresholdTool3DGUIControls.h"

namespace mitk {
  class OtsuMultipleThresholdTool3D;
}

//class QListWidgetItem;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::OtsuTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkOtsuMultipleThresholdTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkOtsuMultipleThresholdTool3DGUI, QmitkToolGUI);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnRun();

    void OnCancel();

    void OnAcceptPreview();

    void OnShowInformation(bool);

    void OnItemSelectionChanged(QListWidgetItem *item);

  protected:

    QmitkOtsuMultipleThresholdTool3DGUI();
    virtual ~QmitkOtsuMultipleThresholdTool3DGUI();

    void BusyStateChanged(bool);

    mitk::OtsuMultipleThresholdTool3D* m_OtsuMultipleThresholdTool3D;

    Ui_QmitkOtsuMultipleThresholdTool3DGUIControls m_Controls;

    int m_NumberOfRegions;

    QListWidgetItem* m_SelectedItem;
};

#endif
