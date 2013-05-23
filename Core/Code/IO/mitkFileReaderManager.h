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


#ifndef FileReaderManager_H_HEADER_INCLUDED_C1E7E521
#define FileReaderManager_H_HEADER_INCLUDED_C1E7E521

#include <MitkExports.h>
#include <mitkBaseData.h>
#include <mitkIFileReader.h>

// Microservices
#include <mitkServiceReference.h>
#include <mitkGetModuleContext.h>


namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class MITK_CORE_EXPORT FileReaderManager
{
  public:

    /**
    * Reads the file located at <code>path</code> and returns the
    * contents as a DataNode.
    *
    * This method will select the best available reader in the service
    * registry for the task.
    *
    * UnsupportedFileException: If no compatible reader was found
    * FileNotFoundException: If no file was found at <code>path</code>
    * FileReadException: If the selected reader failed to read the file
    **/

    static mitk::BaseData::Pointer Read(const std::string& path);

    template <class T>
    static T* Read(const std::string& path)
    {
      mitk::BaseData::Pointer basedata = Read(path);
      T* result = dynamic_cast<T*> (basedata.GetPointer());
      return result;
    }

    /**
    * Returns a compatible Reader to the given file extension
    **/
    static mitk::IFileReader* GetReader(const std::string& extension, mitk::ModuleContext* context = GetModuleContext() );

    static mitk::IFileReader* GetReader(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context = GetModuleContext() );

    static std::list <mitk::IFileReader*> GetReaders(const std::string& extension, mitk::ModuleContext* context = GetModuleContext() );

    static std::list <mitk::IFileReader*> GetReaders(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context = GetModuleContext() );

    static std::string GetSupportedExtensions();

protected:
    //FileReaderManager();
    //virtual ~FileReaderManager();

    static std::list< mitk::ServiceReference > GetReaderList(const std::string& extension, mitk::ModuleContext* context);

    static bool ReaderSupportsOptions(mitk::IFileReader* reader, std::list<std::string> options);

};
} // namespace mitk
#endif /* FileReaderManager_H_HEADER_INCLUDED_C1E7E521 */
