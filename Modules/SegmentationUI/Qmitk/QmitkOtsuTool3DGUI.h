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

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "ui_QmitkOtsuToolWidgetControls.h"

namespace mitk {
  class OtsuTool3D;
}

//class QListWidgetItem;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::OtsuTool3D

  This GUI shows ...

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkOtsuTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    mitkClassMacro(QmitkOtsuTool3DGUI, QmitkToolGUI);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnRun();

    void OnCancel();

    void OnItemSelectionChanged();

    void OnAcceptPreview();

    void OnShowInformation(bool);

  private slots:

    void OnAdvancedSettingsButtonToggled(bool toggled);

  protected:

    QmitkOtsuTool3DGUI();
    virtual ~QmitkOtsuTool3DGUI();

    void BusyStateChanged(bool);

    mitk::OtsuTool3D* m_OtsuMultipleThresholdTool3D;

    Ui_QmitkOtsuWidgetControls m_Controls;

    int m_NumberOfRegions;

    bool m_UseValleyEmphasis;

    int m_NumberOfBins;

    QList<QListWidgetItem *> m_SelectedItems;
};

#endif
