/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdULToolGUI_h_Included
#define QmitkBinaryThresholdULToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "ctkRangeWidget.h"
#include <qcheckbox.h>
#include "mitkBinaryThresholdULTool.h"

#include <MitkSegmentationUIExports.h>

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
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);
  void OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper);

signals:

public slots:

protected slots:

  void OnNewToolAssociated(mitk::Tool *);

  void OnAcceptThresholdPreview();

  void OnThresholdsChanged(double min, double max);

protected:
  QmitkBinaryThresholdULToolGUI();
  ~QmitkBinaryThresholdULToolGUI() override;

  void BusyStateChanged(bool) override;

  ctkRangeWidget *m_DoubleThresholdSlider;
  QCheckBox* m_CheckProcessAll = nullptr;
  QCheckBox* m_CheckCreateNew = nullptr;

  mitk::BinaryThresholdULTool::Pointer m_BinaryThresholdULTool;
};

#endif
