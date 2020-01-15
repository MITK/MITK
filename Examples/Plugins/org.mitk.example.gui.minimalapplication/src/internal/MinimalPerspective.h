/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MINIMALPERSPECTIVE_H_
#define MINIMALPERSPECTIVE_H_

// Berry
#include <berryIPerspectiveFactory.h>

// Qt
#include <QObject>

class MinimalPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:
  MinimalPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;
};

#endif /* MINIMALPERSPECTIVE_H_ */
