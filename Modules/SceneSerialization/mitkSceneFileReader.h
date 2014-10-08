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

#ifndef SceneFileReader_H_HEADER_INCLUDED
#define SceneFileReader_H_HEADER_INCLUDED

// MITK
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>

// Microservices
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usGetModuleContext.h>

namespace us {
  struct PrototypeServiceFactory;
}

namespace mitk {

  class CustomMimeType;

  /**
  * @brief TODO
  * @ingroup IO
  */
  class SceneFileReader : public mitk::AbstractFileReader
  {

  public:

    SceneFileReader(const SceneFileReader& other);

    SceneFileReader();

    virtual ~SceneFileReader();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read();

  protected:

  private:

    SceneFileReader* Clone() const;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };
} // namespace mitk

#endif /* SceneFileReader_H_HEADER_INCLUDED_C1E7E521 */