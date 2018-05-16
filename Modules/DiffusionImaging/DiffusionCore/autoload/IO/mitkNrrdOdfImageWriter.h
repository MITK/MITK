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

#ifndef _MITK_NRRDODF_WRITER__H_
#define _MITK_NRRDODF_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkOdfImage.h>


namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
  class NrrdOdfImageWriter : public mitk::AbstractFileWriter
  {
  public:

    typedef mitk::OdfImage InputType;

    NrrdOdfImageWriter();
    ~NrrdOdfImageWriter() override;

    using AbstractFileWriter::Write;
    void Write() override;

    ConfidenceLevel GetConfidenceLevel() const override;

  protected:

    NrrdOdfImageWriter(const NrrdOdfImageWriter& other);
    mitk::NrrdOdfImageWriter* Clone() const override;
  };


} // end of namespace mitk

#endif //_MITK_NRRDODF_WRITER__H_
