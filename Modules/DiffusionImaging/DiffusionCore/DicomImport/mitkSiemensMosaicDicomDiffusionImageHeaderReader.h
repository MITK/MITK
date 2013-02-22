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


#ifndef __mitkSiemensMosaicDicomDiffusionImageHeaderReader_h__
#define __mitkSiemensMosaicDicomDiffusionImageHeaderReader_h__

#include "mitkDicomDiffusionImageHeaderReader.h"

namespace mitk
{
  /**
    @brief The SiemensMosaicDicomDiffusionImageHeaderReader class reads the image header for diffusion MR data stored in
    Siemens MOSAIC Format
    */
  class SiemensMosaicDicomDiffusionImageHeaderReader : public DicomDiffusionImageHeaderReader
  {
  public:
    mitkClassMacro( SiemensMosaicDicomDiffusionImageHeaderReader, DicomDiffusionImageHeaderReader );
    itkNewMacro(Self);

    /** Extracts the information stored under the key nameString in the tagString */
    int ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray );

    /** Read the data */
    virtual void Update();

  protected:
    /** Default c'tor */
    SiemensMosaicDicomDiffusionImageHeaderReader();

    /** Default class d'tor */
    virtual ~SiemensMosaicDicomDiffusionImageHeaderReader();
  };

}

#endif



