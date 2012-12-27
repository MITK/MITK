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


#ifndef QMITKEDITORPERSPECTIVE_H_
#define QMITKEDITORPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>

class QmitkEditorPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkEditorPerspective() {}
  QmitkEditorPerspective(const QmitkEditorPerspective& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkEditorPerspective() {}

  void CreateInitialLayout(berry::IPageLayout::Pointer /*layout*/);
};

#endif /* QMITKEDITORPERSPECTIVE_H_ */
