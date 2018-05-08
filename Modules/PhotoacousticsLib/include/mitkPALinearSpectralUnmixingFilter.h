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

#ifndef MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
#define MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H

#include <mitkPASpectralUnmixingFilterBase.h>
#include <MitkPhotoacousticsLibExports.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT LinearSpectralUnmixingFilter : public SpectralUnmixingFilterBase
    {
    public:

      mitkClassMacro(LinearSpectralUnmixingFilter, SpectralUnmixingFilterBase)
        itkFactorylessNewMacro(Self)


      void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(std::string chosenAlgorithm);


    protected:
      LinearSpectralUnmixingFilter();
      virtual ~LinearSpectralUnmixingFilter();

 

      // Test algorithm for SU --> later a new class should be set up
      virtual Eigen::VectorXf SpectralUnmixingAlgorithm(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
        Eigen::VectorXf inputVector) override;

    private:

      
      
    };
  }
}
#endif // MITKLINEARPHOTOACOUSTICSPECTRALUNMIXINGFILTER_H
