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


#ifndef MINIMALPERSPECTIVE_H_
#define MINIMALPERSPECTIVE_H_

// berry includes
#include <berryIPerspectiveFactory.h>

// Qt includes
#include <QObject>

class MinimalPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  MinimalPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

#endif /* MINIMALPERSPECTIVE_H_ */
