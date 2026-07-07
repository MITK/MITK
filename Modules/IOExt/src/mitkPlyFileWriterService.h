/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlyFileWriterService_h
#define mitkPlyFileWriterService_h

#include <mitkAbstractFileWriter.h>

namespace mitk
{
  /**
   * \brief Writer service for saving surfaces in PLY format.
   *
   * Currently, this writer uses the binary format as default. Should the ASCII
   * format be required, it is suggested to incorporate Options.
   */
  class PlyFileWriterService : public AbstractFileWriter
  {
  public:
    PlyFileWriterService();
    ~PlyFileWriterService() override;

    using AbstractFileWriter::Write;
    void Write() override;

  private:
    PlyFileWriterService(const PlyFileWriterService &other);

    mitk::PlyFileWriterService *Clone() const override;
  };
}

#endif
