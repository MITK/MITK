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
    ~NrrdTensorImageWriter() override;

    using AbstractFileWriter::Write;
    void Write() override;

    ConfidenceLevel GetConfidenceLevel() const override;

protected:

  NrrdTensorImageWriter(const NrrdTensorImageWriter& other);
  mitk::NrrdTensorImageWriter* Clone() const override;
};


} // end of namespace mitk

#endif //_MITK_NRRDDTI_WRITER__H_
