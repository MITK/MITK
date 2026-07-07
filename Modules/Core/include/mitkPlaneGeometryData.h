/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneGeometryData_h
#define mitkPlaneGeometryData_h

#include <mitkBaseData.h>
#include <mitkGeometryData.h>
#include <mitkPlaneGeometry.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Data class containing PlaneGeometry objects.
   *
   * Wraps a PlaneGeometry as a GeometryData so it can be stored in
   * a DataNode and managed by the DataStorage.
   *
   * \ingroup Geometry
   * \sa PlaneGeometry
   * \sa GeometryData
   */
  class MITKCORE_EXPORT PlaneGeometryData : public GeometryData
  {
  public:
    mitkClassMacro(PlaneGeometryData, GeometryData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Set the reference to a PlaneGeometry that is stored by the object.
     *
     * \warning Accepts only instances of PlaneGeometry or sub-classes.
     * Throws an exception if a non-PlaneGeometry is passed.
     *
     * \param geometry The geometry to store. Must be a PlaneGeometry.
     */
    void SetGeometry(mitk::BaseGeometry *geometry) override;

    /**
     * \brief Set the reference to the PlaneGeometry that is stored by the object.
     * \param geometry2d The PlaneGeometry to store.
     */
    virtual void SetPlaneGeometry(mitk::PlaneGeometry *geometry2d);

    /**
     * \brief Get the reference to the PlaneGeometry that is stored by the object.
     * \return Pointer to the stored PlaneGeometry, or nullptr if none is set.
     */
    virtual mitk::PlaneGeometry *GetPlaneGeometry() const { return static_cast<mitk::PlaneGeometry *>(GetGeometry()); };

    /** \brief Update output information from the stored geometry. */
    void UpdateOutputInformation() override;

    /** \brief Set the requested region to the largest possible region. */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Check if the requested region is outside of the buffered region.
     * \return true if no PlaneGeometry is set, false otherwise.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Verify the requested region.
     * \return false if no PlaneGeometry is set, true otherwise.
     */
    bool VerifyRequestedRegion() override;

    /** \brief Set the requested region from the given data object. */
    void SetRequestedRegion(const itk::DataObject *data) override;

    /** \brief Copy information from the given data object. */
    void CopyInformation(const itk::DataObject *data) override;

  protected:
    PlaneGeometryData();

    ~PlaneGeometryData() override;
  };
} // namespace mitk
#endif
