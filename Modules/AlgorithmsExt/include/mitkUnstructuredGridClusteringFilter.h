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

#ifndef _MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__
#define _MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__

#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>
#include <vtkIdList.h>
#include <vtkPoints.h>


namespace mitk {

   /**
   * @brief The UnstructuredGridClusteringFilter class
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

  class MITKALGORITHMSEXT_EXPORT UnstructuredGridClusteringFilter
      : public UnstructuredGridToUnstructuredGridFilter
  {
    public:

      mitkClassMacro(UnstructuredGridClusteringFilter, UnstructuredGridToUnstructuredGridFilter)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      itkSetMacro(eps, double)
      itkGetMacro(eps, double)

      itkSetMacro(MinPts, int)
      itkGetMacro(MinPts, int)

      itkSetMacro(Meshing, bool)

      virtual std::vector< mitk::UnstructuredGrid::Pointer> GetAllClusters();

      virtual int GetNumberOfFoundClusters();

      virtual void GenerateOutputInformation() override;

      virtual void GenerateData() override;

    protected:

      UnstructuredGridClusteringFilter();

      virtual ~UnstructuredGridClusteringFilter();

  private:

      void ExpandCluster(int id, vtkIdList* pointIDs, vtkPoints* cluster, vtkPoints *inpPoints);

      mitk::UnstructuredGrid::Pointer m_UnstructGrid;

      std::vector< vtkSmartPointer<vtkPoints> > m_Clusters;

      double m_eps;

      int m_MinPts;

      bool m_Meshing;

  };

} // namespace mitk

#endif //_MITKUNSTRUCTUREDGRIDCLUSTERINGFILTER_h__


