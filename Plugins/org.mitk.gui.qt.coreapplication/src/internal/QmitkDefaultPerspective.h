/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKDEFAULTPERSPECTIVE_H_
#define QMITKDEFAULTPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>


struct QmitkDefaultPerspective : public QObject, public berry::IPerspectiveFactory
{

  QmitkDefaultPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer /*layout*/) override;

private:

  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)
};

#endif /* QMITKDEFAULTPERSPECTIVE_H_ */
