/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTransferFunctionGeneratorWidget_h
#define QmitkTransferFunctionGeneratorWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>
#include <memory>

namespace Ui
{
  class QmitkTransferFunctionGeneratorWidget;
}

/**
 * \brief Widget for generating transfer functions via presets, threshold, and bell modes.
 *
 * Provides a tabbed interface with three transfer function generation modes:
 * - **Presets**: Load/save transfer function XML files and select named presets.
 * - **Threshold**: Interactive threshold-based transfer function using crosshair drag.
 * - **Bell**: Interactive bell-shaped (level/window) transfer function using crosshair drag.
 *
 * The widget operates on the "TransferFunction" property of a mitk::DataNode.
 *
 * \sa QmitkTransferFunctionWidget, QmitkCrossWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkTransferFunctionGeneratorWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the transfer function generator widget.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkTransferFunctionGeneratorWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkTransferFunctionGeneratorWidget() override;

  /**
   * \brief Set the data node whose transfer function to generate/modify.
   * \param[in] node The data node (image or unstructured grid).
   * \param[in] timestep The time step for multi-timepoint data.
   */
  void SetDataNode(mitk::DataNode *node, mitk::TimeStepType timestep = 0);

  /**
   * \brief Add a named preset to the presets combo box.
   * \param[in] presetName The display name for the preset.
   * \return The index of the added item in the combo box.
   */
  int AddPreset(const QString &presetName);

  /**
   * \brief Enable or disable the presets tab.
   * \param[in] enable True to enable.
   */
  void SetPresetsTabEnabled(bool enable);

  /**
   * \brief Enable or disable the threshold tab.
   * \param[in] enable True to enable.
   */
  void SetThresholdTabEnabled(bool enable);

  /**
   * \brief Enable or disable the bell (level/window) tab.
   * \param[in] enable True to enable.
   */
  void SetBellTabEnabled(bool enable);

public slots:
  /** \brief Save the current transfer function to an XML file. */
  void OnSavePreset();

  /** \brief Load a transfer function from an XML file. */
  void OnLoadPreset();

  /**
   * \brief Update the bell-shaped transfer function based on crosshair drag.
   * \param[in] dx Horizontal delta (shifts center position).
   * \param[in] dy Vertical delta (changes width).
   */
  void OnDeltaLevelWindow(int dx, int dy);

  /**
   * \brief Update the threshold transfer function based on crosshair drag.
   * \param[in] dx Horizontal delta (shifts threshold position).
   * \param[in] dy Vertical delta (changes width).
   */
  void OnDeltaThreshold(int dx, int dy);

signals:
  /**
   * \brief Emitted when a preset is selected from the combo box.
   * \param[in] mode The index of the selected preset.
   */
  void SignalTransferFunctionModeChanged(int mode);

  /** \brief Emitted when the transfer function canvas should be refreshed. */
  void SignalUpdateCanvas();

protected slots:

  void OnPreset(int mode);

protected:
  std::unique_ptr<Ui::QmitkTransferFunctionGeneratorWidget> m_Controls;
  mitk::TransferFunctionProperty::Pointer tfpToChange;

  double histoMinimum;
  double histoMaximum;

  double thPos;
  double thDelta;

  double deltaScale;
  double deltaMax;
  double deltaMin;

  const mitk::Image::HistogramType *histoGramm;

  QString presetFileName;

  double ScaleDelta(int d) const;
};

#endif
