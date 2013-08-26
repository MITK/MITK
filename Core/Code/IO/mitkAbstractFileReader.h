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

#ifndef AbstractFileReader_H_HEADER_INCLUDED_C1E7E521
#define AbstractFileReader_H_HEADER_INCLUDED_C1E7E521

// Macro
#include <MitkExports.h>

// MITK
#include <mitkIFileReader.h>
#include <mitkBaseData.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {
  //##Documentation
  //## @brief Interface class of readers that read from files
  //## @ingroup Process
  class MITK_CORE_EXPORT AbstractFileReader : public mitk::IFileReader
  {
  public:

    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::string& path, mitk::DataStorage *ds = 0 );

    virtual std::list< itk::SmartPointer<BaseData> > Read(const std::istream& stream, mitk::DataStorage *ds = 0 ) = 0;

    virtual int GetPriority() const;

    virtual std::string GetExtension() const;

    virtual std::string GetDescription() const;

    virtual std::list< mitk::IFileReader::FileServiceOption > GetOptions() const;

    virtual void SetOptions(std::list< mitk::IFileReader::FileServiceOption > options);

    virtual bool CanRead(const std::string& path) const;

    virtual float GetProgress() const;

    us::ServiceRegistration<IFileReader> RegisterService(us::ModuleContext* context = us::GetModuleContext());

  protected:
    AbstractFileReader();
    ~AbstractFileReader();

    AbstractFileReader(const AbstractFileReader& other);

    AbstractFileReader(const std::string& extension, const std::string& description);

    // Minimal Service Properties: ALWAYS SET THESE IN CONSTRUCTOR OF DERIVED CLASSES!
    std::string m_Extension;
    std::string m_Description;
    int m_Priority;

    /**
    * \brief Options supported by this reader. Set sensible default values!
    *
    * Can be left emtpy if no special options are required.
    */
    std::list< mitk::IFileReader::FileServiceOption > m_Options;

    virtual us::ServiceProperties GetServiceProperties();

  private:

    us::PrototypeServiceFactory* m_PrototypeFactory;

    virtual mitk::IFileReader* Clone() const = 0;
  };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::AbstractFileReader, "org.mitk.services.FileReader")

#endif /* AbstractFileReader_H_HEADER_INCLUDED_C1E7E521 */
