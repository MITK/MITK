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
#include <MitkCoreExports.h>
#include <mitkCommon.h>

// Microservices
#include <usServiceInterface.h>

// MITK
#include <mitkMessage.h>

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

    typedef std::pair<std::string, bool> FileServiceOption;
    typedef std::vector<FileServiceOption> OptionList;
    typedef std::vector<std::string> OptionNames;

    virtual void Write(const BaseData* data, const std::string& path ) = 0;

    virtual void Write(const BaseData* data, std::ostream& stream ) = 0;

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
    virtual OptionList GetOptions() const = 0;

    virtual void SetOptions(const OptionList& options) = 0;

    /**
    * \brief Returns true if this reader can confirm that it can write \c data and false otherwise.
    */
    virtual bool CanWrite(const BaseData* data) const = 0;

    /**
    * \brief Returns a value between 0 and 1 depending on the progress of the writing process.
    * This method need not necessarily be implemented meaningfully, always returning zero is accepted.
    */
    virtual float GetProgress() const = 0;

    // Microservice properties
    static std::string PROP_EXTENSION();
    static std::string PROP_BASEDATA_TYPE();
    static std::string PROP_DESCRIPTION();
    static std::string PROP_IS_LEGACY();

  protected:
  };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::IFileWriter, "org.mitk.IFileWriter")

#endif /* IFileWriter_H_HEADER_INCLUDED_C1E7E521 */
