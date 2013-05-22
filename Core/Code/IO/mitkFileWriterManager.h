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
#include <mitkBaseData.h>
#include <mitkIFileWriter.h>

// Microservices
#include <mitkServiceReference.h>
#include <mitkGetModuleContext.h>


namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class MITK_CORE_EXPORT FileWriterManager
{
  public:

    /**
    * Writes the file located at <code>path</code> and returns the
    * contents as a DataNode.
    *
    * This method will select the best available Writer in the service
    * registry for the task.
    *
    * UnsupportedFileException: If no compatible Writer was found
    * FileNotFoundException: If no file was found at <code>path</code>
    * FileWriteException: If the selected Writer failed to Write the file
    **/

    static void Write(mitk::BaseData::Pointer data, const std::string& path);

    template <class T>
    static T* Write(const std::string& path)
    {
      mitk::BaseData::Pointer basedata = Write(path);
      T* result = dynamic_cast<T*> (basedata.GetPointer());
      return result;
    }

    /**
    * Returns a compatible Writer to the given file extension
    **/
    static mitk::IFileWriter* GetWriter(const std::string& extension, mitk::ModuleContext* context = GetModuleContext() );

    static mitk::IFileWriter* GetWriter(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context = GetModuleContext() );

    static std::list <mitk::IFileWriter*> GetWriters(const std::string& extension, mitk::ModuleContext* context = GetModuleContext() );

    static std::list <mitk::IFileWriter*> GetWriters(const std::string& extension, const std::list<std::string>& options, mitk::ModuleContext* context = GetModuleContext() );

protected:
    //FileWriterManager();
    //virtual ~FileWriterManager();

    static std::list< mitk::ServiceReference > GetWriterList(const std::string& extension, mitk::ModuleContext* context);

    static bool WriterSupportsOptions(mitk::IFileWriter* Writer, std::list<std::string> options);

};
} // namespace mitk
#endif /* FileWriterManager_H_HEADER_INCLUDED_C1E7E521 */
