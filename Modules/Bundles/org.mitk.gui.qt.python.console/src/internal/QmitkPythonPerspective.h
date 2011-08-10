/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKPYTHONPERSPECTIVE_H_
#define QMITKPYTHONPERSPECTIVE_H_

#include <berryIPerspectiveFactory.h>
#include <QObject>

struct QmitkPythonPerspective : public QObject, public berry::IPerspectiveFactory
{
 
  Q_OBJECT

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);
  
  QmitkPythonPerspective(const QmitkPythonPerspective& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }

};

#endif /* QMITKEXTDEFAULTPERSPECTIVE_H_ */
