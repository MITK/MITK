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
     * @return   Returns the storage which was loaded or an empty storage if there was an error in the loading process.
     * @throw    mitk::IGTException Throws an Exception if the file cannot be decopressed.
     * @throw    mitk::IGTException Throws an Exception if no tool was found inside the storage.
     */
    mitk::NavigationToolStorage::Pointer Deserialize(std::string filename);

    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolStorageDeserializer(mitk::DataStorage::Pointer dataStorage);
    ~NavigationToolStorageDeserializer();

    std::string m_ErrorMessage;

    mitk::DataStorage::Pointer m_DataStorage;

    std::string m_tempDirectory;

    std::string convertIntToString(int i);

    /**
     * @throws Throws an Exception if particular file cannot be opened for reading
     */
    void decomressFiles(std::string file,std::string path);

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGEDESERIALIZER
