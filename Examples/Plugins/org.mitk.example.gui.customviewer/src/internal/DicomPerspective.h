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


#ifndef DICOMPERSPECTIVE_H_
#define DICOMPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

#include <QObject>


class DicomPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  DicomPerspective();

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

#endif /* DICOMPERSPECTIVE_H_ */
