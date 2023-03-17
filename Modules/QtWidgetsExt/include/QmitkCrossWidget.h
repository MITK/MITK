/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrossWidget_h
#define QmitkCrossWidget_h

#include "MitkQtWidgetsExtExports.h"
#include <QLabel>

#include <mitkCommon.h>

class MITKQTWIDGETSEXT_EXPORT QmitkCrossWidget : public QLabel
{
  Q_OBJECT

public:
  QmitkCrossWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);

  void mousePressEvent(QMouseEvent *mouseEvent) override;
  void mouseMoveEvent(QMouseEvent *mouseEvent) override;
  void mouseReleaseEvent(QMouseEvent *mouseEvent) override;

signals:

  void SignalDeltaMove(int, int);

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
