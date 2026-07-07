/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkColorTransferFunctionCanvas_h
#define QmitkColorTransferFunctionCanvas_h

#include <MitkQtWidgetsExtExports.h>
#include <QmitkTransferFunctionCanvas.h>

#include <vtkColorTransferFunction.h>

/**
 * \brief Canvas widget for editing a vtkColorTransferFunction.
 *
 * Renders a color gradient based on the underlying vtkColorTransferFunction
 * and provides interactive handle manipulation. Users can add, move, remove,
 * and double-click handles to change colors via a QColorDialog.
 *
 * \sa QmitkTransferFunctionCanvas, QmitkPiecewiseFunctionCanvas, QmitkTransferFunctionWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkColorTransferFunctionCanvas : public QmitkTransferFunctionCanvas
{
  Q_OBJECT

public:
  /**
   * \brief Construct the color transfer function canvas.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkColorTransferFunctionCanvas(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Paint the color gradient, handles, title, and min/max labels. */
  void paintEvent(QPaintEvent *e) override;

  /**
   * \brief Find a handle near the given canvas position.
   * \param[in] x The x coordinate in canvas pixels.
   * \param[in] y The y coordinate in canvas pixels (unused for color functions).
   * \param[in] maxSquaredDistance Maximum squared pixel distance to consider a handle "near".
   * \return Index of the nearest handle, or -1 if none is within range.
   */
  int GetNearHandle(int x, int y, unsigned int maxSquaredDistance = 100) override;

  /**
   * \brief Set the title text displayed on the canvas.
   * \param[in] title The title string.
   */
  void SetTitle(const QString &title);

  /**
   * \brief Set the vtkColorTransferFunction to edit.
   * \param[in] colorTransferFunction The color transfer function. Must not be nullptr.
   */
  void SetColorTransferFunction(vtkColorTransferFunction *colorTransferFunction)
  {
    this->m_ColorTransferFunction = colorTransferFunction;
    this->SetMin(colorTransferFunction->GetRange()[0]);
    this->SetMax(colorTransferFunction->GetRange()[1]);
    setEnabled(true);
    update();
  }

  /**
   * \brief Add a color function point at position x, preserving the existing color at x.
   * \param[in] x The scalar position for the new point.
   * \param[in] val Unused for color transfer functions.
   * \return Index of the added point.
   */
  int AddFunctionPoint(double x, double val) override
  {
    (void)val;
    return m_ColorTransferFunction->AddRGBPoint(x,
                                                m_ColorTransferFunction->GetRedValue(x),
                                                m_ColorTransferFunction->GetGreenValue(x),
                                                m_ColorTransferFunction->GetBlueValue(x));
  }

  /**
   * \brief Remove the color function point at position x.
   * \param[in] x The scalar position of the point to remove.
   */
  void RemoveFunctionPoint(double x) override
  {
    int old_size = GetFunctionSize();
    m_ColorTransferFunction->RemovePoint(x);
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
  double GetFunctionX(int index) override { return m_ColorTransferFunction->GetDataPointer()[index * 4]; }

  /**
   * \brief Get the number of points in the color transfer function.
   * \return The point count.
   */
  int GetFunctionSize() override { return m_ColorTransferFunction->GetSize(); }

  /**
   * \brief Open a QColorDialog to change the color at the given handle.
   * \param[in] handle The index of the handle to edit.
   */
  void DoubleClickOnHandle(int handle) override;

  /**
   * \brief Move a function point to a new position, preserving its color.
   * \param[in] index The index of the point to move.
   * \param[in] pos The new (x, y) position. Only x is used for color functions.
   */
  void MoveFunctionPoint(int index, std::pair<double, double> pos) override;

  /**
   * \brief Add an RGB color point to the transfer function.
   * \param[in] x The scalar position.
   * \param[in] r Red component [0, 1].
   * \param[in] g Green component [0, 1].
   * \param[in] b Blue component [0, 1].
   */
  void AddRGB(double x, double r, double g, double b);

  /**
   * \brief Get the maximum scalar value of the function range.
   * \return The maximum value.
   */
  double GetFunctionMax() { return m_ColorTransferFunction->GetRange()[1]; }

  /**
   * \brief Get the minimum scalar value of the function range.
   * \return The minimum value.
   */
  double GetFunctionMin() { return m_ColorTransferFunction->GetRange()[0]; }

  /**
   * \brief Get the span of the function range (max - min).
   * \return The range value.
   */
  double GetFunctionRange()
  {
    double range;
    if ((m_ColorTransferFunction->GetRange()[0]) == 0)
    {
      range = m_ColorTransferFunction->GetRange()[1];
      return range;
    }
    else
    {
      range = (m_ColorTransferFunction->GetRange()[1]) - (m_ColorTransferFunction->GetRange()[0]);
      return range;
    }
  }

  /** \brief Reset the function to a default red-to-yellow gradient. */
  void RemoveAllFunctionPoints()
  {
    m_ColorTransferFunction->AddRGBSegment(this->GetFunctionMin(), 1, 0, 0, this->GetFunctionMax(), 1, 1, 0);
  }

  /**
   * \brief Always returns 0.0 since color transfer functions have no y component.
   * \return 0.0
   */
  double GetFunctionY(int /*index*/) override { return 0.0; }
protected:
  vtkColorTransferFunction *m_ColorTransferFunction;
  QString m_Title;
};

#endif
