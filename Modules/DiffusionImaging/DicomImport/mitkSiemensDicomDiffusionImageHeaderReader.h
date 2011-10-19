/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 13:23:19 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 13561 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __mitkSiemensDicomDiffusionImageHeaderReader_h__
#define __mitkSiemensDicomDiffusionImageHeaderReader_h_

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
    itkNewMacro(Self);

    /** Extract the key value for the key nameString from the tagString */
    int ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray, int startPos = 0 );

    /** Extracts the diffusion gradient information from the tagString and stores it into the valueArray vector */
    int ExtractSiemensDiffusionGradientInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray);

    /** Read the data */
    virtual void Update();

  protected:
    /** Default c'tor */
    SiemensDicomDiffusionImageHeaderReader();

    /** Default d'tor */
    virtual ~SiemensDicomDiffusionImageHeaderReader();

  };

}

#endif



