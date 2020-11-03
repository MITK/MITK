/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYHELPPERSPECTIVE_H_
#define BERRYHELPPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>

namespace berry {

class HelpPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  static const QString ID;

  HelpPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

}

#endif /* BERRYHELPPERSPECTIVE_H_ */
