
#if !defined(MITK_PLANEFIT_H__INCLUDED_)
#define MITK_PLANEFIT_H__INCLUDED_

//#include <mitkCommon.h>
//#include <mitkBaseProcess.h>
#include <Vector3.h>
#include <mitkPointSet.h>
#include <mitkPlaneGeometry.h>
#include <mitkGeometry3D.h>
#include <mitkGeometryData.h>
#include "mitkGeometryDataSource.h"

namespace mitk {
	
//  kind regards to dr. math!
//	function [x0, a, d, normd] = lsplane(X)
// ---------------------------------------------------------------------
// LSPLANE.M   Least-squares plane (orthogonal distance
//             regression).
//
// Version 1.0    
// Last amended   I M Smith 27 May 2002. 
// Created        I M Smith 08 Mar 2002
// ---------------------------------------------------------------------
// Input    
// X        Array [x y z] where x = vector of x-coordinates, 
//          y = vector of y-coordinates and z = vector of 
//          z-coordinates. 
//          Dimension: m x 3. 
// 
// Output   
// x0       Centroid of the data = point on the best-fit plane.
//          Dimension: 3 x 1. 
// 
// a        Direction cosines of the normal to the best-fit 
//          plane. 
//          Dimension: 3 x 1.
// 
// <Optional... 
// d        Residuals. 
//          Dimension: m x 1. 
// 
// normd    Norm of residual errors. 
//          Dimension: 1 x 1. 
// ...>
//
// [x0, a <, d, normd >] = lsplane(X)
// ---------------------------------------------------------------------

class PlaneFit : public GeometryDataSource
{
public:

  mitkClassMacro( PlaneFit, GeometryDataSource);

  itkNewMacro(Self);

  
  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::PointSet *GetInput(void);

  virtual void SetInput(const mitk::PointSet* ps);

  // check number of data points 
  bool CheckInput();
  
  void SetPointSet( mitk::PointSet::Pointer ps );

  // calculate centroid
  mitk::ScalarType* GetCentroid();
  
  // form matrix A of translated points
  void ProcessPointSet();
 
  // find the smallest singular value in S and extract from V the
  // corresponding right singular vector 
  mitk::ScalarType GetPlaneVector();
  
  // calculate residual distances, if required  
  int GetPointDistance();

protected:
	PlaneFit();
	virtual ~PlaneFit();

private:

	const mitk::PointSet* m_PointSet;
  mitk::PlaneGeometry* m_Plane;
  mitk::ScalarType  m_Centroid[3];
	mitk::Vector3D* m_Vector3D;
};
}//namespace mitk
#endif //MITK_PLANFIT_INCLUDE_
