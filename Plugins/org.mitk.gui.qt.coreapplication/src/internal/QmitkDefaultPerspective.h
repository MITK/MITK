/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QMITKDEFAULTPERSPECTIVE_H_
#define QMITKDEFAULTPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>


struct QmitkDefaultPerspective : public QObject, public berry::IPerspectiveFactory
{

  QmitkDefaultPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer /*layout*/);

private:

  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)
};

#endif /* QMITKDEFAULTPERSPECTIVE_H_ */
