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


#ifndef LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521
#define LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>

// MITK
#include <mitkBaseData.h>
#include <mitkAbstractFileWriter.h>
#include <mitkFileWriter.h>

// ITK
#include <itkObjectFactory.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

//##Documentation
//## @brief
//## @ingroup Process
  class MITK_CORE_EXPORT LegacyFileWriterService : public itk::LightObject, public mitk::AbstractFileWriter
{
  public:

    mitkClassMacro( LegacyFileWriterService, AbstractFileWriter)

    mitkNewMacro4Param( Self, mitk::FileWriter::Pointer, const std::string&, const std::string&, const std::string&)

    using AbstractFileWriter::Write;

    virtual void Write(const BaseData* data, const std::string& path);

    virtual void Write(const BaseData* data, std::ostream& stream );

    virtual bool CanWrite(const BaseData* data, const std::string& path) const;


protected:
    LegacyFileWriterService();
    LegacyFileWriterService(mitk::FileWriter::Pointer legacyWriter, const std::string& basedataType,
                            const std::string& extension, const std::string& description);
    virtual ~LegacyFileWriterService();

    mitk::FileWriter::Pointer m_LegacyWriter;

};
} // namespace mitk


#endif /* LegacyFileWriterService_H_HEADER_INCLUDED_C1E7E521 */
