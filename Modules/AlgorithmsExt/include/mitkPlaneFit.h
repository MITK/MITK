/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneFit_h
#define mitkPlaneFit_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkGeometryDataSource.h>
#include <mitkPlaneGeometry.h>
#include <mitkPointSet.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  /**
   * \brief Fits a plane to a point set using least-squares orthogonal distance regression.
   *
   * This filter computes the best-fit plane for a given mitk::PointSet using
   * Singular Value Decomposition (SVD). For each time step the centroid of the
   * points and the normal of the least-squares plane are computed. The output
   * is a GeometryData containing PlaneGeometry instances.
   *
   * The algorithm requires at least 3 points per time step. Based on the LSPLANE
   * method by I. M. Smith (2002).
   *
   * \sa GeometryDataSource
   * \sa PlaneGeometry
   * \sa PointSet
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT PlaneFit : public GeometryDataSource
  {
  public:
    mitkClassMacro(PlaneFit, GeometryDataSource);
    itkNewMacro(Self);

    typedef mitk::PointSet::PointDataType PointDataType;
    typedef mitk::PointSet::PointDataIterator PointDataIterator;

    /** \brief Initialize the output information (time geometry, plane geometries). */
    void GenerateOutputInformation() override;

    /** \brief Compute the best-fit plane for each time step. */
    void GenerateData() override;

    /**
     * \brief Get the input point set.
     * \return Const pointer to the input PointSet, or nullptr if not set.
     */
    const mitk::PointSet *GetInput();

    /**
     * \brief Set the input point set.
     * \param[in] ps The PointSet to fit a plane to.
     * \pre The point set should contain at least 3 points per time step.
     */
    using mitk::GeometryDataSource::SetInput;
    virtual void SetInput(const mitk::PointSet *ps);

    /**
     * \brief Get the centroid (center of gravity) of the point set.
     * \param[in] t The time step (default: 0).
     * \return Const reference to the centroid point.
     */
    virtual const mitk::Point3D &GetCentroid(int t = 0) const;

    /**
     * \brief Get the PlaneGeometry representing the best-fit plane.
     * \param[in] t The time step (default: 0).
     * \return Smart pointer to the PlaneGeometry.
     */
    virtual mitk::PlaneGeometry::Pointer GetPlaneGeometry(int t = 0);

    /**
     * \brief Get the normal vector of the best-fit plane.
     * \param[in] t The time step (default: 0).
     * \return Const reference to the normal vector.
     */
    virtual const mitk::Vector3D &GetPlaneNormal(int t = 0) const;

  protected:
    PlaneFit();
    ~PlaneFit() override;

    /*! Calculates the centroid of the point set.
     * the center of gravity is calculated  through the mean value of the whole point set
     */
    void CalculateCentroid(int t = 0);

    /*! working with an SVD algorithm form matrix dataM.
     * ITK supplies the vnl_svd to solve an plan fit eigentvector problem
     * points are processed in the SVD matrix. The normal vector is the
     * singular vector of dataM corresponding to its smalest singular value.
     * The method uses VNL library from ITK and at least the method nullvector()
     * to extract the normalvector.
     */
    void ProcessPointSet(int t = 0);

    /*! Initialize Plane and configuration.
     *
     */
    void InitializePlane(int t = 0);

  private:
    /*!keeps a copy of the pointset.*/
    const mitk::PointSet *m_PointSet;

    /* output object - a time sliced geometry.*/
    mitk::TimeGeometry::Pointer m_TimeGeometry;

    std::vector<mitk::PlaneGeometry::Pointer> m_Planes;

    /*! the calculatet center point of all points in the point set.*/
    std::vector<mitk::Point3D> m_Centroids;

    /* the normal vector to describe a plane geometry.*/
    std::vector<mitk::Vector3D> m_PlaneVectors;
  };
} // namespace mitk
#endif
