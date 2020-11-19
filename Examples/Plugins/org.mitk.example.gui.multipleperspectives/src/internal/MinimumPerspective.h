/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MINIMUMPERSPECTIVE_H_
#define MINIMUMPERSPECTIVE_H_

// berry includes
#include <berryIPerspectiveFactory.h>

// Qt includes
#include <QObject>

class MinimumPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:
  MinimumPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;
};

#endif /* MINIMUMPERSPECTIVE_H_ */
