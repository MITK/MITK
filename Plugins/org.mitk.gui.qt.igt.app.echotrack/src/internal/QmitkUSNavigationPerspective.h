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

#ifndef QMITKUSNAVIGATIONPERSPECTIVE_H
#define QMITKUSNAVIGATIONPERSPECTIVE_H

#include <QObject>
#include <berryIPerspectiveFactory.h>

/**
 * \brief Perspective for the ultrasound navigation process.
 * This perspective displays the IGT tracking toolbox and the ultrasound
 * support view on the left and the us navigation view on the right. The data
 * manager is not shown by default.
 */
class QmitkUSNavigationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:
  QmitkUSNavigationPerspective();

  virtual void CreateInitialLayout (berry::IPageLayout::Pointer layout);
};

#endif // QMITKUSNAVIGATIONPERSPECTIVE_H
