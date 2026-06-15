/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkBinaryThresholdToolGUIBase_h
#define QmitkBinaryThresholdToolGUIBase_h

#include <QmitkSegWithPreviewToolGUIBase.h>
#include <ctkRangeWidget.h>
#include <ctkSliderWidget.h>

#include <MitkSegmentationUIExports.h>

/**
  \brief Base GUI class for threshold-based segmentation tools.

  Provides the common UI elements for both single-threshold and upper/lower threshold
  segmentation tools. Contains a slider or range widget for adjusting threshold values
  and connects to the tool's threshold change events.

  \sa mitk::BinaryThresholdTool
  \sa QmitkBinaryThresholdToolGUI
  \sa QmitkBinaryThresholdULToolGUI
  \sa QmitkSegWithPreviewToolGUIBase
*/
class MITKSEGMENTATIONUI_EXPORT QmitkBinaryThresholdToolGUIBase : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkBinaryThresholdToolGUIBase, QmitkSegWithPreviewToolGUIBase);

  /**
   * \brief Called when the tool's threshold interval borders change.
   * \param[in] lower The new lower border of the valid threshold range.
   * \param[in] upper The new upper border of the valid threshold range.
   * \param[in] isFloat True if the image pixel type is floating-point.
   */
  void OnThresholdingIntervalBordersChanged(double lower, double upper, bool isFloat);

  /**
   * \brief Called when the tool's current threshold values change.
   * \param[in] lower The new lower threshold value.
   * \param[in] upper The new upper threshold value.
   */
  void OnThresholdingValuesChanged(mitk::ScalarType lower, mitk::ScalarType upper);

protected slots:

  void OnThresholdRangeChanged(double min, double max);
  void OnThresholdSliderChanged(double value);

protected:
  QmitkBinaryThresholdToolGUIBase(bool ulMode);
  ~QmitkBinaryThresholdToolGUIBase() override;

  void DisconnectOldTool(mitk::SegWithPreviewTool* oldTool) override;
  void ConnectNewTool(mitk::SegWithPreviewTool* newTool) override;
  void InitializeUI(QBoxLayout* mainLayout) override;

  void BusyStateChanged(bool) override;

  ctkRangeWidget* m_ThresholdRange = nullptr;
  ctkSliderWidget* m_ThresholdSlider = nullptr;

  /** Indicates if the tool UI is used for a tool with upper an lower threshold (true)
  or only with one threshold (false)*/
  bool m_ULMode;

  bool m_InternalUpdate = false;
};

#endif
