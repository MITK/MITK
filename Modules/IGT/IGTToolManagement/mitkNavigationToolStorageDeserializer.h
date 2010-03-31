/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-11 15:15:02 +0200 (Di, 11 Aug 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef NAVIGATIONTOOLSTORAGEDESERIALIZER_H_INCLUDED
#define NAVIGATIONTOOLSTORAGEDESERIALIZER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include <mitkDataStorage.h>
#include "mitkNavigationToolStorage.h"
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief This class offers methods to load an object of the class NavigationToolStorage
  *        from the harddisc.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolStorageDeserializer : public itk::Object
  {
  public:
    mitkClassMacro(NavigationToolStorageDeserializer,itk::Object);
    mitkNewMacro1Param(Self,mitk::DataStorage::Pointer);

    /**
     * @brief    Loads a collection of navigation tools represented by a mitk::NavigationToolStorage
     *           from a file. 
     * @return   Returns the storage which was loaded or NULL if there was an error in the loading process.
     * 
     */
    mitk::NavigationToolStorage::Pointer Deserialize(std::string filename);
    
    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolStorageDeserializer(mitk::DataStorage::Pointer dataStorage);
    ~NavigationToolStorageDeserializer();

    std::string m_ErrorMessage;

    mitk::DataStorage::Pointer m_DataStorage;

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGEDESERIALIZER