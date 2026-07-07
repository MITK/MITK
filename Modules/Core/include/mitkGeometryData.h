/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryData_h
#define mitkGeometryData_h

#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief Data class that holds only a BaseGeometry without any specific data payload.
   *
   * GeometryData is a lightweight BaseData subclass that provides concrete
   * implementations of the pure-virtual pipeline methods defined in
   * BaseData / itk::DataObject.  It carries spatial geometry information
   * (via its inherited TimeGeometry / BaseGeometry) but stores no image,
   * surface, or other domain-specific data.
   *
   * Use this class when you need a first-class data object in the MITK
   * pipeline that represents only geometry (e.g., for storing a
   * coordinate system or a region of interest).
   *
   * \sa BaseData, BaseGeometry
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT GeometryData : public BaseData
  {
  public:
    mitkClassMacro(GeometryData, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Update the output information.
       *
       * Delegates to Superclass::UpdateOutputInformation().
       */
      void UpdateOutputInformation() override;

    /**
     * \brief Set the requested region to the largest possible region.
     *
     * No-op for GeometryData because there is no data region to manage.
     */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check whether the requested region is outside the buffered region.
     *
     * \return true if a geometry is set (data is buffered), false otherwise.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify that the requested region is valid.
     *
     * \return true if a geometry is set, false otherwise.
     */
    bool VerifyRequestedRegion() override;

    /**
     * \brief Set the requested region from another data object.
     *
     * No-op for GeometryData.
     *
     * \param[in] data The data object to copy the requested region from.
     */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /**
     * \brief Copy information from another data object.
     *
     * No-op for GeometryData.
     *
     * \param[in] data The data object to copy information from.
     */
    void CopyInformation(const itk::DataObject *data) override;

  protected:
    GeometryData();

    ~GeometryData() override;
  };

  /**
  * @brief Equal Compare two GeometryData objects for equality, returns true if found equal.
  * @ingroup MITKTestingAPI
  * @param rightHandSide GeometryData to compare.
  * @param leftHandSide GeometryData to compare.
  * @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
  * @param verbose Flag indicating if the method should give a detailed console output.
  * @return True if every comparison is true, false in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::GeometryData &leftHandSide,
                             const mitk::GeometryData &rightHandSide,
                             mitk::ScalarType eps,
                             bool verbose);

} // namespace mitk
#endif
