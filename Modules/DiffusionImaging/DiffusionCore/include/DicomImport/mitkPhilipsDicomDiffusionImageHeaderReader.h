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
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** read in the data */
    void Update() override;

  protected:

    /** Default c'tor */
    PhilipsDicomDiffusionImageHeaderReader();

    /** Default d'tor */
    ~PhilipsDicomDiffusionImageHeaderReader() override;

  };

}

#endif



