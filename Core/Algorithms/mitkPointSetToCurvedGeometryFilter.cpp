#include "mitkPointSetToCurvedGeometryFilter.h"

mitk::PointSetToCurvedGeometryFilter::PointSetToCurvedGeometryFilter()
{
  m_ProjectionMode = XZPlane;
  m_XYPlane = mitk::PlaneGeometry::New();
  m_XZPlane = mitk::PlaneGeometry::New();
  m_YZPlane = mitk::PlaneGeometry::New();
  m_XYPlane->InitializeStandardPlane();
  m_PCAPlaneCalculator = mitk::PlaneFit::New();
  m_ImageToBeMapped = NULL;
}
  
mitk::PointSetToCurvedGeometryFilter::~PointSetToCurvedGeometryFilter()
{

}

void mitk::PointSetToCurvedGeometryFilter::GenerateOutputInformation()
{
  
}

void mitk::PointSetToCurvedGeometryFilter::GenerateData()
{
  
}

static void mitk::PointSetToCurvedGeometryFilter::SetDefaultCurvedGeometryProperties()
{
  
}
