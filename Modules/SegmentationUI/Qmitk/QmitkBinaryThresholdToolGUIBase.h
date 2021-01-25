/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdToolGUIBase_h_Included
#define QmitkBinaryThresholdToolGUIBase_h_Included

#include "QmitkAutoSegmentationToolGUIBase.h"
#include "ctkRangeWidget.h"
#include "ctkSliderWidget.h"

#include <MitkSegmentationUIExports.h>

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief Base GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdToolGUIBase : public QmitkAutoSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkBinaryThresholdToolGUIBase, QmitkAutoSegmentationToolGUIBase);

  void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);
  void OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper);

protected slots:

  void OnThresholdRangeChanged(double min, double max);
  void OnThresholdSliderChanged(double value);

protected:
  QmitkBinaryThresholdToolGUIBase(bool ulMode);
  ~QmitkBinaryThresholdToolGUIBase() override;

  void DisconnectOldTool(mitk::AutoSegmentationWithPreviewTool* oldTool) override;
  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void BusyStateChanged(bool) override;

  ctkRangeWidget* m_ThresholdRange = nullptr;
  ctkSliderWidget* m_ThresholdSlider = nullptr;

  /** Indicates if the tool UI is used for a tool with upper an lower threshold (true)
  ore only with one threshold (false)*/
  bool m_ULMode;

  bool m_InternalUpdate = false;
};

#endif
