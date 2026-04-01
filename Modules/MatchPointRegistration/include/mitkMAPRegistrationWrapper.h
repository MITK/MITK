/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMAPRegistrationWrapper_h
#define mitkMAPRegistrationWrapper_h

//MITK
#include <mitkBaseData.h>
#include <mitkGeometry3D.h>

//MatchPoint
#include <mapRegistrationBase.h>
#include <mapRegistration.h>
#include <mapExceptionObjectMacros.h>
#include <mapContinuousElements.h>

//MITK
#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{
/**
 * \brief Wrapper class that adapts MatchPoint registration objects to the MITK data model.
 *
 * MAPRegistrationWrapper wraps a %map::core::RegistrationBase instance so that it can be
 * handled as a mitk::BaseData object. This allows registrations to be stored in the MITK
 * DataStorage, displayed in the Data Manager, and visualized via registration mappers.
 *
 * The wrapper provides convenience methods for point mapping (direct and inverse),
 * field representation queries, and kernel precomputation.
 *
 * \sa mitk::MITKRegistrationHelper, mitk::ImageMappingHelper, mitk::MITKRegistrationWrapperMapperBase
 */
class MITKMATCHPOINTREGISTRATION_EXPORT MAPRegistrationWrapper: public mitk::BaseData
{
public:

  mitkClassMacro( MAPRegistrationWrapper, BaseData );

  mitkNewMacro1Param( Self, ::map::core::RegistrationBase*);

  /**
   * \brief Returns the unique identifier of the wrapped registration.
   * \return The UID string of the registration instance.
   */
  Identifiable::UIDType GetUID() const override;

  /**
   * \brief Checks whether the registration is empty at the given time step.
   * \param[in] t The time step to check.
   * \return True if no registration is set (m_spRegistration is null).
   */
  bool IsEmptyTimeStep(unsigned int t) const override;

  /**
   * \brief Checks whether the registration wrapper contains no valid registration.
   * \return True if no registration is set.
   */
  bool IsEmpty() const override;

  /**
   * Empty implementation, since the MAPRegistrationWrapper doesn't
   * support the requested region concept
   */
  void SetRequestedRegionToLargestPossibleRegion() override;

  /**
   * Empty implementation, since the MAPRegistrationWrapper doesn't
   * support the requested region concept
   */
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

  /**
   * Empty implementation, since the MAPRegistrationWrapper doesn't
   * support the requested region concept
   */
  bool VerifyRequestedRegion() override;

  /**
   * Empty implementation, since the MAPRegistrationWrapper doesn't
   * support the requested region concept
   */
  void SetRequestedRegion(const itk::DataObject*) override;

  /**
   * \brief Gets the number of dimensions of the moving space.
   * \return The moving dimensionality of the wrapped registration.
   * \pre A valid registration instance must be set.
   */
  virtual unsigned int GetMovingDimensions() const;

  /**
   * \brief Gets the number of dimensions of the target space.
   * \return The target dimensionality of the wrapped registration.
   * \pre A valid registration instance must be set.
   */
  virtual unsigned int GetTargetDimensions() const;

  /** \brief Tag key type from the MatchPoint registration metadata. */
  typedef ::map::core::RegistrationBase::TagType TagType;
  /** \brief Tag value type from the MatchPoint registration metadata. */
  typedef ::map::core::RegistrationBase::ValueType ValueType;
  /** \brief Map type associating tag keys with their values. */
  typedef ::map::core::RegistrationBase::TagMapType TagMapType;

  /**
   * \brief Returns all metadata tags associated with this registration.
   * \return A const reference to the TagMapType containing all tags.
   * \pre A valid registration instance must be set.
   */
  const TagMapType& GetTags() const;

  /**
   * \brief Retrieves the value for a specific metadata tag.
   * \param[in] tag The tag key to look up.
   * \param[out] value The value associated with the tag, if found.
   * \return True if the tag was found and the value was set, false otherwise.
   * \pre A valid registration instance must be set.
   */
  bool GetTagValue(const TagType & tag, ValueType & value) const;

  /**
   * \brief Checks whether the inverse mapping kernel has a limited representation in target space.
   *
   * \return True if the target representation is limited (the inverse kernel covers only
   *         a part of the target space, so not all inverse mapping operations are guaranteed to succeed).
   *         False if the representation is unlimited (all inverse mappings are guaranteed to succeed).
   * \pre A valid registration instance must be set.
   */
  bool HasLimitedTargetRepresentation() const;

  /**
   * \brief Checks whether the direct mapping kernel has a limited representation in moving space.
   *
   * \return True if the moving representation is limited (the direct kernel covers only
   *         a part of the moving space, so not all direct mapping operations are guaranteed to succeed).
   *         False if the representation is unlimited (all direct mappings are guaranteed to succeed).
   * \pre A valid registration instance must be set.
   */
  bool HasLimitedMovingRepresentation() const;

  /**
   * \brief Maps a point from moving space to target space using the direct mapping kernel.
   *
   * \tparam VMovingDim Dimensionality of the moving space point.
   * \tparam VTargetDim Dimensionality of the target space point.
   * \param[in] inPoint The point in moving space to map.
   * \param[out] outPoint The mapped point in target space (only valid if return is true).
   * \return True if the mapping was successful, false otherwise (e.g. dimension mismatch).
   * \pre A valid registration instance must be set.
   * \pre The direct mapping kernel must be defined.
   * \note The operation may fail if the registration dimensions do not match the point dimensions.
   * \throw mapDefaultException if the registration pointer is null or has invalid dimensions.
   */
  template <unsigned int VMovingDim, unsigned int VTargetDim>
  bool MapPoint(const ::itk::Point<mitk::ScalarType,VMovingDim>& inPoint, ::itk::Point<mitk::ScalarType,VTargetDim>& outPoint) const
  {
    typedef typename ::map::core::continuous::Elements<VMovingDim>::PointType MAPMovingPointType;
    typedef typename ::map::core::continuous::Elements<VTargetDim>::PointType MAPTargetPointType;

    if (m_spRegistration.IsNull())
    {
        mapDefaultExceptionMacro(<< "Error. Cannot map point. Wrapper points to invalid registration (nullptr). Point: " << inPoint);
    }

    bool result = false;

    if ((this->GetMovingDimensions() == VMovingDim)&&(this->GetTargetDimensions() == VTargetDim))
      {
        MAPMovingPointType tempInP;
        MAPTargetPointType tempOutP;
        tempInP.CastFrom(inPoint);

        typedef ::map::core::Registration<VMovingDim,VTargetDim> CastedRegType;
        const CastedRegType* pCastedReg = dynamic_cast<const CastedRegType*>(m_spRegistration.GetPointer());

        if (!pCastedReg)
        {
            mapDefaultExceptionMacro(<< "Error. Cannot map point. Registration has invalid dimension. Point: " << inPoint);
        }

        result = pCastedReg->mapPoint(tempInP,tempOutP);
        if (result)
          {
            outPoint.CastFrom(tempOutP);
          }

      }

    return result;
  };

  /**
   * \brief Maps a point from target space to moving space using the inverse mapping kernel.
   *
   * \tparam VMovingDim Dimensionality of the moving space point.
   * \tparam VTargetDim Dimensionality of the target space point.
   * \param[in] inPoint The point in target space to map.
   * \param[out] outPoint The mapped point in moving space (only valid if return is true).
   * \return True if the mapping was successful, false otherwise (e.g. dimension mismatch).
   * \pre A valid registration instance must be set.
   * \note The operation may fail if the registration dimensions do not match the point dimensions.
   * \throw mapDefaultException if the registration pointer is null or has invalid dimensions.
   */
  template <unsigned int VMovingDim, unsigned int VTargetDim>
  bool MapPointInverse(const ::itk::Point<mitk::ScalarType,VTargetDim> & inPoint, ::itk::Point<mitk::ScalarType,VMovingDim> & outPoint) const
{
  typedef typename ::map::core::continuous::Elements<VMovingDim>::PointType MAPMovingPointType;
  typedef typename ::map::core::continuous::Elements<VTargetDim>::PointType MAPTargetPointType;

  if (m_spRegistration.IsNull())
  {
      mapDefaultExceptionMacro(<< "Error. Cannot map point. Wrapper points to invalid registration (nullptr). Point: " << inPoint);
  }

  bool result = false;

  if ((this->GetMovingDimensions() == VMovingDim)&&(this->GetTargetDimensions() == VTargetDim))
    {
      MAPTargetPointType tempInP;
      MAPMovingPointType tempOutP;
      tempInP.CastFrom(inPoint);

      typedef ::map::core::Registration<VMovingDim,VTargetDim> CastedRegType;
      const CastedRegType* pCastedReg = dynamic_cast<const CastedRegType*>(m_spRegistration.GetPointer());

      if (!pCastedReg)
      {
          mapDefaultExceptionMacro(<< "Error. Cannot map point. Registration has invalid dimension. Point: " << inPoint);
      }

      result = pCastedReg->mapPointInverse(tempInP,tempOutP);
      if (result)
        {
          outPoint.CastFrom(tempOutP);
        }

    }

  return result;
};

  /**
   * \brief Returns the field representation of the direct mapping kernel as a Geometry3D.
   *
   * The field representation defines the part of the moving space that is guaranteed to
   * be mapped by the direct mapping kernel. This method converts the internal MatchPoint
   * field representation descriptor into a mitk::Geometry3D.
   *
   * \return A Geometry3D describing the supported registration space in the moving domain.
   *         Returns a default-constructed Geometry3D if the direct kernel is global (unlimited).
   * \pre A valid registration instance must be set.
   */
  mitk::Geometry3D GetDirectFieldRepresentation() const;

  /**
   * \brief Returns the field representation of the inverse mapping kernel as a Geometry3D.
   *
   * The field representation defines the part of the target space that is guaranteed to
   * be mapped by the inverse mapping kernel. This method converts the internal MatchPoint
   * field representation descriptor into a mitk::Geometry3D.
   *
   * \return A Geometry3D describing the supported registration space in the target domain.
   *         Returns a default-constructed Geometry3D if the inverse kernel is global (unlimited).
   * \pre A valid registration instance must be set.
   */
  mitk::Geometry3D GetInverseFieldRepresentation() const;

  /**
   * \brief Forces the direct mapping kernel to precompute its deformation field.
   *
   * This is useful for lazy field-based kernels that defer computation until first use.
   * Calling this method ensures the field is computed immediately.
   *
   * \pre A valid registration instance must be set.
   */
  void PrecomputeDirectMapping();

  /**
   * \brief Forces the inverse mapping kernel to precompute its deformation field.
   *
   * This is useful for lazy field-based kernels that defer computation until first use.
   * Calling this method ensures the field is computed immediately.
   *
   * \pre A valid registration instance must be set.
   */
  void PrecomputeInverseMapping();

  /**
   * \brief Returns a mutable pointer to the wrapped MatchPoint registration.
   * \return Pointer to the internal RegistrationBase instance.
   */
  ::map::core::RegistrationBase* GetRegistration();

  /**
   * \brief Returns a const pointer to the wrapped MatchPoint registration.
   * \return Const pointer to the internal RegistrationBase instance.
   */
  const ::map::core::RegistrationBase* GetRegistration() const;

protected:
    void PrintSelf (std::ostream &os, itk::Indent indent) const override;

    MAPRegistrationWrapper(::map::core::RegistrationBase* registration);
    ~MAPRegistrationWrapper() override;

    void SetUID(const UIDType& uid) override;

    ::map::core::RegistrationBase::Pointer m_spRegistration;

private:

    MAPRegistrationWrapper& operator = (const MAPRegistrationWrapper&);
    MAPRegistrationWrapper(const MAPRegistrationWrapper&);

};

}

#endif
