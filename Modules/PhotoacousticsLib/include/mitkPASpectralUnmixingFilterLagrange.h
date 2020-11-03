/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H
#define MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SpectralUnmixingFilterLagrange : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(SpectralUnmixingFilterLagrange, SpectralUnmixingFilterBase);

    protected:
      SpectralUnmixingFilterLagrange();
      ~SpectralUnmixingFilterLagrange() override;

    private:

    };
  }
}
#endif // MITKPHOTOACOUSTICSPECTRALUNMIXINGFILTERLAGRANGE_H
