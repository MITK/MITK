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

#ifndef QmitkSplitConnectedRegionsTool3DGUI_h_Included
#define QmitkSplitConnectedRegionsTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "MitkSegmentationUIExports.h"


#include "mitkSplitConnectedRegionsTool3D.h"

#include "ui_QmitkSplitConnectedRegionsTool3DGUIControls.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::SplitConnectedRegionsTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class MitkSegmentationUI_EXPORT QmitkSplitConnectedRegionsTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkSplitConnectedRegionsTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkSplitConnectedRegionsTool3DGUI);

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
    QmitkSplitConnectedRegionsTool3DGUI();
    virtual ~QmitkSplitConnectedRegionsTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkSplitConnectedRegionsTool3DGUIControls m_Controls;

    mitk::SplitConnectedRegionsTool3D* m_SplitConnectedRegionsTool3D;
};

#endif
