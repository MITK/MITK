/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
