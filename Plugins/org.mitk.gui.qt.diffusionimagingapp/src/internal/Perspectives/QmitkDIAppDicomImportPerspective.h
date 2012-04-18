/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date: 2009-10-23 02:59:36 +0200 (Fr, 23 Okt 2009) $
 Version:   $Revision: 19652 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


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
