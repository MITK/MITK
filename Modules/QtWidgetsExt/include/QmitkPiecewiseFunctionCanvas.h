/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPiecewiseFunctionCanvas_h
#define QmitkPiecewiseFunctionCanvas_h

#include <MitkQtWidgetsExtExports.h>
#include <QmitkTransferFunctionCanvas.h>

#include <vtkPiecewiseFunction.h>

/**
 * \brief Canvas widget for editing a vtkPiecewiseFunction (opacity transfer function).
 *
 * Renders a piecewise linear function as connected line segments with draggable
 * control point handles. Supports adding, removing, and moving function points
 * interactively. A histogram is drawn in the background if one is set.
 *
 * \sa QmitkTransferFunctionCanvas, QmitkColorTransferFunctionCanvas, QmitkTransferFunctionWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkPiecewiseFunctionCanvas : public QmitkTransferFunctionCanvas
{
  Q_OBJECT

public:
  /**
   * \brief Construct the piecewise function canvas.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkPiecewiseFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Paint the histogram, function lines, handles, title, and range labels. */
  void paintEvent(QPaintEvent *e) override;

  /**
   * \brief Set the title text displayed on the canvas.
   * \param[in] title The title string.
   */
  void SetTitle(const QString &title);

  /**
   * \brief Find a handle near the given canvas position.
   * \param[in] x The x coordinate in canvas pixels.
   * \param[in] y The y coordinate in canvas pixels.
   * \param[in] maxSquaredDistance Maximum squared pixel distance to consider a handle "near".
   * \return Index of the nearest handle, or -1 if none is within range.
   */
  int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 100) override;

  /**
   * \brief Set the vtkPiecewiseFunction to edit.
   * \param[in] piecewiseFunction The piecewise function. Must not be nullptr.
   */
  void SetPiecewiseFunction(vtkPiecewiseFunction *piecewiseFunction)
  {
    this->m_PiecewiseFunction = piecewiseFunction;
    this->SetMin(m_PiecewiseFunction->GetRange()[0]);
    this->SetMax(m_PiecewiseFunction->GetRange()[1]);
    setEnabled(true);

    update();
  }

  /**
   * \brief Add a function point at position (x, val).
   * \param[in] x The scalar position.
   * \param[in] val The opacity value [0, 1].
   * \return Index of the added point.
   */
  int AddFunctionPoint(double x, double val) override { return m_PiecewiseFunction->AddPoint(x, val); }

  /**
   * \brief Remove the function point at position x.
   * \param[in] x The scalar position of the point to remove.
   */
  void RemoveFunctionPoint(double x) override
  {
    int old_size = GetFunctionSize();
    m_PiecewiseFunction->RemovePoint(x);
    if (GetFunctionSize() + 1 != old_size)
    {
      std::cout << "old/new size" << old_size << "/" << GetFunctionSize() << std::endl;
      std::cout << "called with x=" << x << std::endl;
    }
  }

  /**
   * \brief Get the x (scalar) value of a function point by index.
   * \param[in] index The point index.
   * \return The scalar x value.
   */
  double GetFunctionX(int index) override { return m_PiecewiseFunction->GetDataPointer()[index * 2]; }

  /**
   * \brief Get the y (opacity) value of a function point by index.
   * \param[in] index The point index.
   * \return The opacity value.
   */
  double GetFunctionY(int index) override
  {
    return m_PiecewiseFunction->GetValue(m_PiecewiseFunction->GetDataPointer()[index * 2]);
  }

  /**
   * \brief Get the number of points in the piecewise function.
   * \return The point count.
   */
  int GetFunctionSize() override { return m_PiecewiseFunction->GetSize(); }

  /** \brief No-op for piecewise functions (double-click does not change values). */
  void DoubleClickOnHandle(int) override {}

  /**
   * \brief Move a function point to a new position.
   * \param[in] index The index of the point to move.
   * \param[in] pos The new (x, y) position.
   */
  void MoveFunctionPoint(int index, std::pair<double, double> pos) override;

  /**
   * \brief Get the maximum scalar value of the function range.
   * \return The maximum value.
   */
  double GetFunctionMax() { return m_PiecewiseFunction->GetRange()[1]; }

  /**
   * \brief Get the minimum scalar value of the function range.
   * \return The minimum value.
   */
  double GetFunctionMin() { return m_PiecewiseFunction->GetRange()[0]; }

  /**
   * \brief Get the span of the function range.
   * \return The range value (max - min, or max if min is non-negative).
   */
  double GetFunctionRange()
  {
    double range;
    if ((m_PiecewiseFunction->GetRange()[0]) < 0)
    {
      range = (m_PiecewiseFunction->GetRange()[1]) - (m_PiecewiseFunction->GetRange()[0]);
      return range;
    }
    else
    {
      range = m_PiecewiseFunction->GetRange()[1];
      return range;
    }
  }

  /** \brief Reset the function to a default ramp from 0 to 1 with a zero-point at origin. */
  void RemoveAllFunctionPoints()
  {
    m_PiecewiseFunction->AddSegment(this->GetFunctionMin(), 0, this->GetFunctionMax(), 1);
    m_PiecewiseFunction->AddPoint(0.0, 0.0);
  }

  /** \brief Reset the function to a default gradient opacity configuration. */
  void ResetGO()
  { // Gradient Opacity
    m_PiecewiseFunction->AddSegment(this->GetFunctionMin(), 0, 0, 1);
    m_PiecewiseFunction->AddSegment(0, 1, ((this->GetFunctionRange()) * 0.125), 1);
    m_PiecewiseFunction->AddSegment(((this->GetFunctionRange()) * 0.125), 1, ((this->GetFunctionRange()) * 0.2), 1);
    m_PiecewiseFunction->AddSegment(((this->GetFunctionRange()) * 0.2), 1, ((this->GetFunctionRange()) * 0.25), 1);
  }

protected:
  vtkPiecewiseFunction *m_PiecewiseFunction;
  QString m_Title;
};

#endif
