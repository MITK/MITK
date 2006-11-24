/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef QMITKFUNCTIONALITYFACTORY_H
#define QMITKFUNCTIONALITYFACTORY_H

#include <mitkDataTree.h>

class QmitkFunctionality;
class QmitkStdMultiWidget;
class QObject;

class QmitkFunctionalityFactory {

public:
  typedef QmitkFunctionality*(*CreateFunctionalityPtr)(QObject*, QmitkStdMultiWidget*, mitk::DataTreeIteratorBase*);
  typedef std::list<CreateFunctionalityPtr> CreateFunctionalityPtrList; 
  static QmitkFunctionalityFactory& GetInstance(); 
  void RegisterCreateFunctionalityCall(CreateFunctionalityPtr p) {
    m_RegisteredFunctionalities.push_back(p); 
  };  
  const CreateFunctionalityPtrList& GetCreateFunctionalityPtrList() const { return m_RegisteredFunctionalities; };
protected: 
  QmitkFunctionalityFactory() {};   
  CreateFunctionalityPtrList m_RegisteredFunctionalities;
};

#endif
