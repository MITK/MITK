/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTMAINWINDOWCONTROL_H_
#define BERRYQTMAINWINDOWCONTROL_H_

#include <QMainWindow>

#include <berryShell.h>
#include <internal/berryQtWidgetController.h>

namespace berry {

class QtMainWindowControl : public QMainWindow
{

  Q_OBJECT

public:

  QtMainWindowControl(Shell* shell, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr);

protected:

  // used for shell listeners
  void changeEvent(QEvent* event) override;
  void closeEvent(QCloseEvent* closeEvent) override;

  // used for control listeners
  void moveEvent(QMoveEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void inFocusEvent(QFocusEvent* event);

private:

  QtWidgetController::Pointer controller;

};

}

#endif /* BERRYQTMAINWINDOWCONTROL_H_ */
