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

// Microservices
#include <usServiceInterface.h>



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

    virtual itk::SmartPointer<BaseData> Read(const std::string& path = 0) = 0;

    virtual itk::SmartPointer<BaseData> Read(const std::istream& stream ) = 0;

    virtual int GetPriority() const = 0 ;

    virtual std::string GetExtension() const = 0;

    virtual std::list< std::string > GetSupportedOptions() const = 0;

    virtual bool CanRead(const std::string& path) const = 0;

    virtual float GetProgress() const = 0;

    // Microservice Properties
    static const std::string US_EXTENSION;

    // Microservice Names for defined Properties
    static const std::string OPTION_READ_AS_BINARY;

protected:


public:

protected:
};

} // namespace mitk



// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::FileReaderInterface, "org.mitk.services.FileReader")

#endif /* FileReaderInterface_H_HEADER_INCLUDED_C1E7E521 */
