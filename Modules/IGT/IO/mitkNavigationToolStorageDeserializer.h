/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNavigationToolStorageDeserializer_h
#define mitkNavigationToolStorageDeserializer_h

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
  class MITKIGT_EXPORT NavigationToolStorageDeserializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(NavigationToolStorageDeserializer,itk::Object);
    mitkNewMacro1Param(Self,mitk::DataStorage::Pointer);

    /**
     * @brief    Loads a collection of navigation tools represented by a mitk::NavigationToolStorage
     *           from a file.
     * @return   Returns the storage which was loaded or an empty storage if there was an error in the loading process.
     * @throw    mitk::IGTException Throws an Exception if the file cannot be decompressed.
     * @throw    mitk::IGTException Throws an Exception if no tool was found inside the storage.
     */
    mitk::NavigationToolStorage::Pointer Deserialize(std::string filename);

    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolStorageDeserializer(mitk::DataStorage::Pointer dataStorage);
    ~NavigationToolStorageDeserializer() override;

    std::string m_ErrorMessage;

    mitk::DataStorage::Pointer m_DataStorage;

    std::string m_tempDirectory;

    std::string convertIntToString(int i);

    /**
     * @throws Throws an Exception if particular file cannot be opened for reading
     */
    void decompressFiles(std::string file,std::string path);

  };
} // namespace mitk
#endif
