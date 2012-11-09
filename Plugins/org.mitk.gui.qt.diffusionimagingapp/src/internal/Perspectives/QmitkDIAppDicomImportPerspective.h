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


#ifndef QmitkDIAppDicomImportPerspective_H_
#define QmitkDIAppDicomImportPerspective_H_

#include <berryIPerspectiveFactory.h>

class QmitkDIAppDicomImportPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkDIAppDicomImportPerspective() {}
  QmitkDIAppDicomImportPerspective(const QmitkDIAppDicomImportPerspective& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkDIAppDicomImportPerspective() {}

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);
};

#endif /* QmitkDIAppDicomImportPerspective_H_ */
