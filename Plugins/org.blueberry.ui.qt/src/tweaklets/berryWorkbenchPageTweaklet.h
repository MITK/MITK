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


#ifndef BERRYWORKBENCHPAGETWEAKLET_H_
#define BERRYWORKBENCHPAGETWEAKLET_H_

#include "internal/berryTweaklets.h"
#include "berryIWorkbenchPage.h"

namespace berry
{

struct BERRY_UI_QT WorkbenchPageTweaklet
{

  static Tweaklets::TweakKey<WorkbenchPageTweaklet> KEY;

  virtual QWidget* CreateClientComposite(QWidget* pageControl) = 0;
  virtual QWidget* CreatePaneControl(QWidget* parent) = 0;

  virtual Object::Pointer CreateStatusPart(QWidget* parent, const QString& title, const QString& msg) = 0;
  virtual IEditorPart::Pointer CreateErrorEditorPart(const QString& partName, const QString& msg) = 0;

};

}

Q_DECLARE_INTERFACE(berry::WorkbenchPageTweaklet, "org.blueberry.WorkbenchPageTweaklet")

#endif /* BERRYWORKBENCHPAGETWEAKLET_H_ */
