/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTWORKBENCHPAGETWEAKLET_H_
#define BERRYQTWORKBENCHPAGETWEAKLET_H_

#include <berryWorkbenchPageTweaklet.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

class BERRY_UI_QT QtWorkbenchPageTweaklet : public QObject, public WorkbenchPageTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::WorkbenchPageTweaklet)

public:

  QtWorkbenchPageTweaklet();

  QWidget* CreateClientComposite(QWidget* pageControl) override;
  QWidget* CreatePaneControl(QWidget* parent) override;

  Object::Pointer CreateStatusPart(QWidget* parent, const QString& title, const QString& msg) override;
  IEditorPart::Pointer CreateErrorEditorPart(const QString& partName, const QString& msg) override;

};

}

#endif /* BERRYQTWORKBENCHPAGETWEAKLET_H_ */
