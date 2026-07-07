/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMaskedAlgorithmHelper_h
#define mitkMaskedAlgorithmHelper_h

#include <itkSpatialObject.h>

//MatchPoint
#include <mapRegistrationAlgorithmBase.h>

//MITK
#include <mitkImage.h>

//MITK
#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
  /**
   * \brief Helper class for setting MITK images as masks on MatchPoint registration algorithms.
   *
   * MaskedAlgorithmHelper bridges MITK mask images to the MatchPoint MaskedRegistrationAlgorithm
   * interface. Mask pixels with values != 0 are treated as inside the mask region.
   *
   * \note Currently only 2D-2D and 3D-3D algorithms are supported.
   * \note Current implementation is not thread-safe. Use one helper instance per registration task.
   *
   * \sa mitk::MAPAlgorithmHelper
   */
  class MITKMATCHPOINTREGISTRATION_EXPORT MaskedAlgorithmHelper
  {
  public:

    /**
     * \brief Constructs a helper for the given registration algorithm.
     * \param[in] algorithm Pointer to the MatchPoint registration algorithm that should receive masks.
     */
    MaskedAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase* algorithm);

    /**
     * \brief Sets one or both masks on the algorithm.
     *
     * If the algorithm does not support the masked registration interface, the masks are ignored.
     * The mask images are converted to spatial objects internally.
     *
     * \param[in] movingMask Pointer to the mask image for the moving data, or nullptr to skip.
     * \param[in] targetMask Pointer to the mask image for the target data, or nullptr to skip.
     * \return True if the masks were successfully set (algorithm supports masks), false otherwise.
     */
    bool SetMasks(const mitk::Image* movingMask, const mitk::Image* targetMask);

    /**
     * \brief Checks whether the algorithm supports masks of the given types.
     *
     * \param[in] movingMask Pointer to the moving mask image to check compatibility for.
     * \param[in] targetMask Pointer to the target mask image to check compatibility for.
     * \return True if the algorithm supports the given mask types, false otherwise.
     */
    bool CheckSupport(const mitk::Image* movingMask, const mitk::Image* targetMask) const;

    /**
     * \brief Checks whether the given algorithm implements the MaskedRegistrationAlgorithm interface.
     * \param[in] algorithm Pointer to the algorithm to check.
     * \return True if the algorithm supports masks, false otherwise.
     */
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

    /**Helper member that contains the result of the last call of DoConvertMask().*/
    itk::DataObject::Pointer m_convertResult;

    map::algorithm::RegistrationAlgorithmBase::Pointer m_AlgorithmBase;
  };

}

#endif
