/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTransferFunctionWidget_h
#define QmitkTransferFunctionWidget_h

#include <MitkQtWidgetsExtExports.h>

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkSimpleHistogram.h>
#include <mitkTransferFunctionProperty.h>

#include <QPushButton>
#include <QSlider>
#include <memory>

namespace mitk
{
  class BaseRenderer;
}

namespace Ui
{
  class QmitkTransferFunctionWidget;
}

/**
 * \brief Widget for editing the three components of a volume rendering transfer function.
 *
 * Provides interactive canvases for:
 * - Scalar opacity (grayvalue to opacity mapping)
 * - Color (grayvalue to RGB color mapping)
 * - Gradient opacity (gradient magnitude to opacity mapping)
 *
 * Includes a range slider for zooming into a subrange of the scalar domain.
 * Each canvas shows the underlying histogram and supports adding, removing,
 * and dragging control points.
 *
 * \sa QmitkTransferFunctionGeneratorWidget, QmitkColorTransferFunctionCanvas,
 *     QmitkPiecewiseFunctionCanvas
 */
class MITKQTWIDGETSEXT_EXPORT QmitkTransferFunctionWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the transfer function widget.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkTransferFunctionWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Destructor. */
  ~QmitkTransferFunctionWidget() override;

  /**
   * \brief Set the data node whose transfer function to edit.
   *
   * Retrieves or creates a "TransferFunction" property on the node and
   * initializes the canvases with the function data and histogram.
   *
   * \param[in] node The data node (must contain image data).
   * \param[in] timestep The time step for multi-timepoint images.
   * \param[in] renderer Optional renderer for renderer-specific properties.
   */
  void SetDataNode(mitk::DataNode *node, mitk::TimeStepType timestep = 0, const mitk::BaseRenderer *renderer = nullptr);

  /**
   * \brief Set the label text for the scalar axis on all three canvases.
   * \param[in] scalarLabel The axis label (e.g. "Grayvalue" or "HU").
   */
  void SetScalarLabel(const QString &scalarLabel);

  /**
   * \brief Show or hide the scalar opacity function canvas.
   * \param[in] show True to show.
   */
  void ShowScalarOpacityFunction(bool show);

  /**
   * \brief Show or hide the color function canvas.
   * \param[in] show True to show.
   */
  void ShowColorFunction(bool show);

  /**
   * \brief Show or hide the gradient opacity function canvas.
   * \param[in] show True to show.
   */
  void ShowGradientOpacityFunction(bool show);

  /**
   * \brief Enable or disable the scalar opacity function canvas.
   * \param[in] enable True to enable.
   */
  void SetScalarOpacityFunctionEnabled(bool enable);

  /**
   * \brief Enable or disable the color function canvas.
   * \param[in] enable True to enable.
   */
  void SetColorFunctionEnabled(bool enable);

  /**
   * \brief Enable or disable the gradient opacity function canvas.
   * \param[in] enable True to enable.
   */
  void SetGradientOpacityFunctionEnabled(bool enable);

public slots:
  /**
   * \brief Set the x value for the scalar opacity canvas from a line edit.
   * \param[in] text The x value as text.
   */
  void SetXValueScalar(const QString text);

  /**
   * \brief Set the y value for the scalar opacity canvas from a line edit.
   * \param[in] text The y value as text.
   */
  void SetYValueScalar(const QString text);

  /**
   * \brief Set the x value for the gradient opacity canvas from a line edit.
   * \param[in] text The x value as text.
   */
  void SetXValueGradient(const QString text);

  /**
   * \brief Set the y value for the gradient opacity canvas from a line edit.
   * \param[in] text The y value as text.
   */
  void SetYValueGradient(const QString text);

  /**
   * \brief Set the x value for the color canvas from a line edit.
   * \param[in] text The x value as text.
   */
  void SetXValueColor(const QString text);

  /** \brief Refresh all three canvases from the current transfer function data. */
  void OnUpdateCanvas();

  /** \brief Update the visible ranges of all canvases from the range slider. */
  void UpdateRanges();

  /** \brief Recalculate the range slider step size based on the current range. */
  void UpdateStepSize();

  /** \brief Reset the range slider to its full extent. */
  void OnResetSlider();

  /**
   * \brief Handle range slider span changes and update canvas ranges.
   * \param[in] lower The new lower bound.
   * \param[in] upper The new upper bound.
   */
  void OnSpanChanged(double lower, double upper);

protected:
  std::unique_ptr<Ui::QmitkTransferFunctionWidget> m_Controls;
  mitk::TransferFunctionProperty::Pointer tfpToChange;
  mitk::SimpleHistogramCache histogramCache;
};

#endif
