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

#ifndef __mitkExampleDataStructureReaderService_h
#define __mitkExampleDataStructureReaderService_h

#include "mitkCommon.h"
#include <mitkAbstractFileReader.h>
#include <mitkBaseData.h>
#include <mitkMimeType.h>
#include "mitkExampleDataStructure.h"

namespace mitk
{

  /** \brief The reader service for the MITK example data type
  */

  class ExampleDataStructureReaderService : public mitk::AbstractFileReader
  {
  public:

    typedef mitk::ExampleDataStructure OutputType;

    ExampleDataStructureReaderService(const ExampleDataStructureReaderService& other);
    ExampleDataStructureReaderService();
    virtual ~ExampleDataStructureReaderService();

    using AbstractFileReader::Read;
    virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

  private:
    ExampleDataStructureReaderService* Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
  };

} //namespace MITK

#endif // __mitkExampleDataStructureReaderService_h
