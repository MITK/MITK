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

#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:
   
  DataStorageReference(DataStorage::Pointer dataStorage, bool isDefault = false);
  
  DataStorage::Pointer GetDataStorage() const;
  
  bool IsDefault() const;
  
  std::string GetLabel() const;
  void SetLabel(const std::string& label);
    
  bool operator==(const IDataStorageReference* o) const;
  
private:
  
  bool m_Default;
  std::string m_Label;
  DataStorage::Pointer m_DataStorage;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
