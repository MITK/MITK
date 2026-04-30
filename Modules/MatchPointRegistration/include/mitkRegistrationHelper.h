/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRegistrationHelper_h
#define mitkRegistrationHelper_h

//ITK
#include <itkScalableAffineTransform.h>

//MatchPoint
#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistration.h>

//MITK
#include <mitkImage.h>
#include <mitkDataNode.h>
#include <mitkNodePredicateBase.h>

//MITK
#include <MitkMatchPointRegistrationExports.h>
#include <mitkMAPRegistrationWrapper.h>

namespace mitk
{
/**
 * \brief Static utility class providing helper methods for working with MatchPoint registrations in MITK.
 *
 * MITKRegistrationHelper offers convenience functions for extracting affine matrices from
 * registrations, checking registration dimensionality, identifying registration nodes in the
 * data storage, and obtaining node predicates for common data types.
 *
 * \sa mitk::MAPRegistrationWrapper, mitk::ImageMappingHelper, mitk::PointSetMappingHelper
 */
class MITKMATCHPOINTREGISTRATION_EXPORT MITKRegistrationHelper
{
public:

  /** \brief 3D scalable affine transform type used for matrix extraction. */
  typedef ::itk::ScalableAffineTransform< ::mitk::ScalarType,3 > Affine3DTransformType;
  /** \brief 3D-to-3D MatchPoint registration type. */
  typedef ::map::core::Registration<3,3> Registration3DType;
  /** \brief Base type of all MatchPoint registrations. */
  typedef ::map::core::RegistrationBase RegistrationBaseType;

  /**
   * \brief Extracts the affine transformation matrix from a registration wrapper's kernel.
   *
   * \param[in] wrapper Pointer to the registration wrapper.
   * \param[in] inverseKernel If true, extract from the inverse kernel; if false, from the direct kernel.
   * \return Pointer to the extracted affine transform, or nullptr if extraction is not possible
   *         (e.g. the kernel is not affine-decomposable).
   * \pre \p wrapper must point to a valid instance.
   * \pre \p wrapper must wrap a 3D-3D registration.
   */
  static Affine3DTransformType::Pointer getAffineMatrix(const mitk::MAPRegistrationWrapper* wrapper, bool inverseKernel);

  /**
   * \brief Extracts the affine transformation matrix from a MatchPoint registration's kernel.
   *
   * \param[in] registration Pointer to the MatchPoint registration base.
   * \param[in] inverseKernel If true, extract from the inverse kernel; if false, from the direct kernel.
   * \return Pointer to the extracted affine transform, or nullptr if extraction is not possible.
   * \pre \p registration must point to a valid 3D-3D registration instance.
   */
  static Affine3DTransformType::Pointer getAffineMatrix(const RegistrationBaseType* registration, bool inverseKernel);

  /**
   * \brief Checks whether the given registration wrapper represents a 3D-to-3D registration.
   *
   * \param[in] wrapper Pointer to the registration wrapper.
   * \return True if both moving and target dimensions are 3, false otherwise or if wrapper is nullptr.
   */
  static bool is3D(const mitk::MAPRegistrationWrapper* wrapper);

  /**
   * \brief Checks whether the given MatchPoint registration is a 3D-to-3D registration.
   *
   * \param[in] regBase Pointer to the MatchPoint registration base.
   * \return True if both moving and target dimensions are 3, false otherwise or if regBase is nullptr.
   */
  static bool is3D(const RegistrationBaseType* regBase);

  /**
   * \brief Checks whether the given DataNode contains a MatchPoint registration.
   *
   * \param[in] node Pointer to the node to be checked.
   * \return True if the node contains a MAPRegistrationWrapper. False if \p node is nullptr
   *         or does not contain a registration wrapper.
   */
  static bool IsRegNode(const mitk::DataNode* node);

  /**
   * \brief Returns a node predicate that identifies nodes containing MatchPoint registrations.
   *
   * \return A constant pointer to a NodePredicateBase matching MAPRegistrationWrapper data.
   */
  static NodePredicateBase::ConstPointer RegNodePredicate();

  /**
   * \brief Returns a node predicate that identifies nodes containing images.
   *
   * \return A constant pointer to a NodePredicateBase matching mitk::Image data.
   */
  static NodePredicateBase::ConstPointer ImageNodePredicate();

  /**
   * \brief Returns a node predicate that identifies nodes containing point sets.
   *
   * \return A constant pointer to a NodePredicateBase matching mitk::PointSet data.
   */
  static NodePredicateBase::ConstPointer PointSetNodePredicate();

private:
    typedef ::map::core::Registration<3,3>::DirectMappingType RegistrationKernel3DBase;
    static Affine3DTransformType::Pointer getAffineMatrix(const RegistrationKernel3DBase& kernel);

    MITKRegistrationHelper();
    ~MITKRegistrationHelper();
    MITKRegistrationHelper& operator = (const MITKRegistrationHelper&);
    MITKRegistrationHelper(const MITKRegistrationHelper&);

};

}

#endif
