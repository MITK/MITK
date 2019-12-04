/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  void CreateInitialLayout (berry::IPageLayout::Pointer layout) override;
};

#endif // QMITKUSNAVIGATIONPERSPECTIVE_H
