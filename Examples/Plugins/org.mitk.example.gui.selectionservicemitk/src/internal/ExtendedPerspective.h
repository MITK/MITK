/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef EXTENDEDPERSPECTIVE_H_
#define EXTENDEDPERSPECTIVE_H_

// berry includes
#include <berryIPerspectiveFactory.h>

// Qt includes
#include <QObject>

class ExtendedPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:
  ExtendedPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;
};

#endif /* EXTENDEDPERSPECTIVE_H_ */
