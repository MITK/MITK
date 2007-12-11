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

#ifndef QMITKFUNCTIONALITYFACTORY_H
#define QMITKFUNCTIONALITYFACTORY_H

#include <mitkDataTree.h>

class QmitkFunctionality;
class QmitkStdMultiWidget;
class QObject;

class QmitkFunctionalityFactory {

public:
  typedef QmitkFunctionality*(*CreateFunctionalityPtr)(QObject*, QmitkStdMultiWidget*, mitk::DataTreeIteratorBase*);
  typedef std::map<std::string,CreateFunctionalityPtr> CreateFunctionalityPtrMap; 
  static QmitkFunctionalityFactory& GetInstance(); 
  void RegisterCreateFunctionalityCall(const std::string name,CreateFunctionalityPtr p) {
    m_RegisteredFunctionalities.insert(std::make_pair(name,p)); 
  };  
  const CreateFunctionalityPtrMap& GetCreateFunctionalityPtrMap() const { return m_RegisteredFunctionalities; };
  CreateFunctionalityPtr GetCreateFunctionalityPtrByName(const std::string name) const;    
protected: 
  QmitkFunctionalityFactory() {};   
  CreateFunctionalityPtrMap m_RegisteredFunctionalities;
};

#endif
