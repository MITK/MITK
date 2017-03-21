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


#ifndef mitkPhotoacousticImage_H_HEADER_INCLUDED
#define mitkPhotoacousticImage_H_HEADER_INCLUDED

#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkImage.h"

#include "Algorithms/mitkPhotoacousticBeamformingDASFilter.h"
#include "Algorithms/mitkPhotoacousticBeamformingDMASFilter.h"

#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {

    class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticImage : public itk::Object
    {
    public:
      mitkClassMacroItkParent(mitk::PhotoacousticImage, itk::Object);
      itkFactorylessNewMacro(Self);
      mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer inputImage, bool UseLogFilter = false, float resampleSpacing = 0.15);
//      mitk::Image::Pointer ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scatteringCoefficient);
      mitk::Image::Pointer ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[3]);
      mitk::Image::Pointer ApplyBeamformingDAS(mitk::Image::Pointer inputImage, BeamformingDASFilter::beamformingSettings config, int cutoff);
      mitk::Image::Pointer ApplyBeamformingDMAS(mitk::Image::Pointer inputImage, BeamformingDMASFilter::beamformingSettings config, int cutoff);
      mitk::Image::Pointer ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left);

    protected:
      PhotoacousticImage();
      virtual ~PhotoacousticImage();
    };
} // namespace mitk

#endif /* mitkPhotoacousticImage_H_HEADER_INCLUDED */
