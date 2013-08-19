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


#ifndef IFileWriter_H_HEADER_INCLUDED_C1E7E521
#define IFileWriter_H_HEADER_INCLUDED_C1E7E521

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

namespace mitk {

/**
* \brief The common interface of all FileWriters.
*
* This interface defines the methods necessary for the FileWriterManager
* to interact with its FileWriters. To implement a new FileWriter, it is
* recommended to derive from FileWriterAbstract instead of directly from this Interface,
* as the abstract class already implements most of the methods and also makes sure that your writer
* will be managed by the FileWriterManager.
*/
struct MITK_CORE_EXPORT IFileWriter
{

    virtual ~IFileWriter();

    /**
    * \brief Get the complete file name and path to the file that will be written.
    */
    virtual std::string GetFileName() const = 0;

    /**
    * \brief Set the complete file name and path to the file that will be written.
    */
    virtual void SetFileName(const std::string& aFileName) = 0;

    // File Pattern Functions are currently not used for all writers
    ///**
    //* \brief Get the File Prefix of the file that will be written.
    //*
    //* You should specify either a FileName or FilePrefix. Use FilePrefix if
    //* the data is stored in multiple files.
    //*/
    //virtual std::string GetFilePrefix() const = 0;

    ///**
    //* \brief Set the File Prefix of the file that will be written.
    //*
    //* You should specify either a FileName or FilePrefix. Use FilePrefix if
    //* the data is stored in multiple files.
    //*/
    //virtual void SetFilePrefix(const std::string& aFilePrefix) = 0;

    ///**
    //* \brief Get the specified file pattern for the file(s) to write.
    //*
    //* The sprintf format used to build filename from FilePrefix and number.
    //* You should specify either a FileName or FilePrefix. Use FilePrefix if
    //* the data is stored in multiple files.
    //*/
    //virtual std::string GetFilePattern() const = 0;

    ///**
    //* \brief Set the specified file pattern for the file(s) to write.
    //*
    //* The sprintf format used to build filename from FilePrefix and number.
    //* You should specify either a FileName or FilePrefix. Use FilePrefix if
    //* the data is stored in multiple files.
    //*/
    //virtual void SetFilePattern(const std::string& aFilePattern) = 0;

    virtual void Write(const BaseData* data) = 0;

    virtual void Write(const BaseData* data, const std::string& path ) = 0;

    virtual void Write(const BaseData* data, std::ostream& stream ) = 0;

    /**
    * \brief Returns the priority which defines how 'good' the FileWriter can handle it's file format.
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
    * \brief Returns a human readable description of the file format to be written.
    *
    * This will be used in FileDialogs for example.
    */
    virtual std::string GetDescription() const = 0 ;

    /**
    * \brief returns the file extension that this FileWriter is able to handle.
    *
    * Please return only the characters after the fullstop, e.g "nrrd" is correct
    * while "*.nrrd" and ".nrrd" are incorrect.
    */
    virtual std::string GetExtension() const = 0;

    /**
    * \brief returns the itk classname that this FileWriter is able to handle.
    */
    virtual std::string GetSupportedBasedataType() const = 0;

    /**
    * \brief returns a list of the supported Options
    *
    * Options are strings that are treated as flags when passed to the write method.
    */
    virtual std::list< std::string > GetSupportedOptions() const = 0;

    /**
    * \brief Returns true if this reader can confirm that it can write \c data and false otherwise.
    */
    virtual bool CanWrite(const BaseData* data, const std::string& path) const = 0;

    /**
    * \brief Returns a value between 0 and 1 depending on the progress of the writing process.
    * This method need not necessarily be implemented meaningfully, always returning zero is accepted.
    */
    virtual float GetProgress() const = 0;

    // Microservice properties
    static const std::string PROP_EXTENSION;
    static const std::string PROP_BASEDATA_TYPE;
    static const std::string PROP_DESCRIPTION;
    static const std::string PROP_IS_LEGACY;

protected:

};

} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.IFileWriter")

#endif /* IFileWriter_H_HEADER_INCLUDED_C1E7E521 */
