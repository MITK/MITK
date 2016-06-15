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


#ifndef __mitkSiemensDicomDiffusionImageHeaderReader_h__
#define __mitkSiemensDicomDiffusionImageHeaderReader_h__

#include "mitkDicomDiffusionImageHeaderReader.h"

namespace mitk
{
  /**
    @brief The SiemensDicomDiffusionImageHeaderReader class reads in the header information for the standard Siemens DICOM format
  */
  class SiemensDicomDiffusionImageHeaderReader : public DicomDiffusionImageHeaderReader
  {
  public:

    mitkClassMacro( SiemensDicomDiffusionImageHeaderReader, DicomDiffusionImageHeaderReader );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** Extract the key value for the key nameString from the tagString */
    int ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray, int startPos = 0 );

    /** Extracts the diffusion gradient information from the tagString and stores it into the valueArray vector */
    int ExtractSiemensDiffusionGradientInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray);

    /** Read the data */
    virtual void Update() override;

  protected:
    /** Default c'tor */
    SiemensDicomDiffusionImageHeaderReader();

    /** Default d'tor */
    virtual ~SiemensDicomDiffusionImageHeaderReader();

  };

}

#endif



