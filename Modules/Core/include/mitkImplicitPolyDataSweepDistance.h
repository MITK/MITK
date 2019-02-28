#pragma once

#include <array>
#include <vector>

#include <vtkCommonTransformsModule.h> // For export macro
#include <vtkImplicitPolyDataDistance.h>
#include <vtkPoints.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkLine.h>
#include <vtkPolygon.h>
#include <vtkDoubleArray.h>
#include <vtkBoundingBox.h>

#include <MitkCoreExports.h>

class vtkMatrix4x4;
namespace mitk
{
class MITKCORE_EXPORT ImplicitPolyDataSweepDistance : public vtkImplicitFunction
{
public:
  static ImplicitPolyDataSweepDistance *New();
  vtkTypeMacro(ImplicitPolyDataSweepDistance, vtkImplicitPolyDataDistance);


  // Description:
  // Evaluate plane equation of nearest triangle to point x[3].
  virtual double EvaluateFunction(double x[3]) override;
  void SetInputPolyPoints(vtkPoints*poly);
  void setViewportMatrix(vtkMatrix4x4* matrix);

  // Description:
  // Evaluate function gradient of nearest triangle to point x[3].
  virtual void EvaluateGradient(double x[3], double g[3]) override;

protected:
  ImplicitPolyDataSweepDistance();
  ~ImplicitPolyDataSweepDistance();


private:
  ImplicitPolyDataSweepDistance(const ImplicitPolyDataSweepDistance&) = delete;
  void operator=(const ImplicitPolyDataSweepDistance&) = delete;

  int isLeft(double* P0, double* P1, double* P2);
  int getWindingNumber(double* P, double* V, double* V2);
  void clearPoints();

  void worldToView(double &x, double &y, double &z);

  double m_ViewportMatrix[16];

  // For performance reasons, store points as vector of pointers
  // Pointers are actually double[3] arrays that are created and destroyed dynamically
  std::vector<double*> m_Points;
  long m_PointsCount;
};
}

