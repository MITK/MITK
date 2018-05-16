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
#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
/*!
  \brief MAPRegistrationWrapper
  Wrapper class to allow the handling of MatchPoint registration objects as mitk data (e.g. in the data explorer).
*/
class MITKMATCHPOINTREGISTRATION_EXPORT MAPRegistrationWrapper: public mitk::BaseData
{
public:

  mitkClassMacro( MAPRegistrationWrapper, BaseData );

  itkNewMacro( Self );

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

  /*! @brief Gets the number of moving dimensions
  @pre valid registration instance must be set.
  */
  virtual unsigned int GetMovingDimensions() const;

  /*! @brief Gets the number of target dimensions
  @pre valid registration instance must be set.
  */
  virtual unsigned int GetTargetDimensions() const;

  /*! typedefs used for the TagMap
  */
  typedef ::map::core::RegistrationBase::TagType TagType;
  typedef ::map::core::RegistrationBase::ValueType ValueType;
  typedef ::map::core::RegistrationBase::TagMapType TagMapType;

  /*! @brief returns the tags associated with this registration
  @pre valid registration instance must be set.
  @return a TagMapType containing tags
  */
  const TagMapType& GetTags() const;

  /*! @brief returns the tag value for a specific tag
  @pre valid registration instance must be set.
  @return the success of the operation
  */
  bool GetTagValue(const TagType & tag, ValueType & value) const;

  /*! Indicates
  @pre valid registration instance must be set.
  @return is the target representation limited
  @retval true if target representation is limited. Thus it is not guaranteed that all inverse mapping operations
  will succeed. Transformation(inverse kernel) covers only a part of the target space).
  @retval false if target representation is not limited. Thus it is guaranteed that all inverse mapping operations
  will succeed.
  */
  bool HasLimitedTargetRepresentation() const;

  /*!
  @pre valid registration instance must be set.
  @return is the moving representation limited
  @retval true if moving representation is limited. Thus it is not guaranteed that all direct mapping operations
  will succeed. Transformation(direct kernel) covers only a part of the moving space).
  @retval false if moving representation is not limited. Thus it is guaranteed that all direct mapping operations
  will succeed.
  */
  bool HasLimitedMovingRepresentation() const;

  /*! Helper function that maps a mitk point (of arbitrary dimension) from moving space to target space.
  @remarks The operation might fail, if the moving and target dimension of the registration
  is not equal to the dimensionality of the passed points.
  @pre valid registration instance must be set.
  @param inPoint Reference pointer to a MovingPointType
  @param outPoint pointer to a TargetPointType
  @return success of operation.
  @pre direct mapping kernel must be defined
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

  /*! Helper function that maps a mitk point (of arbitrary dimension) from target space to moving space
  @remarks The operation might faile, if the moving and target dimension of the registration
  is not equal to the dimensionalities of the passed points.
  @pre valid registration instance must be set.
  @param inPoint pointer to a TargetPointType
  @param outPoint pointer to a MovingPointType
  @return success of operation
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

  /*! returns the direct FieldRepresentationDescriptor which defines the part
  of the moving space that is guaranteed to be mapped by the direct mapping kernel.
  This member converts the internal MatchPoint type into a mitk::Geometry3D.
  @pre valid registration instance must be set.
  @return smart pointer to a FieldRepresentationDescriptor for the supported registration space in the moving domain.
  May be null if the direct registration kernel is global and thus not limited.
  If there is a limitation, the retun value is not nullptr.
  @retval nullptr no field representation set/requested by the creating registration algorithm.
  */
  mitk::Geometry3D GetDirectFieldRepresentation() const;

  /*! returns the inverse FieldRepresentationDescriptor which defines the part
  of the target space that is guaranteed to be mapped by the inverse mapping kernel.
  This member converts the internal MatchPoint type into a mitk::Geometry3D.
  @pre valid registration instance must be set.
  @return a const FieldRepresentationDescriptor for the supported registration space in the target domain.
  May be null if the inverse registration kernel is global and thus not limited.
  If there is a limitation, the retun value is not nullptr.
  @retval nullptr no field representation set/requested by the creating registration algorithm.
  */
  mitk::Geometry3D GetInverseFieldRepresentation() const;

  /*! forces kernel to precompute, even if it is a LazyFieldKernel
  @pre valid registration instance must be set.
  @todo der LazyFieldBasedRegistrationKernel muss dann die stong guarantee erfllen beim erzeugen des feldes ansonsten
  ist die garantie dieser methode nicht erfllbar. noch berprfen
  */
  void PrecomputeDirectMapping();

  /*! forces kernel to precompute, even if it is a LazyFieldKernel
  @pre valid registration instance must be set.
  @todo der LazyFieldBasedRegistrationKernel muss dann die stong guarantee erfllen beim erzeugen des feldes ansonsten
  ist die garantie dieser methode nicht erfllbar. noch berprfen
  */
  void PrecomputeInverseMapping();

  ::map::core::RegistrationBase* GetRegistration();
  const ::map::core::RegistrationBase* GetRegistration() const;

  void SetRegistration(::map::core::RegistrationBase* pReg);

protected:
    void PrintSelf (std::ostream &os, itk::Indent indent) const override;

    MAPRegistrationWrapper();
    ~MAPRegistrationWrapper() override;

    ::map::core::RegistrationBase::Pointer m_spRegistration;

private:

    MAPRegistrationWrapper& operator = (const MAPRegistrationWrapper&);
    MAPRegistrationWrapper(const MAPRegistrationWrapper&);

};

}

#endif

