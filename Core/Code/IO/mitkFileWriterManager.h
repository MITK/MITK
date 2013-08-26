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

#ifndef FileWriterManager_H_HEADER_INCLUDED_C1E7E521
#define FileWriterManager_H_HEADER_INCLUDED_C1E7E521

#include <MitkExports.h>

// Microservices
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <usServiceObjects.h>

// Temporarily disable warning until PIMPL pattern is implemented here
#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4251)
#endif

namespace mitk {
  class BaseData;
  struct IFileWriter;
}

namespace mitk {
  /**
  * @ingroup Process
  *
  * Provides convenient access to mitk::IFileWriter instances and writing
  * files from mitk::BaseData types.
  *
  * \note The life-time of all mitk::IFileWriter objects returned by an
  * instance of this class ends with the destruction of that instance.
  */
  class MITK_CORE_EXPORT FileWriterManager
  {
  public:

    FileWriterManager();
    ~FileWriterManager();

    void Write(const mitk::BaseData* data, const std::string& path, us::ModuleContext* context = us::GetModuleContext());

    /**
    * Returns a compatible Writer to the given file extension
    */
    mitk::IFileWriter* GetWriter(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    mitk::IFileWriter* GetWriter(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    std::vector <mitk::IFileWriter*> GetWriters(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    std::vector <mitk::IFileWriter*> GetWriters(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    void UngetWriter(mitk::IFileWriter* writer);
    void UngetWriters(const std::vector<mitk::IFileWriter*>& writers);

    std::string GetSupportedExtensions(const std::string& extension = std::string(), us::ModuleContext* context = us::GetModuleContext());

    std::string GetSupportedWriters(const std::string& basedataType, us::ModuleContext* context = us::GetModuleContext());

  protected:

    std::vector< us::ServiceReference<IFileWriter> > GetWriterList(const std::string& extension, us::ModuleContext* context);
    std::vector< us::ServiceReference<IFileWriter> > GetWriterListByBasedataType(const std::string& basedataType, us::ModuleContext* context);

    std::string CreateFileDialogString(const std::vector<us::ServiceReference<IFileWriter> >& refs);

    bool WriterSupportsOptions(mitk::IFileWriter* writer, const std::list<std::string>& options);

  private:

    // purposely not implemented
    FileWriterManager(const FileWriterManager&);
    FileWriterManager& operator=(const FileWriterManager&);

    std::map<mitk::IFileWriter*, us::ServiceObjects<mitk::IFileWriter> > m_ServiceObjects;
  };
} // namespace mitk

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif /* FileWriterManager_H_HEADER_INCLUDED_C1E7E521 */
