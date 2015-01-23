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

#include <mitkCommon.h>

#include <mitkUnstructuredGrid.h>
#include <mitkUnstructuredGridToUnstructuredGridFilter.h>
#include <mitkGeometryData.h>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>


namespace mitk {

  class MitkSurfaceInterpolation_EXPORT ClusteredPlaneSuggestionFilter
      : public UnstructuredGridToUnstructuredGridFilter
  {
    public:

      mitkClassMacro(ClusteredPlaneSuggestionFilter,
                     UnstructuredGridToUnstructuredGridFilter)

      itkFactorylessNewMacro(Self)

      itkCloneMacro(Self)

      itkGetMacro(GeoData, mitk::GeometryData::Pointer)
      itkGetMacro(Clusters, std::vector< mitk::UnstructuredGrid::Pointer >)
      itkGetMacro(Meshing, bool)
      itkGetMacro(MinPts, int)
      itkGetMacro(Eps, double)

      itkSetMacro(Meshing, bool)
      itkSetMacro(MinPts, int)
      itkSetMacro(Eps, double)


    protected:

      ClusteredPlaneSuggestionFilter();

      virtual ~ClusteredPlaneSuggestionFilter();

      virtual void GenerateData();

      virtual void GenerateOutputInformation();


  private:

      mitk::GeometryData::Pointer m_GeoData;

      std::vector< mitk::UnstructuredGrid::Pointer > m_Clusters;

      mitk::UnstructuredGrid::Pointer m_MainCluster;

      bool m_Meshing;

      int m_MinPts;

      double m_Eps;

  };

} // namespace mitk

#endif //_MITKCLUSTEREDPLANESUGGESTIONFILTER_h__


