/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUnstructuredGridClusteringFilter_h
#define mitkUnstructuredGridClusteringFilter_h

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
  * @brief This filter uses the DBSCAN algorithm for clustering an
  * mitk::UnstructuredGrid. "MinPts" defines the number of neighbours which are
  * required to be a kernel point if a point is in range of a kernel point
  * but hasnt enough neighbours this point is added to the cluster but is a
  * density reachable point and the cluster ends at this point. "eps" is the
  * range in which the neighbours are searched. If "Meshing" is set the
  * clusteres UnstructuredGrid is meshed and visible in 2D renderwindows.
  *
  * DBSCAN algorithm:
  *
  *     DBSCAN(D, eps, MinPts)
  *     C = 0
  *     for each unvisited point P in dataset D
  *       mark P as visited
  *       N = D.regionQuery(P, eps)
  *       if sizeof(N) < MinPts
  *         mark P as NOISE
  *       else
  *         C = next cluster
  *         expandCluster(P, N, C, eps, MinPts)
  *
  *     expandCluster(P, N, C, eps, MinPts)
  *       add P to cluster C
  *       for each point P' in N
  *         if P' is not visited
  *           mark P' as visited
  *           N' = D.regionQuery(P', eps)
  *           if sizeof(N') >= MinPts
  *             N = N joined with N'
  *         if P' is not yet member of any cluster
  *           add P' to cluster C
  */

  class MITKALGORITHMSEXT_EXPORT UnstructuredGridClusteringFilter : public UnstructuredGridToUnstructuredGridFilter
  {
  public:
    mitkClassMacro(UnstructuredGridClusteringFilter, UnstructuredGridToUnstructuredGridFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Sets the distance for the neighbour search */
      itkSetMacro(eps, double);
      itkGetMacro(eps, double);

      /** Sets the number of required neighbours */
      itkSetMacro(MinPts, int);
      itkGetMacro(MinPts, int);

      /** If activated the clusteres UnstructuredGrid is meshed */
      itkSetMacro(Meshing, bool);

      /** Returns all clusters as UnstructuredGrids which were found */
      virtual std::vector<mitk::UnstructuredGrid::Pointer> GetAllClusters();

    /** Returns the number of the clusters which were found */
    virtual int GetNumberOfFoundClusters();

  protected:
    /** Constructor */
    UnstructuredGridClusteringFilter();

    /** Destructor */
    ~UnstructuredGridClusteringFilter() override;

    /** Defines the output of the filter */
    void GenerateOutputInformation() override;

    /** Is called by the Update() method */
    void GenerateData() override;

  private:
    /** Used for the DBSCAN algorithm to expand a cluster and add more points to it */
    void ExpandCluster(int id, vtkIdList *pointIDs, vtkPoints *cluster, vtkPoints *inpPoints);

    /** The result main Cluster */
    mitk::UnstructuredGrid::Pointer m_UnstructGrid;

    /** All clusters which were found */
    std::vector<vtkSmartPointer<vtkPoints>> m_Clusters;

    /** The distances of the points from the input UnstructuredGrid*/
    std::vector<vtkSmartPointer<vtkDoubleArray>> m_DistanceArrays;

    /** The range for the neighbout search */
    double m_eps;

    /** The number of the required neighbours */
    int m_MinPts;

    /** Activates the meshing for the UnstructuredGrid clusters*/
    bool m_Meshing;

    /** If its activated the distance of the clusters is used instead of the
    * size */
    bool m_DistCalc;
  };

} // namespace mitk

#endif
