/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrossWidget_h
#define QmitkCrossWidget_h

#include <MitkQtWidgetsExtExports.h>
#include <QLabel>

#include <mitkCommon.h>

/**
 * \brief A crosshair label widget that reports mouse drag deltas.
 *
 * Captures mouse press, move, and release events and emits the
 * relative pixel movement (delta) as a signal. During mouse drag,
 * the cursor position is reset to the press location (except on macOS)
 * so the widget acts as an infinite drag surface.
 *
 * \sa QmitkTransferFunctionGeneratorWidget
 */
class MITKQTWIDGETSEXT_EXPORT QmitkCrossWidget : public QLabel
{
  Q_OBJECT

public:
  /**
   * \brief Construct the cross widget.
   * \param[in] parent The parent widget.
   * \param[in] f Window flags.
   */
  QmitkCrossWidget(QWidget *parent = nullptr, Qt::WindowFlags f = {});

  /** \brief Handle mouse press: record initial position and emit zero delta. */
  void mousePressEvent(QMouseEvent *mouseEvent) override;

  /** \brief Handle mouse move: compute delta from last position and emit it. */
  void mouseMoveEvent(QMouseEvent *mouseEvent) override;

  /** \brief Handle mouse release (no-op). */
  void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

signals:
  /**
   * \brief Emitted during mouse drag with the delta movement.
   * \param[in] deltaX Horizontal pixel delta.
   * \param[in] deltaY Vertical pixel delta.
   */
  void SignalDeltaMove(int deltaX, int deltaY);

protected:
// fix for bug 3378 - setPos() causes an app crash on macOS
#ifdef __APPLE__
  void ResetMousePosition(int, int){};
#else
  void ResetMousePosition(int xpos, int ypos) { QCursor::setPos(xpos, ypos); };
#endif

  int lastX, lastY;
};

#endif
