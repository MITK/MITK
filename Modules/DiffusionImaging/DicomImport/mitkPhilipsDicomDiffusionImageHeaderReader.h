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


#ifndef __mitkPhilipsDicomDiffusionImageHeaderReader_h__
#define __mitkPhilipsDicomDiffusionImageHeaderReader_h__

#include "mitkDicomDiffusionImageHeaderReader.h"

namespace mitk
{
  /**
    @brief The PhilipsDicomDiffusionImageHeaderReader class reads in the image header information for the Philips DICOM Format */
  class PhilipsDicomDiffusionImageHeaderReader : public DicomDiffusionImageHeaderReader 
  {
  public:

    mitkClassMacro( PhilipsDicomDiffusionImageHeaderReader, DicomDiffusionImageHeaderReader );
    itkNewMacro(Self);

    /** read in the data */
    virtual void Update();

  protected:

    /** Default c'tor */
    PhilipsDicomDiffusionImageHeaderReader();

    /** Default d'tor */
    virtual ~PhilipsDicomDiffusionImageHeaderReader();

  };

}

#endif



