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


#ifndef PlyFileWriterService_h
#define PlyFileWriterService_h

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
  virtual ~PlyFileWriterService();

  using AbstractFileWriter::Write;
  virtual void Write() override;

private:

  PlyFileWriterService(const PlyFileWriterService& other);

  virtual mitk::PlyFileWriterService* Clone() const override;

};

}

#endif
