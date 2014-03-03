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

#ifndef QmitkBinaryThresholdULToolGUI_h_Included
#define QmitkBinaryThresholdULToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "SegmentationUIExports.h"

#include "ui_QmitkBinaryThresholdULToolGUIControls.h"

namespace mitk {
  class BinaryThresholdULTool;
}

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/
class SegmentationUI_EXPORT QmitkBinaryThresholdULToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkBinaryThresholdULToolGUI, QmitkToolGUI);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void OnThresholdingIntervalBordersSet(mitk::ScalarType lower, mitk::ScalarType upper, bool isFloat);
    void OnThresholdingValuesSet(mitk::ScalarType lower, mitk::ScalarType upper);

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);
    void OnCancel();
    void OnAcceptPreview();
    void OnInvertPreview();
    void OnNewLabel();
    void OnShowInformation(bool);
    void OnShowAdvancedControls(bool);

    void OnThresholdsChanged(double min, double max);

  protected:

    QmitkBinaryThresholdULToolGUI();
    virtual ~QmitkBinaryThresholdULToolGUI();

    Ui::QmitkBinaryThresholdULToolGUIControls m_Controls;

    void BusyStateChanged(bool value);

    bool m_SelfCall;

    mitk::BinaryThresholdULTool* m_BinaryThresholdULTool;
};

#endif

