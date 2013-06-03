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


#ifndef IFileReader_H_HEADER_INCLUDED_C1E7E521
#define IFileReader_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>
#include <mitkCommon.h>

// Microservices
#include <usServiceInterface.h>

// STL
#include <list>



namespace mitk {

  class BaseData;
}

namespace itk {

  template<class T> class SmartPointer;
}

namespace mitk {

/**
* \brief The common interface of all FileReader.
*
* This interface defines the Methods necessary for the FileReaderManager
* to interact with its FileReaders. To implement a new Filereader, it is
* recommended to derive from FileReaderAbstract instead of from the Interface,
* as the abstract class already implements most of the functions.
*/

  struct MITK_CORE_EXPORT IFileReader
{

    virtual ~IFileReader();


    /**
    * \brief Get the complete file name and path to the file that will be read.
    */
    virtual std::string GetFileName() const = 0;

    /**
    * \brief Set the complete file name and path to the file that will be read.
    */
    virtual void SetFileName(const std::string aFileName) = 0;

    /**
    * \brief Get the File Prefix of the file that will be read.
    *
    * You should specify either a FileName or FilePrefix. Use FilePrefix if
    * the data is stored in multiple files.
    */
    // File Pattern mechanic is currently deactivated until decision has been reached on how to handle patterns
    //virtual std::string GetFilePrefix() const = 0;

    /**
    * \brief Set the File Prefix of the file that will be read.
    *
    * You should specify either a FileName or FilePrefix. Use FilePrefix if
    * the data is stored in multiple files.
    */
    // File Pattern mechanic is currently deactivated until decision has been reached on how to handle patterns
    //virtual void SetFilePrefix(const std::string& aFilePrefix) = 0;

    /**
    * \brief Get the specified file pattern for the file(s) to load.
    *
    * The sprintf format used to build filename from FilePrefix and number.
    * You should specify either a FileName or FilePrefix. Use FilePrefix if
    * the data is stored in multiple files.
    */
    // File Pattern mechanic is currently deactivated until decision has been reached on how to handle patterns
    //virtual std::string GetFilePattern() const = 0;

    /**
    * \brief Set the specified file pattern for the file(s) to load.
    *
    * The sprintf format used to build filename from FilePrefix and number.
    * You should specify either a FileName or FilePrefix. Use FilePrefix if
    * the data is stored in multiple files.
    */
    // File Pattern mechanic is currently deactivated until decision has been reached on how to handle patterns
    //virtual void SetFilePattern(const std::string& aFilePattern) = 0;

    /**
    * \brief Reads the specified file and returns its contents.
    *
    * If you extend this class, it does make sense to more closely specify the
    * result's class.
    */
    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::string& path = 0) = 0;

    /**
    * \brief Reads the specified input stream and returns its contents.
    *
    * If you extend this class, it does make sense to more closely specify the
    * result's class.
    */
    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::istream& stream ) = 0;

    /**
    * \brief Returns the priority which defined how 'good' the FileReader can handle it's file format.
    *
    * Default is zero and should only be chosen differently for a reason.
    * The priority is intended to be used by the MicroserviceFramework to determine
    * which reader to use if several equivalent readers have been found.
    * It may be used to replace a default reader from MITK in your own project.
    * If you want to use your own reader for *.nrrd files instead of the default,
    * just implement it and give it a higher priority than zero.
    */
    virtual int GetPriority() const = 0 ;

    /**
    * \brief returns the file extension that this FileReader is able to handle.
    *
    * Please enter only the characters after the fullstop, e.g "nrrd" is correct
    * while "*.nrrd" and ".nrrd" are incorrect.
    */
    virtual std::string GetExtension() const = 0;

    /**
    * \brief Returns a human readable description of the file format.
    *
    * This will be used in FileDialogs for example.
    */
    virtual std::string GetDescription() const = 0;

    /**
    * \brief returns a list of the supported Options
    *
    * Options are strings that are treated as flags when passed to the read method.
    */
    virtual std::list< std::string > GetSupportedOptions() const = 0;

    virtual bool CanRead(const std::string& path) const = 0;

    virtual float GetProgress() const = 0;

    // Microservice properties
    static const std::string PROP_EXTENSION;
    static const std::string PROP_DESCRIPTION;


    // Microservice names for defined properties
    static const std::string OPTION_READ_AS_BINARY;
    static const std::string OPTION_READ_MULTIPLE_FILES;

protected:


public:

protected:
};

} // namespace mitk



// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.services.FileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
