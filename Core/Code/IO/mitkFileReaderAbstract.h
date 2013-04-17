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


#ifndef FileReaderAbstract_H_HEADER_INCLUDED_C1E7E521
#define FileReaderAbstract_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>
// MITK
#include <mitkBaseData.h>
#include <mitkFileReaderInterface.h>
// ITK
#include <itkProcessObject.h>
#include <itkObjectFactory.h>
#include <itkLightObject.h>
// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>
#include <usModuleContext.h>



namespace mitk {

//##Documentation
//## @brief Interface class of readers that read from files
//## @ingroup Process
  class MITK_CORE_EXPORT FileReaderAbstract : public mitk::FileReaderInterface, public itk::LightObject
{
  public:

    mitkClassMacro(FileReaderAbstract, itk::LightObject);

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

    /**
    @brief Specifies, whether the file reader also can
    read a file from a memory buffer */
    virtual bool CanReadFromMemory(  );

    /**
    @brief Set/Get functions to advise the file reader to
    use a memory array for reading a file*/
    virtual void SetReadFromMemory( bool read );
    virtual bool GetReadFromMemory(  );

    /**
    @brief To be used along with a call of SetReadFromMemory(true). This sets
    the memory buffer and the size from which the reader will read.*/
    virtual void SetMemoryBuffer(const std::string dataArray, unsigned int size);

    virtual itk::SmartPointer<BaseData> Read(const std::string& path = 0) = 0;

    virtual itk::SmartPointer<BaseData> Read(const std::istream& stream ) = 0;

    virtual int GetPriority() const;

    virtual std::string GetExtension() const;

    virtual std::list< std::string > GetSupportedOptions() const;

    // virtual std::list< std::string > GetOptions() = 0;

    // virtual void SetOptions(std::list< std::string > options ) = 0;

    virtual bool CanRead(const std::string& path) const;

    virtual float GetProgress() const;


protected:
    FileReaderAbstract();
    virtual ~FileReaderAbstract();

    // Filenames etc..
    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;

    // Reading from Memory
    bool m_CanReadFromMemory;
    bool m_ReadFromMemory;
    std::string m_MemoryBuffer;
    unsigned int   m_MemorySize;

    // Minimal Service Properties: ALWAYS SET THESE IN CONSTRUCTOR OF DERIVED CLASSES!
    std::string m_Extension;
    int m_Priority;
    std::list< std::string > m_Options; // Options supported by this reader. Can be left emtpy if no special options are required

    // Registration
    mitk::ServiceRegistration m_Registration;

    virtual void RegisterMicroservice(mitk::ModuleContext* context);

    virtual mitk::ServiceProperties ConstructServiceProperties();

};
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::FileReaderAbstract, "org.mitk.services.FileReader")

#endif /* FileReaderAbstract_H_HEADER_INCLUDED_C1E7E521 */
