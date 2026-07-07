/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkScalarBar_h
#define QmitkScalarBar_h

#include <MitkQtOverlaysExports.h>
#include <mitkCommon.h>

#include <QPen>
#include <QWidget>

/**
 * \class QmitkScalarBar
 * \brief A widget that draws a scale bar with subdivision lines for spatial reference.
 *
 * The scalar bar can be oriented either vertically or horizontally. It automatically
 * adapts its number of subdivisions and size based on the parent widget dimensions and
 * the given scale factor (mm-per-pixel). The bar is drawn using a configurable QPen.
 *
 * \sa QmitkScalarBarOverlay
 */
class MITKQTOVERLAYS_EXPORT QmitkScalarBar : public QWidget
{
  Q_OBJECT
public:
  /**
   * \brief Orientation of the scalar bar.
   */
  enum alignment
  {
    vertical = 0,   ///< Vertical orientation (default)
    horizontal = 1  ///< Horizontal orientation
  };

  /**
   * \brief Constructor.
   * \param[in] parent Optional parent widget.
   */
  QmitkScalarBar(QWidget *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkScalarBar() override;

  /**
   * \brief Sets the scale factor (mm per pixel) and resizes the bar accordingly.
   *
   * The method dynamically adjusts the number of subdivisions and the widget size
   * to fit within a reasonable proportion of the parent widget.
   *
   * \param[in] scale The scale factor in mm per pixel.
   */
  virtual void SetScaleFactor(double scale);

  /**
   * \brief Sets the orientation of the scalar bar.
   * \param[in] align The desired alignment (vertical or horizontal).
   */
  virtual void SetAlignment(alignment align);

  /**
   * \brief Sets the pen used to draw the scalar bar lines.
   * \param[in] pen The QPen to use for rendering.
   */
  void SetPen(const QPen &pen);

  /**
   * \brief Sets the number of subdivision tick marks.
   * \param[in] subs The number of subdivisions.
   */
  void SetNumberOfSubdivisions(unsigned int subs);

  /**
   * \brief Returns the current number of subdivision tick marks.
   * \return The number of subdivisions.
   */
  unsigned int GetNumberOfSubdivisions();

protected:
  void paintEvent(QPaintEvent *event) override;

  void SetupGeometry(alignment align);

  void CleanUpLines();
  // void moveEvent(QMoveEvent*);

  alignment m_Alignment;

  double m_ScaleFactor;

  QLine *m_MainLine;

  std::vector<QLine *> m_SubDivisionLines;

  QPen m_Pen;

  unsigned int m_NumberOfSubDivisions;
};

#endif
