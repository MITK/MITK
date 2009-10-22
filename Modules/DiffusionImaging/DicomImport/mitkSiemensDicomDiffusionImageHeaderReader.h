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
  class SiemensDicomDiffusionImageHeaderReader : public DicomDiffusionImageHeaderReader 
  {
  public:

    mitkClassMacro( SiemensDicomDiffusionImageHeaderReader, DicomDiffusionImageHeaderReader );
    itkNewMacro(Self);

    int ExtractSiemensDiffusionInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray, int startPos = 0 );
    int ExtractSiemensDiffusionGradientInformation( std::string tagString, std::string nameString, std::vector<double>& valueArray);

    virtual void Update();

  protected:

    SiemensDicomDiffusionImageHeaderReader();
    virtual ~SiemensDicomDiffusionImageHeaderReader();

  };

}

#endif



