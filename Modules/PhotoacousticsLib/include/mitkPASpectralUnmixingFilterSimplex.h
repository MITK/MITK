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

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterSimplex : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterSimplex, SpectralUnmixingFilterBase)
        //itkFactorylessNewMacro(Self)
     
    protected:
      SpectralUnmixingFilterSimplex();
      virtual ~SpectralUnmixingFilterSimplex();

    private:

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERSIMPLEX_H
