/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMaskedAlgorithmHelper_h
#define mitkMaskedAlgorithmHelper_h

#include "itkSpatialObject.h"

//MatchPoint
#include "mapRegistrationAlgorithmBase.h"

//MITK
#include <mitkImage.h>

//MITK
#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
  /*!
    \brief MaskedAlgorithmHelper
    Helper class as an easy bridge to set mitk images as masks for registration algorithms. It is assumed that the
    Image indicates the mask by pixel values != 0.
    \remark Currently only 2D-2D and 3D-3D algorithms are supported.
    \remark Current implementation is not thread-save. Just use one Helper class per registration task.
  */
  class MITKMATCHPOINTREGISTRATION_EXPORT MaskedAlgorithmHelper
  {
  public:

    MaskedAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase* algorithm);

    /** Set one or both masks to an algorithm.
     * If the algorithm does not support masks it will be ignored.
     * @remark Set a mask to nullptr if you don't want to set it.
     * @return Indicates if the masks could be set/was supported by algorithm.*/
    bool SetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask);

    /** Checks if the algorithm supports masks of the passed type.*/
    bool CheckSupport(const mitk::Image* movingMask, const mitk::Image* targetMask) const;

    static bool HasMaskedRegistrationAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm);

    ~MaskedAlgorithmHelper() {}

  private:
    using MaskPixelType = unsigned char;

    MaskedAlgorithmHelper& operator = (const MaskedAlgorithmHelper&);
    MaskedAlgorithmHelper(const MaskedAlgorithmHelper&);

    /**Internal helper that is used by SetMasks if the data are images to set them properly.*/
    template<unsigned int VImageDimension1, unsigned int VImageDimension2>
    bool DoSetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask);

    /**Internal helper that is used by SetData if the data are images to cast and set them properly.*/
    template<typename TPixelType, unsigned int VImageDimension>
    void DoConvertMask(const itk::Image<TPixelType, VImageDimension>* mask);
    /**Internal helper that is used by SetData if the data are images to set them properly.*/
    template<unsigned int VImageDimension>
    void DoConvertMask(const itk::Image<MaskPixelType, VImageDimension>* mask);

    /**Internal helper that is used to pack the mask image into a spatial object.*/
    template<unsigned int VImageDimension>
    typename itk::SpatialObject<VImageDimension>::Pointer ConvertMaskSO(const itk::Image<MaskPixelType, VImageDimension>* mask) const;

    /**Helper member that containes the result of the last call of DoConvertMask().*/
    itk::DataObject::Pointer m_convertResult;

    map::algorithm::RegistrationAlgorithmBase::Pointer m_AlgorithmBase;
  };

}

#endif

