/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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

  QWidget* CreateClientComposite(QWidget* pageControl);
  QWidget* CreatePaneControl(QWidget* parent);

  Object::Pointer CreateStatusPart(QWidget* parent, const QString& title, const QString& msg);
  IEditorPart::Pointer CreateErrorEditorPart(const QString& partName, const QString& msg);

};

}

#endif /* BERRYQTWORKBENCHPAGETWEAKLET_H_ */
