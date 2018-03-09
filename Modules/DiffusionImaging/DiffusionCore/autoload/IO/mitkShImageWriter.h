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

#ifndef _MITK_SH_WRITER__H_
#define _MITK_SH_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkShImage.h>


namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
  class ShImageWriter : public mitk::AbstractFileWriter
  {
  public:

    typedef itk::VectorImage<float, 3> VecImgType;
    typedef mitk::ShImage InputType;

    ShImageWriter();
    ~ShImageWriter() override;

    using AbstractFileWriter::Write;
    void Write() override;

    ConfidenceLevel GetConfidenceLevel() const override;

  protected:

    ShImageWriter(const ShImageWriter& other);
    mitk::ShImageWriter* Clone() const override;

    template <int shOrder>
    void WriteShImage(InputType::ConstPointer input);
  };


} // end of namespace mitk

#endif //_MITK_NRRDODF_WRITER__H_
