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

#ifndef _MITKCLUSTEREDPLANESUGGESTIONFILTER_h__
#define _MITKCLUSTEREDPLANESUGGESTIONFILTER_h__

#include <MitkSurfaceInterpolationExports.h>

#include <mitkGeometryData.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>


namespace mitk {

   /**
   * @brief Clustering an UnstructuredGrid and calculating a Plane through it.
   *
   * The output is the biggest found cluster but you can get all clusters in a
   * std::vector represented by vtkPoints. Use GetClusters() the get the vector.
   * With GetGeoData() you get the calculated geometry as a mitk::GeometryData.
   * Internally the mitk::UnstructuredGridClusteringFilter is used for
   * clustering and after the mitk::PlaneFit for calculating the plane.
   * The parameters m_Meshing (Set/GetMeshing()), m_MinPts (Set/GetMinPts()) and
   * m_Eps (Set/GetEps()) are used for the UnstructuredGridClusteringFilter.
   */
  class MITKSURFACEINTERPOLATION_EXPORT ClusteredPlaneSuggestionFilter
      : public UnstructuredGridToUnstructuredGridFilter
  {
    public:

      mitkClassMacro(ClusteredPlaneSuggestionFilter,
                     UnstructuredGridToUnstructuredGridFilter)

      itkFactorylessNewMacro(Self)

      itkCloneMacro(Self)

      /** Returns the geometry of the calculated plane from mitk::PlaneFit */
      itkGetMacro(GeoData, mitk::GeometryData::Pointer)

      /** Returns all clusters which were found by the clustering filter */
      itkGetMacro(Clusters, std::vector< mitk::UnstructuredGrid::Pointer >)

      /** Activate the meshing function for the returned clusters. The meshing
      * is needed to see the result in the 2D-renderwindows */
      itkGetMacro(Meshing, bool)
      itkSetMacro(Meshing, bool)

      /** Minimal points which have to be located in the neighbourhood to define
      * the point as a core point. For more information see DBSCAN algorithm */
      itkGetMacro(MinPts, int)
      itkSetMacro(MinPts, int)

      /** The range/neighbourhood for clustering the points. For more
      * information see DBSCAN algorithm */
      itkGetMacro(Eps, double)
      itkSetMacro(Eps, double)

      /** Setting to true, uses the distances of the clusters otherwise the
      * the size of the clusters is used */
      itkSetMacro(UseDistances, bool)

      /** Sets the number of clusters which are used for the plane suggestion
      * if the number of found clusters is lower than the "NumberOfUsedClusters"
      * all found clusters are used */
      itkSetMacro(NumberOfUsedClusters, unsigned int)


    protected:

      /** Constructor */
      ClusteredPlaneSuggestionFilter();

      /** Destructor */
      virtual ~ClusteredPlaneSuggestionFilter();

      /** Is called by the Update() method of the filter */
      virtual void GenerateData();

      /** Defines the output of the filter */
      virtual void GenerateOutputInformation();


  private:

      /** The geometry of the calculated plane */
      mitk::GeometryData::Pointer m_GeoData;

      /** The vector which holds all found clusters */
      std::vector< mitk::UnstructuredGrid::Pointer > m_Clusters;

      /** The biggest found cluster - the output */
      mitk::UnstructuredGrid::Pointer m_MainCluster;

      /** Connect the points to meshes. Required for 2D rendering */
      bool m_Meshing;

      /** Number of points required to define a core point */
      int m_MinPts;

      /** The distance/neighbourhood for clustering */
      double m_Eps;

      /** Decides to use the distances or the size */
      bool m_UseDistances;

      /** The number of clusters which are used for the plane suggestion */
      unsigned int m_NumberOfUsedClusters;

  };

} // namespace mitk

#endif //_MITKCLUSTEREDPLANESUGGESTIONFILTER_h__
