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
#include "QmitkExtExports.h"
#include "mitkBinaryThresholdULTool.h"
#include <qxtspanslider.h>

#include <QSpinBox>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/
class QmitkExt_EXPORT QmitkBinaryThresholdULToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkBinaryThresholdULToolGUI, QmitkToolGUI);
    itkNewMacro(QmitkBinaryThresholdULToolGUI);

    void OnThresholdingIntervalBordersChanged(int lower, int upper);
    void OnThresholdingValuesChanged(int lower, int upper);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnSpanChanged(int lower, int upper);
    void OnAcceptThresholdPreview();
    void OnOneSpinnerChanged();

  protected:
    QmitkBinaryThresholdULToolGUI();
    virtual ~QmitkBinaryThresholdULToolGUI();

    QxtSpanSlider *m_RangeSlider;
    QSpinBox* m_LowerSpinner;
    QSpinBox* m_UpperSpinner;

    mitk::BinaryThresholdULTool::Pointer m_BinaryThresholdULTool;
};

#endif

