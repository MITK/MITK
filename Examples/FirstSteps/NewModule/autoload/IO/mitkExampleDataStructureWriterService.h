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

#ifndef __mitkExampleDataStructureWriterService_h
#define __mitkExampleDataStructureWriterService_h

#include <mitkAbstractFileWriter.h>
#include <mitkExampleDataStructure.h>

namespace mitk
{

  /**
  * Writes example data strucutres to a file
  * @ingroup Process
  */
  class ExampleDataStructureWriterService : public mitk::AbstractFileWriter
  {
  public:
    typedef mitk::ExampleDataStructure InputType;

    ExampleDataStructureWriterService();
    virtual ~ExampleDataStructureWriterService();

    using AbstractFileWriter::Write;
    virtual void Write() override;

  protected:

    ExampleDataStructureWriterService(const ExampleDataStructureWriterService& other);
    virtual mitk::ExampleDataStructureWriterService* Clone() const override;

  };


} // end of namespace mitk

#endif //__mitkExampleDataStructureWriterService_h
