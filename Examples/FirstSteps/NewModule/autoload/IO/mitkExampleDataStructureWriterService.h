/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExampleDataStructureWriterService_h
#define mitkExampleDataStructureWriterService_h

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
    ~ExampleDataStructureWriterService() override;

    using AbstractFileWriter::Write;
    void Write() override;

  protected:
    ExampleDataStructureWriterService(const ExampleDataStructureWriterService &other);
    mitk::ExampleDataStructureWriterService *Clone() const override;
  };

} // end of namespace mitk

#endif
