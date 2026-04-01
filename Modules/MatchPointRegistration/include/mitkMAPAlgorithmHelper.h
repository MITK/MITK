/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMAPAlgorithmHelper_h
#define mitkMAPAlgorithmHelper_h


//MatchPoint
#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistrationBase.h>

//MITK
#include <mitkImage.h>
#include <mitkPointSet.h>

//MITK
#include <MitkMatchPointRegistrationExports.h>
#include <mitkMAPRegistrationWrapper.h>

namespace mitk
{
  /**
   * \brief Helper class for bridging between MITK data objects and MatchPoint registration algorithms.
   *
   * MAPAlgorithmHelper simplifies setting MITK images or point sets as input data for MatchPoint
   * registration algorithms. It handles data type checking, optional pixel type casting, and
   * extraction of registration results.
   *
   * \note Current implementation is not thread-safe. Use one helper instance per registration task.
   *
   * \sa mitk::MAPRegistrationWrapper, mitk::MaskedAlgorithmHelper
   */
  class MITKMATCHPOINTREGISTRATION_EXPORT MAPAlgorithmHelper
  {
  public:

    /**
     * \brief Constructs a helper for the given registration algorithm.
     * \param[in] algorithm Pointer to the MatchPoint registration algorithm to assist.
     */
    MAPAlgorithmHelper(map::algorithm::RegistrationAlgorithmBase* algorithm);

    /**
     * \brief Sets moving and target data on the algorithm.
     *
     * The data can be either images or point sets. If images require pixel type casting
     * and casting is allowed, they will be cast to the algorithm's default internal type.
     *
     * \param[in] moving Pointer to the moving data (Image or PointSet).
     * \param[in] target Pointer to the target data (Image or PointSet).
     * \pre The algorithm must be set.
     * \pre \p moving and \p target must be valid pointers.
     * \throw map::core::ExceptionObject if the data cannot be set on the algorithm.
     */
    void SetData(const mitk::BaseData* moving, const mitk::BaseData* target);

    /**
     * \brief Enables or disables automatic pixel type casting for images.
     *
     * When enabled, images whose pixel types differ from the algorithm's expected type
     * will be automatically cast to the default internal pixel type.
     *
     * \param[in] allowCasting True to allow casting, false to disallow.
     */
    void SetAllowImageCasting(bool allowCasting);

    /**
     * \brief Returns whether automatic pixel type casting is currently allowed.
     * \return True if casting is allowed, false otherwise.
     */
    bool GetAllowImageCasting() const;

    /**
     * \brief Checks whether the given algorithm supports image-based registration.
     * \param[in] algorithm Pointer to the algorithm to check.
     * \return True if the algorithm implements the ImageRegistrationAlgorithmInterface.
     */
    static bool HasImageAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm);

    /**
     * \brief Checks whether the given algorithm supports point-set-based registration.
     * \param[in] algorithm Pointer to the algorithm to check.
     * \return True if the algorithm implements the PointSetRegistrationAlgorithmInterface.
     */
    static bool HasPointSetAlgorithmInterface(const map::algorithm::RegistrationAlgorithmBase* algorithm);

    /**
     * \brief Error codes returned by CheckData() indicating data compatibility issues.
     */
    struct CheckError
    {
      enum Type
      {
        none = 0,              /**< \brief No error, data is fully compatible. */
        onlyByCasting = 1,     /**< \brief Data compatible only if image pixel type casting is performed. */
        wrongDimension = 2,    /**< \brief Data dimensions do not match the algorithm's requirements. */
        unsupportedDataType = 3 /**< \brief Data type is not supported by the algorithm. */
      };
    };

    /**
     * \brief Checks whether the given moving and target data are compatible with the algorithm.
     *
     * \param[in] moving Pointer to the moving data.
     * \param[in] target Pointer to the target data.
     * \param[out] error The specific error type if the data is incompatible.
     * \return True if the data can be used with the algorithm (considering the AllowImageCasting setting).
     * \pre The algorithm, \p moving, and \p target must be valid pointers.
     */
    bool CheckData(const mitk::BaseData* moving, const mitk::BaseData* target,
                   CheckError::Type& error) const;

    /**
     * \brief Retrieves the registration result from the algorithm.
     *
     * \return Smart pointer to the computed MatchPoint registration, or nullptr if unavailable.
     * \throw map::core::ExceptionObject if the algorithm has unsupported dimensionality.
     */
    map::core::RegistrationBase::Pointer GetRegistration() const;

    /**
     * \brief Retrieves the registration result wrapped in a MITK-compatible wrapper.
     *
     * \return Smart pointer to a MAPRegistrationWrapper containing the algorithm's registration.
     * \sa GetRegistration
     */
    mitk::MAPRegistrationWrapper::Pointer GetMITKRegistrationWrapper() const;

    ~MAPAlgorithmHelper() {}

  private:

    MAPAlgorithmHelper& operator = (const MAPAlgorithmHelper&);
    MAPAlgorithmHelper(const MAPAlgorithmHelper&);

    /**Internal helper that casts itk images from one pixel type into an other
      (used by DoSetImages if the images have the right dimension but wrong type and AllowImageCasting is activated)*/
    template<typename TInImageType, typename TOutImageType>
    typename TOutImageType::Pointer CastImage(const TInImageType* input) const;

    /**Internal helper that is used by SetData if the data are images to set them properly.*/
    template<typename TPixelType1, unsigned int VImageDimension1,
             typename TPixelType2, unsigned int VImageDimension2>
    void DoSetImages(const itk::Image<TPixelType1, VImageDimension1>* moving,
                     const itk::Image<TPixelType2, VImageDimension2>* target);

    /**Internal helper that is used by SetData if the data are images to check if the image types are supported by the algorithm.*/
    template<typename TPixelType1, unsigned int VImageDimension1,
             typename TPixelType2, unsigned int VImageDimension2>
    void DoCheckImages(const itk::Image<TPixelType1, VImageDimension1>* moving,
                       const itk::Image<TPixelType2, VImageDimension2>* target) const;

    map::algorithm::RegistrationAlgorithmBase::Pointer m_AlgorithmBase;

    bool m_AllowImageCasting;

    mutable CheckError::Type m_Error;
  };

  /**
   * \brief Generates a 3D identity registration wrapped as a MAPRegistrationWrapper.
   *
   * Creates a dummy registration algorithm that produces an identity transform mapping,
   * useful as a default or placeholder registration.
   *
   * \return Smart pointer to a MAPRegistrationWrapper containing the 3D identity registration.
   */
  mitk::MAPRegistrationWrapper::Pointer MITKMATCHPOINTREGISTRATION_EXPORT GenerateIdentityRegistration3D();

}

#endif
