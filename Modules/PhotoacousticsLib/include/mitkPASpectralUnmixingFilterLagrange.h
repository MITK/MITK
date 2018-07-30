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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterLagrange : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterLagrange, SpectralUnmixingFilterBase)
        //itkFactorylessNewMacro(Self)

    protected:
      SpectralUnmixingFilterLagrange();
      virtual ~SpectralUnmixingFilterLagrange();
 
    private:

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H
