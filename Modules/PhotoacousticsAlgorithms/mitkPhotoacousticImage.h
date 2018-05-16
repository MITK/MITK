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
#include <functional>

#include "mitkPhotoacousticBeamformingFilter.h"
#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {

    class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticImage : public itk::Object
    {
    public:
      mitkClassMacroItkParent(mitk::PhotoacousticImage, itk::Object);
      itkFactorylessNewMacro(Self);
      enum BModeMethod { ShapeDetection, Abs };
      mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer inputImage, BModeMethod method = BModeMethod::Abs, bool UseGPU = false, bool UseLogFilter = false, float resampleSpacing = 0.15);
//      mitk::Image::Pointer ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scatteringCoefficient);
      mitk::Image::Pointer ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[2]);
      mitk::Image::Pointer ApplyBeamforming(mitk::Image::Pointer inputImage, BeamformingSettings config, int cutoff, std::function<void(int, std::string)> progressHandle = [](int, std::string) {});
      mitk::Image::Pointer ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left, int minSlice, int maxSlice);
      mitk::Image::Pointer BandpassFilter(mitk::Image::Pointer data, float recordTime, float BPHighPass, float BPLowPass, float alpha);
    protected:
      PhotoacousticImage();
      virtual ~PhotoacousticImage();

      itk::Image<float, 3U>::Pointer BPFunction(mitk::Image::Pointer reference, int cutoffFrequencyPixelHighPass, int cutoffFrequencyPixelLowPass, float alpha);
    };
} // namespace mitk

#endif /* mitkPhotoacousticImage_H_HEADER_INCLUDED */
