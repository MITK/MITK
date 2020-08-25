/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdToolGUI_h_Included
#define QmitkBinaryThresholdToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkBinaryThresholdTool.h"
#include <MitkSegmentationUIExports.h>

#include "ctkSliderWidget.h"
#include <qcheckbox.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  There is only a slider for INT values in QT. So, if the working image has a float/double pixeltype, we need to convert
  the original float intensity into a respective int value for the slider. The slider range is then between 0 and 99.

  If the pixeltype is INT, then we do not need any conversion.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdToolGUI : public QmitkToolGUI
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkBinaryThresholdToolGUI, QmitkToolGUI);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);
  void OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper);

signals:

  /// \brief Emitted when threshold Accepted
  void thresholdAccepted();

  /// \brief Emitted when threshold Canceled
  void thresholdCanceled();

public slots:

protected slots:

  void OnNewToolAssociated(mitk::Tool *);
  void OnAcceptThresholdPreview();

  void OnSliderValueChanged(double value);

protected:
  QmitkBinaryThresholdToolGUI();
  ~QmitkBinaryThresholdToolGUI() override;

  void BusyStateChanged(bool) override;

  ctkSliderWidget* m_ThresholdSlider = nullptr;
  QCheckBox* m_CheckProcessAll = nullptr;
  QCheckBox* m_CheckCreateNew = nullptr;

  bool m_InternalUpdate = false;

  mitk::BinaryThresholdTool::Pointer m_BinaryThresholdTool;
};

#endif
