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

#ifndef _MITK_NRRDDTI_WRITER__H_
#define _MITK_NRRDDTI_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkTensorImage.h>


namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
class NrrdTensorImageWriter : public mitk::AbstractFileWriter
{
public:

    typedef mitk::TensorImage InputType;

    NrrdTensorImageWriter();
    virtual ~NrrdTensorImageWriter();

    using AbstractFileWriter::Write;
    virtual void Write();

    virtual ConfidenceLevel GetConfidenceLevel() const;

protected:

  NrrdTensorImageWriter(const NrrdTensorImageWriter& other);
  virtual mitk::NrrdTensorImageWriter* Clone() const;
};


} // end of namespace mitk

#endif //_MITK_NRRDDTI_WRITER__H_
