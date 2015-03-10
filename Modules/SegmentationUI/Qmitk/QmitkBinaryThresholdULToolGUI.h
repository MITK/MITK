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
#include <MitkSegmentationUIExports.h>
#include "mitkBinaryThresholdULTool.h"
#include "ctkRangeWidget.h"


/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdULToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkBinaryThresholdULToolGUI, QmitkToolGUI);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void OnThresholdingIntervalBordersChanged(double lower, double upper);
    void OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnAcceptThresholdPreview();

    void OnThresholdsChanged(double min, double max);

  protected:
    QmitkBinaryThresholdULToolGUI();
    virtual ~QmitkBinaryThresholdULToolGUI();

    ctkRangeWidget* m_DoubleThresholdSlider;

    mitk::BinaryThresholdULTool::Pointer m_BinaryThresholdULTool;
};

#endif

