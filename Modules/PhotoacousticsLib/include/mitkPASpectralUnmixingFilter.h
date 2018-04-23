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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H

#include "mitkImageToImageFilter.h"

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilter : public mitk::ImageToImageFilter
    {
    public:

      mitkClassMacro(SpectralUnmixingFilter, mitk::ImageToImageFilter)
        itkFactorylessNewMacro(Self)

      //Contains all (so far) possible chromophores
      enum ChromophoreType
      {
        OXYGENATED, DEOXYGENATED
      };

      void AddWavelength(int wavelength);
      void Test();
      std::vector<int> m_Wavelengths;
      int size;

    protected:
      SpectralUnmixingFilter();
      virtual ~SpectralUnmixingFilter();

    private:

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
