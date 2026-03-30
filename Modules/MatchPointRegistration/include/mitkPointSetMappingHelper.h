/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkPointSetMappingHelper_h
#define mitkPointSetMappingHelper_h

#include <mapRegistrationBase.h>
#include <mitkPointSet.h>

#include <mitkMAPRegistrationWrapper.h>

#include <MitkMatchPointRegistrationExports.h>

namespace mitk
{

  /**
   * \brief Namespace providing helper functions for mapping MITK point sets using MatchPoint registrations.
   *
   * \sa mitk::ImageMappingHelper, mitk::MAPRegistrationWrapper
   */
  namespace PointSetMappingHelper
  {
    /** \brief MatchPoint registration base type. */
    typedef ::map::core::RegistrationBase RegistrationType;
    /** \brief MITK wrapper type for MatchPoint registrations. */
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    /**
     * \brief Converts an MITK point set to the MatchPoint internal point set type.
     *
     * \param[in] mitkSet Pointer to the ITK point set data underlying the MITK PointSet.
     * \return Smart pointer to the converted MatchPoint internal point set.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ::map::core::continuous::Elements<3>::InternalPointSetType::Pointer ConvertPointSetMITKtoMAP(const mitk::PointSet::DataType* mitkSet);

    /**
     * \brief Maps a given input point set using a MatchPoint registration.
     *
     * \param[in] input Point set that should be mapped.
     * \param[in] registration Pointer to the MatchPoint registration to use.
     * \param[in] timeStep The time step to map (-1 maps all time steps, default). Other time steps are copied unmodified.
     * \param[in] throwOnMappingError If true, throws an exception when a point cannot be mapped.
     *            If false, unmappable points are transferred without mapping and tagged with \p errorPointValue.
     * \param[in] errorPointValue Point data value assigned to unmappable points (when not throwing).
     * \return Smart pointer to the resulting mapped point set.
     * \pre \p input must be valid.
     * \pre \p registration must be valid.
     * \pre \p timeStep must be a valid time step of \p input or -1.
     * \pre Dimensionality of the registration must match the point set (3D).
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ::mitk::PointSet::Pointer map(const ::mitk::PointSet* input, const RegistrationType* registration, int timeStep = -1,
      bool throwOnMappingError = true, const ::mitk::PointSet::PointDataType& errorPointValue = ::mitk::PointSet::PointDataType());

    /**
     * \brief Maps a given input point set using a MITK registration wrapper.
     * \overload
     */
    MITKMATCHPOINTREGISTRATION_EXPORT ::mitk::PointSet::Pointer map(const ::mitk::PointSet* input, const MITKRegistrationType* registration, int timeStep = -1,
      bool throwOnMappingError = true, const ::mitk::PointSet::PointDataType& errorPointValue = ::mitk::PointSet::PointDataType());
  }

}

#endif
