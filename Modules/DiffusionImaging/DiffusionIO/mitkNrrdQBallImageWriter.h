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

#ifndef _MITK_NRRDQBI_WRITER__H_
#define _MITK_NRRDQBI_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkQBallImage.h>


namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
  class NrrdQBallImageWriter : public mitk::AbstractFileWriter
  {
  public:

    typedef mitk::QBallImage InputType;

    NrrdQBallImageWriter();
    virtual ~NrrdQBallImageWriter();

    using AbstractFileWriter::Write;
    virtual void Write();

    virtual ConfidenceLevel GetConfidenceLevel() const;

  protected:

    NrrdQBallImageWriter(const NrrdQBallImageWriter& other);
    virtual mitk::NrrdQBallImageWriter* Clone() const;
  };


} // end of namespace mitk

#endif //_MITK_NRRDQBI_WRITER__H_
