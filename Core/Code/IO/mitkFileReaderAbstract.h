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
// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>


namespace mitk {

//##Documentation
//## @brief Interface class of readers that read from files
//## @ingroup Process
  class MITK_CORE_EXPORT FileReaderAbstract : public mitk::FileReaderInterface
{
  public:

    //##Documentation
    //## @brief Get the specified the file to load.
    //##
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual const std::string GetFileName() const;

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
    virtual const std::string GetFilePrefix() const;

    //##Documentation
    //## @brief Specify file prefix for the file(s) to load.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const std::string aFilePrefix);

    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to load. The
    //## sprintf format used to build filename from FilePrefix and number.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual const std::string GetFilePattern() const;

    /**
    @brief Specified file pattern for the file(s) to load. The sprintf
    format used to build filename from FilePrefix and number.

    You should specify either a FileName or FilePrefix. Use FilePrefix if
    the data is stored in multiple files. */
    virtual void SetFilePattern(const std::string aFilePattern);

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

    virtual mitk::BaseData::Pointer Read(std::string path = 0) = 0;

    virtual mitk::BaseData::Pointer Read(std::istream*) = 0;

    int GetPriority();

    std::string GetExtension();

    virtual std::list< std::string > GetSupportedOptions() = 0;

    virtual std::list< std::string > GetOptions() = 0;

    virtual void SetOptions(std::list< std::string > options ) = 0;

    virtual bool CanRead(const std::string& path) = 0; // Todo: nach möglichkeit constref

    virtual float GetProgress() = 0;


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

public:

protected:
};
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::FileReaderAbstract, "org.mitk.services.FileReader")

#endif /* FileReaderAbstract_H_HEADER_INCLUDED_C1E7E521 */
