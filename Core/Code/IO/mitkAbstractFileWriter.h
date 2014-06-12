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
#include <MitkCoreExports.h>

// MITK
#include <mitkIFileWriter.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {
  /**
  * @brief This abstract class gives a meaningful default implementations to most methods of mitkIFileWriter.h.
  *
  * In general, all FileWriters should derive from this class, this way it is made sure that the new implementation is
  * exposed to the Microservice-Framework and that is automatically available troughout MITK.
  * The default implementation only requires the two write
  * methods and the Clone() method to be implemented. Be sure to set all important members in the constructor. These are:
  * <ul>
  *  <li> m_Extension: To define which file extension can be written.
  *  <li> m_Description: To give a human readable name for this writer to be displayed in FileDialogs.
  *  <li> m_BasedataType: To define which type of Basedata this fileWriter can handle.
  *  <li> (Optional) m_Priority: To make this writer rank higher when choosing writers automatically
  *  <li> (Optional) m_SupportedOptions: To define which options this writer can handle. Options can modify writing behaviour (e.g. set a compression)
  * </ul>
  * You can also use the protected constructor for this.
  *
  * @ingroup Process
  */
  class MITK_CORE_EXPORT AbstractFileWriter : public mitk::IFileWriter
  {
  public:

    /**
    * \brief Write the data in <code>data</code> to the the location specified in <code>path</code>
    */
    virtual void Write(const BaseData* data, const std::string& path);

    /**
    * \brief Write the data in <code>data</code> to the the stream specified in <code>stream</code>
    */
    virtual void Write(const BaseData* data, std::ostream& stream ) = 0;

    /**
    * \brief Returns the priority which defined how 'good' the FileWriter can handle it's file format.
    *
    * Default is zero and should only be chosen differently for a reason.
    * The priority is intended to be used by the MicroserviceFramework to determine
    * which reader to use if several equivalent readers have been found.
    * It may be used to replace a default reader from MITK in your own project.
    * E.g. if you want to use your own reader for *.nrrd files instead of the default,
    * implement it and give it a higher priority than zero.
    *
    * This method has a default implementation and need not be reimplemented, simply set m_Priority in the constructor.
    */
    virtual int GetPriority() const;

    /**
    * \brief returns the file extension that this FileWriter is able to handle.
    *
    * Please enter only the characters after the fullstop, e.g "nrrd" is correct
    * while "*.nrrd" and ".nrrd" are incorrect.
    *
    * This method has a default implementation and need not be reimplemented, simply set m_Extension in the constructor.
    */
    virtual std::string GetExtension() const;

    /**
    * \brief returns the name of the itk Basedata that this FileWriter is able to handle.
    *
    * The correct value is the one given as the first parameter in the ItkNewMacro of that Basedata derivate.
    * You can also retrieve it by calling <code>GetNameOfClass()</code> on an instance of said data.
    *
    * This method has a default implementation and need not be reimplemented, simply set m_BaseDataType in the constructor.
    */
    virtual std::string GetSupportedBasedataType() const;

    /**
    * \brief Returns a human readable description of the file format.
    *
    * This will be used in FileDialogs for example.
    * This method has a default implementation and need not be reimplemented, simply set m_BaseDataType in the constructor.
    */
    virtual std::string GetDescription() const;

    /**
    * \brief returns a list of the supported Options
    *
    * Options are strings that are treated as flags when passed to the read method.
    */
    virtual OptionList GetOptions() const;

    virtual void SetOptions(const OptionList& options);

    /**
    * \brief Return true if this writer can confirm that it can read this file and false otherwise.
    *
    * The default implementation of AbstractFileWriter checks if the supplied filename is of the same extension as m_extension.
    * Overwrite this method if you require more specific behaviour
    */
    virtual bool CanWrite(const BaseData* data) const;

    /**
    * \brief Returns a value between 0 and 1 depending on the progress of the writing process.
    * This method need not necessarily be implemented, always returning zero is accepted.
    */
    virtual float GetProgress() const;

    us::ServiceRegistration<IFileWriter> RegisterService(us::ModuleContext* context = us::GetModuleContext());

  protected:
    AbstractFileWriter();
    ~AbstractFileWriter();

    AbstractFileWriter(const AbstractFileWriter& other);

    AbstractFileWriter(const std::string& basedataType, const std::string& extension, const std::string& description);

    // Minimal Service Properties: ALWAYS SET THESE IN CONSTRUCTOR OF DERIVED CLASSES!
    std::string m_Extension;
    std::string m_BasedataType;
    std::string m_Description;
    int m_Priority;
    OptionList m_Options; // Options supported by this Writer. Can be left emtpy if no special options are required

    virtual us::ServiceProperties GetServiceProperties() const;

  private:

    us::PrototypeServiceFactory* m_PrototypeFactory;

    virtual mitk::IFileWriter* Clone() const = 0;
  };
} // namespace mitk

#endif /* AbstractFileWriter_H_HEADER_INCLUDED_C1E7E521 */
