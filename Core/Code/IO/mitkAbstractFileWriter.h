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


#ifndef AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521
#define AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>

// MITK
#include <mitkBaseData.h>
#include <mitkIFileWriter.h>

// ITK
#include <itkObjectFactory.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

//##Documentation
//## @brief Interface class of Writers that read from files
//## @ingroup Process
  class MITK_CORE_EXPORT AbstractFileWriter : public mitk::IFileWriter
{
  public:

    //##Documentation
    //## @brief Get the specified the file to load.
    //##
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual std::string GetFileName() const;

    //##Documentation
    //## @brief Specify the file to load.
    //##
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual void SetFileName(const std::string aFileName);

    //##Documentation
    //## @brief Get the specified file prefix for the file(s) to load.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual std::string GetFilePrefix() const;

    //##Documentation
    //## @brief Specify file prefix for the file(s) to load.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const std::string& aFilePrefix);

    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to load. The
    //## sprintf format used to build filename from FilePrefix and number.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual std::string GetFilePattern() const;

    /**
    @brief Specified file pattern for the file(s) to load. The sprintf
    format used to build filename from FilePrefix and number.

    You should specify either a FileName or FilePrefix. Use FilePrefix if
    the data is stored in multiple files. */
    virtual void SetFilePattern(const std::string& aFilePattern);

    virtual void Write(const itk::SmartPointer<BaseData> data, const std::string& path = 0);

    virtual void Write(const itk::SmartPointer<BaseData> data, const std::istream& stream ) = 0;

    virtual int GetPriority() const;

    virtual std::string GetExtension() const;

    virtual std::string GetDescription() const;

    virtual std::list< std::string > GetSupportedOptions() const;

    virtual bool CanWrite(const std::string& path) const;

    virtual float GetProgress() const;


protected:
    AbstractFileWriter();
    AbstractFileWriter(std::string extension, std::string description);
    virtual ~AbstractFileWriter();

    // Filenames etc..
    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    // Minimal Service Properties: ALWAYS SET THESE IN CONSTRUCTOR OF DERIVED CLASSES!
    std::string m_Extension;
    std::string m_Description;
    int m_Priority;
    std::list< std::string > m_Options; // Options supported by this Writer. Can be left emtpy if no special options are required

    // Registration
    mitk::ServiceRegistration m_Registration;

    virtual void RegisterMicroservice(mitk::ModuleContext* context);

    virtual void UnregisterMicroservice(mitk::ModuleContext* context);

    virtual mitk::ServiceProperties ConstructServiceProperties();

};
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::AbstractFileWriter, "org.mitk.services.FileWriter")

#endif /* AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521 */
