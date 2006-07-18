#include <mitkPlaneFit.h>
#include <mitkPlaneGeometry.h>
#include <vnl/algo/vnl_svd.h>
#include <vcl_iostream.h>


//  function [x0, a, d, normd] = lsplane(X)
//% ---------------------------------------------------------------------
//% LSPLANE.M   Least-squares plane (orthogonal distance
//%             regression).
//%
//% Version 1.0    
//% Last amended   I M Smith 27 May 2002. 
//% Created        I M Smith 08 Mar 2002
//% ---------------------------------------------------------------------
//% Input    
//% X        Array [x y z] where x = vector of x-coordinates, 
//%          y = vector of y-coordinates and z = vector of 
//%          z-coordinates. 
//%          Dimension: m x 3. 
//% 
//% Output   
//% x0       Centroid of the data = point on the best-fit plane.
//%          Dimension: 3 x 1. 
//% 
//% a        Direction cosines of the normal to the best-fit 
//%          plane. 
//%          Dimension: 3 x 1.
//% 
//% <Optional... 
//% d        Residuals. 
//%          Dimension: m x 1. 
//% 
//% normd    Norm of residual errors. 
//%          Dimension: 1 x 1. 
//% ...>
//%
//% [x0, a <, d, normd >] = lsplane(X)
//% ---------------------------------------------------------------------
//





mitk::PlaneFit::PlaneFit()
{
  m_Plane = mitk::PlaneGeometry::New();
}

mitk::PlaneFit::~PlaneFit(){}

void mitk::PlaneFit::GenerateOutputInformation()
{
	mitk::PointSet::ConstPointer input  = this->GetInput();
	mitk::GeometryData::Pointer output  = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

  output->SetGeometry( m_Plane );
}

void mitk::PlaneFit::GenerateData()
{
  int t_start = 0; // to be replaced by requested begninning time
  int t_end = 1; //first input
  //int t_end = this->GetNumberOfInputs(); //to be replaced by requested time period.
  
  for(int t = t_start; t< t_end; t++)
  {
    // set the timeslice globaly known.
    m_PointSet = static_cast<const mitk::PointSet * > (this->ProcessObject::GetInput( t ));
  
    if (CheckInput()) return ;

    GetCentroid();

    ProcessPointSet();

  }
}


bool mitk::PlaneFit::CheckInput()
{
  // check number of data points 
  if(m_PointSet->GetSize() < 3 )
  {
    itkDebugMacro(<<"mitkPlaneFit--> Faild initializing a plane because the are not enogh point");
    return false;

  }
  else 
    return true;
}

void mitk::PlaneFit::SetInput( const mitk::PointSet* ps )
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast< mitk::PointSet * >( ps ) );
}

const mitk::PointSet* mitk::PlaneFit::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }

  return static_cast<const mitk::PointSet * > (this->ProcessObject::GetInput(0) );
}


mitk::ScalarType* mitk::PlaneFit::GetCentroid(void)
{
  // calculate centroid
  if( m_PointSet == NULL ) return NULL;

  int ps_total = m_PointSet->GetSize();

  for(int i=0; i<ps_total; i++)
  {
     m_Centroid[0] = m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[0];
     m_Centroid[1] = m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[1];
     m_Centroid[2] = m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[2];
  }

  //mean of centroid
  m_Centroid[0] = m_Centroid[0]/ps_total;
  m_Centroid[1] = m_Centroid[1]/ps_total;
  m_Centroid[2] = m_Centroid[2]/ps_total;
 // m_Centroid = v;

	return m_Centroid;
}


void mitk::PlaneFit::ProcessPointSet()
{
// form matrix A of translated points
//  A = [(X(:, 1) - x0(1)) (X(:, 2) - x0(2)) (X(:, 3) - x0(3))];

  vnl_matrix<mitk::ScalarType> dataM( m_PointSet->GetSize(), 3);
  vnl_matrix<mitk::ScalarType> dataV( m_PointSet->GetSize(), 3);

  for(int p = 0; p < m_PointSet->GetSize(); p++)
  {
    dataM[p][0] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[0] - m_Centroid[0];
    dataM[p][1] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[1] - m_Centroid[1];
    dataM[p][2] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[2] - m_Centroid[2];

    dataV[p][0] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[0];
    dataV[p][1] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[1];
    dataV[p][2] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[2];
  }

  dataV.transpose();

  /*vnl_matrix<mitk::ScalarType> D*/ 
  //vnl_svd<mitk::ScalarType> svd(dataM, 0);
  vnl_svd<mitk::ScalarType> svd(dataM, 0);

 if (svd.singularities() > 1)
  vcl_cerr << "Warning: Singular matrix, condition = " << svd.well_condition() << vcl_endl;
  svd.solve(dataM);
//
// calculate the SVD of A
//  [U, S, V] = svd(A, 0);
}


mitk::ScalarType mitk::PlaneFit::GetPlaneVector()
{
// find the smallest singular value in S and extract from V the
// corresponding right singular vector
//  [s, i] = min(diag(S));
//  a = V(:, i);
  mitk::ScalarType v3 = 0.0;
  return v3;
}

int mitk::PlaneFit::GetPointDistance()
{
// calculate residual distances, if required  
//  if nargout > 2
//    d = U(:, i)*s;
//    normd = norm(d);
//  end
	return 0;
}
