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

#ifndef QmitkFillHolesTool3DGUI_h_Included
#define QmitkFillHolesTool3DGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"

#include "ui_QmitkFillHolesTool3DGUIControls.h"

namespace mitk {
  class FillHolesTool3D;
}

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::FillHolesTool3D.

  This GUI shows ...

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkFillHolesTool3DGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkFillHolesTool3DGUI, QmitkToolGUI);
    itkNewMacro(QmitkFillHolesTool3DGUI);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnRun();
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

  protected:
    QmitkFillHolesTool3DGUI();
    virtual ~QmitkFillHolesTool3DGUI();

    void BusyStateChanged(bool);

    Ui::QmitkFillHolesTool3DGUIControls m_Controls;

    mitk::FillHolesTool3D* m_FillHolesTool3D;
};

#endif
