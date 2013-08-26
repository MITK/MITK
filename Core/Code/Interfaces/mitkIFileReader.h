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
  class DataStorage;
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
  * as the abstract class already implements most of the functions and also makes sure
  * that your reader will be managed by the FileReaderManager.
  */
  struct MITK_CORE_EXPORT IFileReader
  {
    virtual ~IFileReader();

    typedef std::pair<std::string, bool> FileServiceOption;

    /**
    * \brief Reads the specified file and returns its contents.
    */
    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::string& path, mitk::DataStorage* ds = 0) = 0;

    /**
    * \brief Reads the specified input stream and returns its contents.
    */
    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::istream& stream, mitk::DataStorage* ds  = 0) = 0;

    /**
    * \brief Returns the priority which defined how 'good' the FileReader can handle it's file format.
    *
    * Default is zero and should only be chosen differently for a reason.
    * The priority is intended to be used by the MicroserviceFramework to determine
    * which reader to use if several equivalent readers have been found.
    * It may be used to replace a default reader from MITK in your own project.
    * E.g. if you want to use your own reader for *.nrrd files instead of the default,
    * implement it and give it a higher priority than zero.
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
    * Inititally, the reader contains a set of standard options. These can be retrieved,
    * manipulated and set again. To activate or deactivate an option, just set it's bool
    * value accordingly. All supported options are in the default set, it does not
    * make sense to add strings artificially after retrieving a reader - the specific
    * implementation decides which options it wants to support.
    * If no options are supported, an empty list is returned.
    */
    virtual std::list< mitk::IFileReader::FileServiceOption > GetOptions() const = 0;

    /**
    * \brief Sets the options for this reader
    *
    * The best way to use this method is to retireve the options via GetOptions, manipulate the bool values,
    * and then set the options again.
    */
    virtual void SetOptions(std::list< mitk::IFileReader::FileServiceOption > options) = 0;

    /**
    * \brief Returns true if this writer can confirm that it can read this file and false otherwise.
    */
    virtual bool CanRead(const std::string& path) const = 0;

    /**
    * \brief Returns a value between 0 and 1 depending on the progress of the read.
    * This method need not necessarily be implemented meaningfully, always returning zero is accepted.
    */
    virtual float GetProgress() const = 0;

    // Microservice properties
    static const std::string PROP_EXTENSION;
    static const std::string PROP_DESCRIPTION;
    static const std::string PROP_IS_LEGACY;

    // Microservice names for defined properties
    static const std::string OPTION_READ_AS_BINARY;
    static const std::string OPTION_READ_MULTIPLE_FILES;

  protected:
  };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileReader, "org.mitk.IFileReader")

#endif /* IFileReader_H_HEADER_INCLUDED_C1E7E521 */
