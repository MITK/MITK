#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkCellLocator.h>
#include <vtkGenericCell.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

#include <vtkTriangleFilter.h>
#include <vtkSmartPointer.h>

#include <vtkMath.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>


#include "mitkImplicitPolyDataSweepDistance.h"
namespace mitk {
vtkStandardNewMacro(ImplicitPolyDataSweepDistance);

ImplicitPolyDataSweepDistance::ImplicitPolyDataSweepDistance()
  : m_PointsCount(0)
{
}

ImplicitPolyDataSweepDistance::~ImplicitPolyDataSweepDistance()
{
  clearPoints();
}

//-----------------------------------------------------------------------------
void ImplicitPolyDataSweepDistance::SetInputPolyPoints(vtkPoints* pd)
{
  clearPoints();
  m_PointsCount = pd->GetNumberOfPoints();

  // Transform points to view's coordinate system
  for (int i = 0; i < m_PointsCount; i++) {
    double* point = new double[3];
    pd->GetPoint(i, point);
    worldToView(point[0], point[1], point[2]);
    m_Points.push_back(point);
  }
}

int ImplicitPolyDataSweepDistance::isLeft(double* P0, double* P1, double* P2)
{
  return (  (P1[0] - P0[0]) * (P2[1] - P0[1]) -
            (P2[0] - P0[0]) * (P1[1] - P0[1])
         );
}

int ImplicitPolyDataSweepDistance::getWindingNumber(double* P, double* V, double* V2)
{
  int wn = 0;

  if (V[1] <= P[1]) {          // start y <= P.y
    if (V2[1] > P[1]) {     // an upward crossing
      if (isLeft(V, V2, P) > 0) { // P left of  edge
        ++wn;            // have  a valid up intersect
      }
    }
  } else {                        // start y > P.y (no test needed)
    if (V2[1] <= P[1]) {     // a downward crossing
      if (isLeft(V, V2, P) < 0) {  // P right of  edge
        --wn;            // have  a valid down intersect
      }
    }
  }

  return wn;
}
//-----------------------------------------------------------------------------
double ImplicitPolyDataSweepDistance::EvaluateFunction(double x[3])
{
  // Transform voxel to view's coordinate system, just as contour points
  worldToView(x[0], x[1], x[2]);

  // Count winding number: http://geomalgorithms.com/a03-_inclusion.html
  int wn = 0;
  double* P0;
  double* P1;
  for (int i = 0; i < m_PointsCount - 1; i++) {
    P0 = m_Points[i];
    P1 = m_Points[i + 1];

    wn += getWindingNumber(x, P0, P1);
  }

  P0 = m_Points[m_PointsCount - 1];
  P1 = m_Points[0];

  wn += getWindingNumber(x, P0, P1);

  if (wn % 2 == 0) {
    return 1.0;
  } else {
    return -1.0;
  }
}

//-----------------------------------------------------------------------------
void ImplicitPolyDataSweepDistance::EvaluateGradient(double x[3], double g[3])
{
}

void ImplicitPolyDataSweepDistance::clearPoints()
{
  for (double* p : m_Points) {
    delete []p;
  }
  m_Points.clear();
}

void ImplicitPolyDataSweepDistance::setViewportMatrix(vtkMatrix4x4* matrix)
{
  vtkMatrix4x4::DeepCopy(m_ViewportMatrix, matrix);
}

void ImplicitPolyDataSweepDistance::worldToView(double &x, double &y, double &z)
{
  double view[4];

  view[0] = x*m_ViewportMatrix[0] + y*m_ViewportMatrix[1] + z*m_ViewportMatrix[2] + m_ViewportMatrix[3];
  view[1] = x*m_ViewportMatrix[4] + y*m_ViewportMatrix[5] + z*m_ViewportMatrix[6] + m_ViewportMatrix[7];
  view[2] = x*m_ViewportMatrix[8] + y*m_ViewportMatrix[9] + z*m_ViewportMatrix[10] + m_ViewportMatrix[11];
  view[3] = x*m_ViewportMatrix[12] + y*m_ViewportMatrix[13] + z*m_ViewportMatrix[14] + m_ViewportMatrix[15];

  const double roundingInsurance = 10000.0;
  if (view[3] != 0.0) {
    x = view[0] * roundingInsurance / view[3];
    y = view[1] * roundingInsurance / view[3];
    z = view[2] * roundingInsurance / view[3];
  }
}
}

