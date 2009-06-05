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


#ifndef __mitkGEDicomDiffusionVolumeHeaderReader_h__
#define __mitkGEDicomDiffusionVolumeHeaderReader_h_

#include "mitkDicomDiffusionVolumeHeaderReader.h"

namespace mitk
{

  class GEDicomDiffusionVolumeHeaderReader : public DicomDiffusionVolumeHeaderReader 
  {
  public:

    mitkClassMacro( GEDicomDiffusionVolumeHeaderReader, DicomDiffusionVolumeHeaderReader );
    itkNewMacro(Self);

    virtual void Update();

  protected:

    GEDicomDiffusionVolumeHeaderReader();
    virtual ~GEDicomDiffusionVolumeHeaderReader();

  };

}

#endif



