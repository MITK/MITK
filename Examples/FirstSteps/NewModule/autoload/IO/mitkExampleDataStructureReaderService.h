/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExampleDataStructureReaderService_h
#define mitkExampleDataStructureReaderService_h

#include "mitkCommon.h"
#include "mitkExampleDataStructure.h"
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>

namespace mitk
{
  /** \brief The reader service for the MITK example data type
  */

  class ExampleDataStructureReaderService : public mitk::AbstractFileReader
  {
  public:
    typedef mitk::ExampleDataStructure OutputType;

    ExampleDataStructureReaderService(const ExampleDataStructureReaderService &other);
    ExampleDataStructureReaderService();
    ~ExampleDataStructureReaderService() override;

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ExampleDataStructureReaderService *Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} // namespace MITK

#endif
