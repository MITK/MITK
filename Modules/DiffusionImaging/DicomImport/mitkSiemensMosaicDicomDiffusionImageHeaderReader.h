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



