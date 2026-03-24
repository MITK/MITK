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
   * @brief Used to write Surfaces to the PLY format.
   *
   * Currently, this writer uses the binary format as standard. Should the ASCII Format be required,
   * it is suggested to incorporate Options.
   *
   * @ingroup IOExt
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
