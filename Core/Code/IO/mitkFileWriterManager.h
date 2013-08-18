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
#include <usServiceReference.h>
#include <usGetModuleContext.h>


namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
class MITK_CORE_EXPORT FileWriterManager
{
  public:

    static void Write(const mitk::BaseData::Pointer data, const std::string& path, us::ModuleContext* context = us::GetModuleContext());

    /**
    * Returns a compatible Writer to the given file extension
    **/
    static mitk::IFileWriter* GetWriter(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    static mitk::IFileWriter* GetWriter(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    static std::vector <mitk::IFileWriter*> GetWriters(const std::string& extension, us::ModuleContext* context = us::GetModuleContext() );

    static std::vector <mitk::IFileWriter*> GetWriters(const std::string& extension, const std::list<std::string>& options, us::ModuleContext* context = us::GetModuleContext() );

    static std::string GetSupportedExtensions(const std::string& extension = 0);

    static std::string GetSupportedWriters(const std::string& basedataType);

protected:
    //FileWriterManager();
    //virtual ~FileWriterManager();

    static std::vector< us::ServiceReference<IFileWriter> > GetWriterList(const std::string& extension, us::ModuleContext* context);
    static std::vector< us::ServiceReference<IFileWriter> > GetWriterListByBasedataType(const std::string& basedataType, us::ModuleContext* context);

    static std::string CreateFileDialogString(const std::vector<us::ServiceReference<IFileWriter> >& refs);

    static bool WriterSupportsOptions(mitk::IFileWriter* Writer, std::list<std::string> options);

    //static std::list< mitk::LegacyFileWriterService::Pointer > m_LegacyWriters;

};
} // namespace mitk
#endif /* FileWriterManager_H_HEADER_INCLUDED_C1E7E521 */
