/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTransferFunctionCanvas_h
#define QmitkTransferFunctionCanvas_h

#include <MitkQtWidgetsExtExports.h>

#include <mitkRenderingManager.h>
#include <mitkSimpleHistogram.h>

#include <QLineEdit>
#include <QWidget>

/**
 * \brief Abstract base class for transfer function editing canvases.
 *
 * Provides common infrastructure for interactive editing of transfer function
 * control points: mouse-based adding, removing, and dragging of handles,
 * histogram background rendering, coordinate conversion, keyboard navigation,
 * and optional QLineEdit binding for precise coordinate input.
 *
 * Subclasses must implement the pure virtual methods that define the specific
 * transfer function type (color vs. piecewise).
 *
 * \sa QmitkColorTransferFunctionCanvas, QmitkPiecewiseFunctionCanvas, QmitkTransferFunctionWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkTransferFunctionCanvas : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Construct the transfer function canvas.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /**
   * \brief Get the associated histogram.
   * \return Pointer to the histogram, or nullptr.
   */
  mitk::SimpleHistogram *GetHistogram() { return m_Histogram; }

  /**
   * \brief Set the histogram to display as background.
   * \param[in] histogram The histogram. May be nullptr.
   */
  void SetHistogram(mitk::SimpleHistogram *histogram) { m_Histogram = histogram; }

  /**
   * \brief Get the minimum scalar value of the data range.
   * \return The minimum value.
   */
  double GetMin() { return m_Min; }

  /**
   * \brief Set the minimum scalar value and update the lower display bound.
   * \param[in] min The minimum value.
   */
  void SetMin(double min)
  {
    this->m_Min = min;
    SetLower(min);
  }

  /**
   * \brief Get the maximum scalar value of the data range.
   * \return The maximum value.
   */
  double GetMax() { return m_Max; }

  /**
   * \brief Set the maximum scalar value and update the upper display bound.
   * \param[in] max The maximum value.
   */
  void SetMax(double max)
  {
    this->m_Max = max;
    SetUpper(max);
  }

  /**
   * \brief Get the lower display bound.
   * \return The lower bound.
   */
  double GetLower() { return m_Lower; }

  /**
   * \brief Set the lower display bound (visible range start).
   * \param[in] lower The lower bound.
   */
  void SetLower(double lower) { this->m_Lower = lower; }

  /**
   * \brief Get the upper display bound.
   * \return The upper bound.
   */
  double GetUpper() { return m_Upper; }

  /**
   * \brief Set the upper display bound (visible range end).
   * \param[in] upper The upper bound.
   */
  void SetUpper(double upper) { this->m_Upper = upper; }

  /** \brief Handle mouse press: add or remove function points, select handles. */
  void mousePressEvent(QMouseEvent *mouseEvent) override;

  /** \brief Default paint event handler. */
  void paintEvent(QPaintEvent *e) override;

  /**
   * \brief Called when a handle is double-clicked (e.g. to open a color dialog).
   * \param[in] handle The index of the double-clicked handle.
   */
  virtual void DoubleClickOnHandle(int handle) = 0;

  /** \brief Handle mouse move: drag the grabbed handle. */
  void mouseMoveEvent(QMouseEvent *mouseEvent) override;

  /** \brief Handle mouse release: trigger repaint. */
  void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

  /** \brief Handle double-click on a handle. */
  void mouseDoubleClickEvent(QMouseEvent *mouseEvent) override;

  /**
   * \brief Paint the histogram as a background overlay.
   * \param[in,out] p The QPainter to draw with.
   */
  void PaintHistogram(QPainter &p);

  /**
   * \brief Find a handle near the given canvas position.
   * \param[in] x The x coordinate in canvas pixels.
   * \param[in] y The y coordinate in canvas pixels.
   * \param[in] maxSquaredDistance Maximum squared pixel distance.
   * \return Index of the nearest handle, or -1.
   */
  virtual int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 100) = 0;

  /**
   * \brief Add a function point at (x, val).
   * \param[in] x The scalar position.
   * \param[in] val The function value.
   * \return The index of the added point.
   */
  virtual int AddFunctionPoint(double x, double val) = 0;

  /**
   * \brief Remove the function point at position x.
   * \param[in] x The scalar position to remove.
   */
  virtual void RemoveFunctionPoint(double x) = 0;

  /**
   * \brief Move a function point to a new position.
   * \param[in] index The index of the point.
   * \param[in] pos The new (x, y) position.
   */
  virtual void MoveFunctionPoint(int index, std::pair<double, double> pos) = 0;

  /**
   * \brief Get the x (scalar) value of a function point.
   * \param[in] index The point index.
   * \return The x value.
   */
  virtual double GetFunctionX(int index) = 0;

  /**
   * \brief Get the y (function) value of a function point.
   * \param[in] index The point index.
   * \return The y value.
   */
  virtual double GetFunctionY(int index) = 0;

  /**
   * \brief Get the number of function points.
   * \return The point count.
   */
  virtual int GetFunctionSize() = 0;

  /** \brief Index of the currently grabbed (selected) handle, or -1. */
  int m_GrabbedHandle;

  double m_Lower; ///< Lower display bound.
  double m_Upper; ///< Upper display bound.
  double m_Min;   ///< Minimum data range value.
  double m_Max;   ///< Maximum data range value.

  /**
   * \brief Convert function coordinates to canvas pixel coordinates.
   * \param[in] functionPoint The (x, y) function coordinates.
   * \return The (x, y) canvas pixel coordinates.
   */
  std::pair<int, int> FunctionToCanvas(std::pair<double, double> functionPoint);

  /**
   * \brief Convert canvas pixel coordinates to function coordinates.
   * \param[in] canvasPoint The (x, y) canvas pixel coordinates.
   * \return The (x, y) function coordinates.
   */
  std::pair<double, double> CanvasToFunction(std::pair<int, int> canvasPoint);

  mitk::SimpleHistogram *m_Histogram; ///< The background histogram.

  /** \brief Handle keyboard input for point manipulation (Delete, arrow keys). */
  void keyPressEvent(QKeyEvent *e) override;

  /**
   * \brief Enable or disable immediate rendering updates during interaction.
   * \param[in] state True to enable immediate updates.
   */
  void SetImmediateUpdate(bool state);

  /**
   * \brief Clamp coordinates to valid histogram range and [0, 1] for y.
   * \param[in] x The (x, y) coordinates to validate.
   * \return The clamped coordinates.
   */
  std::pair<double, double> ValidateCoord(std::pair<double, double> x)
  {
    double max = m_Histogram->GetMax();
    double min = m_Histogram->GetMin();
    if (x.first < min)
      x.first = min;
    if (x.first > max)
      x.first = max;
    if (x.second < 0)
      x.second = 0;
    if (x.second > 1)
      x.second = 1;
    return x;
  }

  /**
   * \brief Set the x coordinate of the currently grabbed handle.
   * \param[in] x The new x value.
   */
  void SetX(float x)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(x, GetFunctionY(m_GrabbedHandle))));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  /**
   * \brief Set the y coordinate of the currently grabbed handle.
   * \param[in] y The new y value.
   */
  void SetY(float y)
  {
    if (m_GrabbedHandle != -1)
    {
      this->MoveFunctionPoint(m_GrabbedHandle, ValidateCoord(std::make_pair(GetFunctionX(m_GrabbedHandle), y)));
      update();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }

  /**
   * \brief Connect QLineEdit widgets for displaying/editing the selected handle coordinates.
   * \param[in] xEdit The line edit for the x coordinate.
   * \param[in] yEdit The line edit for the y coordinate. May be nullptr.
   */
  void SetQLineEdits(QLineEdit *xEdit, QLineEdit *yEdit)
  {
    m_XEdit = xEdit;
    m_YEdit = yEdit;
    m_LineEditAvailable = true;
  }

protected:
  bool m_ImmediateUpdate;
  float m_Range;

  bool m_LineEditAvailable;
  QLineEdit *m_XEdit;
  QLineEdit *m_YEdit;
};

#endif
