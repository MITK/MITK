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

// Microservices
#include <usServiceReference.h>
#include <usGetModuleContext.h>

namespace mitk {
  struct IFileReader;
}

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

    static std::list< itk::SmartPointer<BaseData> > Read(const std::string& path);

    static std::list< mitk::BaseData::Pointer > ReadAll(const std::list<std::string> paths, std::list<std::string>* unreadableFiles = 0);

    template <class T>
    static itk::SmartPointer<T>  Read(const std::string& path)
    {
      std::list<mitk::BaseData::Pointer> basedatas = Read(path);
      T* result = dynamic_cast<T*> (basedatas.front().GetPointer());
      return result;
    }

    /**
    * Returns a compatible Reader to the given file extension
    **/
    static mitk::IFileReader* GetReader(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    static mitk::IFileReader* GetReader(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    static std::vector <mitk::IFileReader*> GetReaders(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    static std::vector <mitk::IFileReader*> GetReaders(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    static std::string GetSupportedExtensions(const std::string& extension = "");

protected:
    //FileReaderManager();
    //virtual ~FileReaderManager();

    static std::vector< us::ServiceReference<IFileReader> > GetReaderList(const std::string& extension, us::ModuleContext* context);

    static bool ReaderSupportsOptions(mitk::IFileReader* reader, const std::list<std::string>& options);

};
} // namespace mitk
#endif /* FileReaderManager_H_HEADER_INCLUDED_C1E7E521 */
