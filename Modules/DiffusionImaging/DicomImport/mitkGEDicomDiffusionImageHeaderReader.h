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


#ifndef __mitkGEDicomDiffusionImageHeaderReader_h__
#define __mitkGEDicomDiffusionImageHeaderReader_h_

#include "mitkDicomDiffusionImageHeaderReader.h"

namespace mitk
{

  class GEDicomDiffusionImageHeaderReader : public DicomDiffusionImageHeaderReader
  {
  public:

    mitkClassMacro( GEDicomDiffusionImageHeaderReader, DicomDiffusionImageHeaderReader );
    itkNewMacro(Self);

    virtual void Update();

  protected:

    GEDicomDiffusionImageHeaderReader();
    virtual ~GEDicomDiffusionImageHeaderReader();

  };

}

#endif



