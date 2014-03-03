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

#ifndef QmitkFastMarchingToolGUI_h_Included
#define QmitkFastMarchingToolGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>

namespace mitk {
  class FastMarchingTool;
}

#include "ui_QmitkFastMarchingToolGUIControls.h"
/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::FastMarchingTool.
\sa mitk::FastMarchingTool
*/
class MitkSegmentationUI_EXPORT QmitkFastMarchingToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkFastMarchingToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void OnSigmaValueSet(mitk::ScalarType value);
  void OnAlphaValueSet(mitk::ScalarType value);
  void OnBetaValueSet(mitk::ScalarType value);
  void OnStopValueSet(mitk::ScalarType value);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnAlphaChanged(double);
    void OnBetaChanged(double);
    void OnSigmaChanged(double);
    void OnStopValueChanged(double);
    void OnAcceptPreview();
    void OnCancel();
    void OnClearSeeds();
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

protected:

  QmitkFastMarchingToolGUI();
  virtual ~QmitkFastMarchingToolGUI();

  void BusyStateChanged(bool);

  Ui::QmitkFastMarchingToolGUIControls m_Controls;

  mitk::FastMarchingTool* m_FastMarchingTool;

  bool m_SelfCall;
};

#endif
