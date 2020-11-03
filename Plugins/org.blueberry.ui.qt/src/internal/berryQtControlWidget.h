/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTCONTROLWIDGET_H_
#define BERRYQTCONTROLWIDGET_H_

#include <QFrame>

#include "berryQtWidgetController.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

class BERRY_UI_QT QtControlWidget : public QFrame
{

public:

  QtControlWidget(QWidget* parent, Shell* shell, Qt::WindowFlags f = nullptr);

  ~QtControlWidget() override;

  void FireActivateEvent();

protected:

  // used for shell listeners
  void changeEvent(QEvent* event) override;
  void closeEvent(QCloseEvent* closeEvent) override;

  // used for control listeners
  void moveEvent(QMoveEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;


private:

  QtWidgetController::Pointer controller;
};

}

#endif /* BERRYQTCONTROLWIDGET_H_ */
