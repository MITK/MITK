/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneProposer_h
#define mitkPlaneProposer_h

#include <MitkSurfaceInterpolationExports.h>

#include <mitkPoint.h>
#include <mitkVector.h>

#include <itkSmartPointer.h>

#include <array>
#include <vector>

#include <mitkDataStorage.h>

namespace mitk
{
  class SliceNavigationController;
  class UnstructuredGrid;

  /**
   * @brief The PlaneProposer creates a new plane based on an input point cloud
   *
   * The proposal is either created by using the lease squares in order to fit
   * a plane to the provided point cloud or by using the centroid of three clusters
   *
   * If less than three clusters are provided, least squares is chosen automatically.
   * If the centroid method is chosed either the three biggest clusters are chosen by
   * default. If the users sets PlaneProposer::SetUseDistances(true) the three clusters
   * with the biggerst mean distance of all points are chosen. The latter requires the
   * distances to be set as PointData scalar to the underlying VTK object.
   *
   * The user can either take the plane information or he can set a mitk::SliceNavigationController
   * which will be used to automatically rotate to the suggested position.
   */
  class MITKSURFACEINTERPOLATION_EXPORT PlaneProposer
  {
  public:
    /**
     * @brief Encapsulates the geometrical information needed to descripe a PlaneInfo
     *
     * normal = the normal of the plane
     * x,y = the axes of the PlaneInfo
     */
    struct PlaneInfo
    {
      mitk::Vector3D normal;
      mitk::Vector3D x;
      mitk::Vector3D y;
      mitk::Point3D pointOnPlane;
    };

    PlaneProposer();
    ~PlaneProposer();

    void SetUnstructuredGrids(std::vector<itk::SmartPointer<mitk::UnstructuredGrid>> &grids);

    /**
     * @brief If true, the three clusters with the biggest mean distances are used for plane proposal
     * Required the distance for each point to be set in PointData scalars
     */
    void SetUseDistances(bool);

    /**
     * @brief Tells the proposer to use least squares method for plane creating
     *
     * This will eb chosen automatically if less than three point clusters are provided
     */
    void SetUseLeastSquares(bool);

    /**
     * @brief Sets the number of the clusters to be used for plane creation (default=3)
     */
    void SetNumberOfClustersToUse(unsigned int);
    void SetSliceNavigationController(itk::SmartPointer<mitk::SliceNavigationController> &snc);

    /**
     * @brief Creates the actual plane proposal
     *
     * Is less than three clusters are provide the least squares method will be chosen automatically
     * The result will either be executed on a mitk::SliceNavigationController if provided or can be
     * retrieved by calling mitk::PlaneProposer::GetProposedPlaneInfo()
     */
    void CreatePlaneInfo();

    PlaneInfo GetProposedPlaneInfo();

    std::array<std::array<double, 3>, 3> GetCentroids();

  private:
    PlaneProposer(const PlaneProposer &);            // not implemented on purpose
    PlaneProposer &operator=(const PlaneProposer &); // not implemented on purpose

    /**
     * @brief Creates a plane suggestion based on the cluster centriods
     */
    PlaneInfo CreatePlaneByCentroids(const std::vector<std::pair<int, int>> &sizeIDs,
                                     const std::vector<std::pair<double, int>> &avgDistances);

    /**
     * @brief Creates a plane suggestion based on the least squares
     */
    PlaneInfo CreatePlaneByLeastSquares(const std::vector<std::pair<int, int>> &sizeIDs,
                                        const std::vector<std::pair<double, int>> &avgDistances);

    std::vector<itk::SmartPointer<mitk::UnstructuredGrid>> m_Grids;
    bool m_UseDistances;
    bool m_UseLeastSquares;
    unsigned int m_NumberOfClustersToUse;
    std::array<std::array<double, 3>, 3> m_Centroids;
    itk::SmartPointer<mitk::SliceNavigationController> m_SNC;
    PlaneInfo m_ProposedPlaneInfo;
  };

} // namespace

#endif
