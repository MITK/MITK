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

#ifndef _MITK_NRRDDIFFVOL_WRITER__H_
#define _MITK_NRRDDIFFVOL_WRITER__H_

#include <mitkAbstractFileWriter.h>
#include <mitkDiffusionImage.h>

namespace mitk
{

/**
 * Writes diffusion volumes to a file
 * @ingroup Process
 */
class NrrdDiffusionImageWriter : public mitk::AbstractFileWriter
{
public:

    typedef mitk::DiffusionImage<short> InputType;

    NrrdDiffusionImageWriter();
    virtual ~NrrdDiffusionImageWriter();

    using AbstractFileWriter::Write;
    virtual void Write();

    virtual ConfidenceLevel GetConfidenceLevel() const;

  protected:

    NrrdDiffusionImageWriter(const NrrdDiffusionImageWriter& other);
    virtual mitk::NrrdDiffusionImageWriter* Clone() const;


};


} // end of namespace mitk

#endif
