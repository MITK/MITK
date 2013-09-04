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


#ifndef FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521
#define FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521

#include <MitkExports.h>
#include <mitkBaseData.h>

#include <mitkIFileReader.h>

// Microservices
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <usServiceObjects.h>


namespace mitk {

/**
 * @ingroup Process
 *
 * Provides convenient access to mitk::IFileReader instances and reading
 * files into mitk::BaseData types.
 *
 * \note The life-time of all mitk::IFileReader objects returned by an
 * instance of this class ends with the destruction of that instance.
 */
class MITK_CORE_EXPORT FileReaderRegistry
{

public:

  typedef us::ServiceReference<IFileReader> ReaderReference;

  FileReaderRegistry();
  ~FileReaderRegistry();

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
    static std::vector< itk::SmartPointer<BaseData> > Read(const std::string& path, us::ModuleContext* context = us::GetModuleContext());

    static std::vector< mitk::BaseData::Pointer > ReadAll(const std::vector<std::string>& paths, std::vector<std::string>* unreadableFiles = 0,
                                                          us::ModuleContext* context = us::GetModuleContext());

    template <class T>
    static itk::SmartPointer<T>  Read(const std::string& path, us::ModuleContext* context = us::GetModuleContext())
    {
      std::vector<mitk::BaseData::Pointer> basedatas = Read(path, context);
      if (basedatas.empty()) return NULL;
      T* result = dynamic_cast<T*> (basedatas.front().GetPointer());
      return result;
    }

    static ReaderReference GetReaderReference(const std::string& mimeType, us::ModuleContext* context = us::GetModuleContext());

    static std::vector<ReaderReference> GetReaderReferences(const std::string& mimeType, us::ModuleContext* context = us::GetModuleContext());

    mitk::IFileReader* GetReader(const ReaderReference& ref, us::ModuleContext* context = us::GetModuleContext());

    /**
    * Returns a compatible Reader to the given file extension
    **/
    mitk::IFileReader* GetReader(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    mitk::IFileReader* GetReader(const std::string& extension, const mitk::IFileReader::OptionNames& options, us::ModuleContext* context = us::GetModuleContext() );

    std::vector <mitk::IFileReader*> GetReaders(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    std::vector <mitk::IFileReader*> GetReaders(const std::string& extension, const mitk::IFileReader::OptionNames& options, us::ModuleContext* context = us::GetModuleContext() );

    void UngetReader(mitk::IFileReader* reader);
    void UngetReaders(const std::vector<mitk::IFileReader*>& readers);

protected:

    std::vector< us::ServiceReference<IFileReader> > GetReaderList(const std::string& extension, us::ModuleContext* context);

    bool ReaderSupportsOptions(mitk::IFileReader* reader, const mitk::IFileReader::OptionNames& options);

private:

    // purposely not implemented
    FileReaderRegistry(const FileReaderRegistry&);
    FileReaderRegistry& operator=(const FileReaderRegistry&);

    std::map<mitk::IFileReader*, us::ServiceObjects<mitk::IFileReader> > m_ServiceObjects;

};
} // namespace mitk
#endif /* FileReaderRegistry_H_HEADER_INCLUDED_C1E7E521 */
