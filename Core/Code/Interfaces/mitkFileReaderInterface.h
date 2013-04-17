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


#ifndef FileReaderInterface_H_HEADER_INCLUDED_C1E7E521
#define FileReaderInterface_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>
// MITK
//#include <mitkBaseData.h>
// ITK
//#include <itkProcessObject.h>
//#include <itkObjectFactory.h>
// Microservices
#include <usServiceInterface.h>
//#include <usServiceRegistration.h>
//#include <usServiceProperties.h>


namespace mitk {

  class BaseData;
}

namespace itk {

  template<class T> class SmartPointer;
}

namespace mitk {


//##Documentation
//## @brief Interface class of readers that read from files
//## @ingroup Process
  struct MITK_CORE_EXPORT FileReaderInterface
{

  //mitkClassMacro(FileReaderInterface,itk::ProcessObject);
  //itkNewMacro(Self);

    virtual ~FileReaderInterface();

    //##Documentation
    //## @brief Get the specified the file to load.
    //##
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual std::string GetFileName() const = 0;

    //##Documentation
    //## @brief Specify the file to load.
    //##
    //## Either the FileName or FilePrefix plus FilePattern are used to read.
    virtual void SetFileName(const std::string aFileName) = 0;

    //##Documentation
    //## @brief Get the specified file prefix for the file(s) to load.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual std::string GetFilePrefix() const = 0;

    //##Documentation
    //## @brief Specify file prefix for the file(s) to load.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual void SetFilePrefix(const std::string& aFilePrefix) = 0;

    //##Documentation
    //## @brief Get the specified file pattern for the file(s) to load. The
    //## sprintf format used to build filename from FilePrefix and number.
    //##
    //## You should specify either a FileName or FilePrefix. Use FilePrefix if
    //## the data is stored in multiple files.
    virtual std::string GetFilePattern() const = 0;

    /**
    @brief Specified file pattern for the file(s) to load. The sprintf
    format used to build filename from FilePrefix and number.

    You should specify either a FileName or FilePrefix. Use FilePrefix if
    the data is stored in multiple files. */
    virtual void SetFilePattern(const std::string& aFilePattern) = 0;

    /**
    @brief Specifies, whether the file reader also can
    read a file from a memory buffer */
    virtual bool CanReadFromMemory(  ) = 0;

    /**
    @brief Set/Get functions to advise the file reader to
    use a memory array for reading a file*/
    virtual void SetReadFromMemory( bool read ) = 0;

    virtual bool GetReadFromMemory(  ) = 0;

    /**
    @brief To be used along with a call of SetReadFromMemory(true). This sets
    the memory buffer and the size from which the reader will read.*/
    virtual void SetMemoryBuffer(const std::string dataArray, unsigned int size) = 0;

    virtual itk::SmartPointer<BaseData> Read(const std::string& path = 0) = 0;

    virtual itk::SmartPointer<BaseData> Read(const std::istream& stream ) = 0;

    virtual int GetPriority() const = 0 ;

    virtual std::string GetExtension() const = 0;

    virtual std::list< std::string > GetSupportedOptions() const = 0;

    //virtual std::list< std::string > GetOptions() = 0;

    //virtual void SetOptions(std::list< std::string > options ) = 0;

    virtual bool CanRead(const std::string& path) const = 0;

    virtual float GetProgress() const = 0;

    // Microservice Properties
    static const std::string US_EXTENSION;
    static const std::string US_CAN_READ_FROM_MEMORY;

    // Microservice Names for defined Properties
    static const std::string OPTION_READ_AS_BINARY;

protected:
    //FileReaderInterface();

public:

protected:
};

} // namespace mitk



// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::FileReaderInterface, "org.mitk.services.FileReader")

#endif /* FileReaderInterface_H_HEADER_INCLUDED_C1E7E521 */
