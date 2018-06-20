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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_H

#include "mitkImageToImageFilter.h"
#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"


namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingSO2 : public mitk::ImageToImageFilter
    {
    public:
      mitkClassMacro(SpectralUnmixingSO2, mitk::ImageToImageFilter);
      itkFactorylessNewMacro(Self);
      virtual void AddSO2Settings(float value);
      /*
      * \brief Verbose gives more information to the console. Default value is false.
      * @param m_Verbose is the boolian to activate the MITK_INFO logged to the console
      */
      virtual void Verbose(bool verbose);

    protected:
      SpectralUnmixingSO2();
      virtual ~SpectralUnmixingSO2();
      std::vector<int> m_SO2Settings;
      bool m_Verbose = false;

    private:
      virtual void GenerateData() override;
      virtual void InitializeOutputs();
      virtual void CheckPreConditions(mitk::Image::Pointer inputHbO2, mitk::Image::Pointer inputHb);
      float calculateSO2(float pixelHb, float pixelHbO2);
    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGSO2_
