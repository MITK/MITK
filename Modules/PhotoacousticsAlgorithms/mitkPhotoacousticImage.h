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
#include "mitkOclImageToImageFilter.h"

#include "mitkPhotoacousticBeamformingFilter.h"

#include "MitkPhotoacousticsAlgorithmsExports.h"

namespace mitk {

    class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticImage : public itk::Object
    {
    public:
      mitkClassMacroItkParent(mitk::PhotoacousticImage, itk::Object);
      itkFactorylessNewMacro(Self);
      mitk::Image::Pointer ApplyBmodeFilter(mitk::Image::Pointer inputImage, bool UseLogFilter = false, float resampleSpacing = 0.15);
//      mitk::Image::Pointer ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scatteringCoefficient);
      mitk::Image::Pointer ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[2]);
      mitk::Image::Pointer ApplyBeamforming(mitk::Image::Pointer inputImage, BeamformingFilter::beamformingSettings config, int cutoff, std::function<void(int, std::string)> progressHandle = [](int, std::string) {});
      mitk::Image::Pointer ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left, int minSlice, int maxSlice);
      mitk::Image::Pointer BandpassFilter(mitk::Image::Pointer data, float recordTime, float BPHighPass, float BPLowPass, unsigned int butterworthOrder);
    protected:
      PhotoacousticImage();
      virtual ~PhotoacousticImage();

      itk::Image<float, 3U>::Pointer BPFunction(mitk::Image::Pointer reference, int width, int center, unsigned int butterworthOrder);
    };

    /*
    class CropFilter : public OclImageToImageFilter, public itk::Object
    {

    public:
      mitkClassMacroItkParent(CropFilter, itk::Object);
      itkNewMacro(Self);

      void SetInput(Image::Pointer image);

      mitk::Image::Pointer GetOutput();

      void Update();

      void SetCropping(int above, int below, int right, int left, int minSlice, int maxSlice)
      {
        m_Above = above;
        m_Below = below;
        m_Right = right;
        m_Left = left;
        m_MinSlice = minSlice;
        m_MaxSlice = maxSlice;
      }


    protected:
      CropFilter();

      virtual ~CropFilter();

      bool Initialize();

      void Execute();

      mitk::PixelType GetOutputType()
      {
        return mitk::MakeScalarPixelType<float>();
      }

      int GetBytesPerElem()
      {
        return sizeof(double);
      }

      virtual us::Module* GetModule();

    private:
      cl_kernel m_PixelCalculation;

      int m_Above; 
      int m_Below;
      int m_Right;
      int m_Left;
      int m_MinSlice;
      int m_MaxSlice;

      unsigned int m_OutputDim[3];
    };
    */
} // namespace mitk

#endif /* mitkPhotoacousticImage_H_HEADER_INCLUDED */
