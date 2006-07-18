#include <mitkPlaneFit.h>
#include <mitkPlaneGeometry.h>
#include <vnl/algo/vnl_svd.h>
#include <vcl_iostream.h>


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
  // set the timeslice globaly known.
  m_PointSet = static_cast<const mitk::PointSet * > (this->ProcessObject::GetInput( 0 ));
  
  // check number of data points - less then 3points isn't enough
  if(m_PointSet->GetSize() < 3 ) return;

  CalculateCentroid();

  ProcessPointSet();

  InitializePlane();
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


void mitk::PlaneFit::CalculateCentroid()
{  
  if( m_PointSet == NULL ) return;

  int ps_total = m_PointSet->GetSize();
  m_Centroid[0]=m_Centroid[1]=m_Centroid[2]=0;

  // summ of all points
  for(int i=0; i<ps_total; i++)
  {
     m_Centroid[0] += m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[0];
     m_Centroid[1] += m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[1];
     m_Centroid[2] += m_PointSet->GetPointSet()->GetPoints()->GetElement(i)[2];
  }

  // calculation of centroid
  m_Centroid[0] = m_Centroid[0]/ps_total;
  m_Centroid[1] = m_Centroid[1]/ps_total;
  m_Centroid[2] = m_Centroid[2]/ps_total;
}


void mitk::PlaneFit::ProcessPointSet()
{
  if (m_PointSet == NULL ) return;

  // int matrix with POINTS x (X,Y,Z)
  vnl_matrix<mitk::ScalarType> dataM( m_PointSet->GetSize(), 3);

  //calculate point distance to centroid and inserting it in the matrix
  for(int p = 0; p < m_PointSet->GetSize(); p++)
  {
    dataM[p][0] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[0] - m_Centroid[0];
    dataM[p][1] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[1] - m_Centroid[1];
    dataM[p][2] = m_PointSet->GetPointSet()->GetPoints()->GetElement(p)[2] - m_Centroid[2];
  }

  // process the SVD (singular value decomposition) from ITK
  // the vector will be orderd 	descending 
  vnl_svd<mitk::ScalarType> svd(dataM, 0.0);
 
  // calculate the SVD of A
  vnl_vector<mitk::ScalarType> v = svd.nullvector();

  std::cout<<"NULLVECTOR: "<<v[0]<<", "<<v[1]<<", "<<v[2]<<std::endl;

  m_PlaneVector[0] = v[0];
  m_PlaneVector[1] = v[1];
  m_PlaneVector[2] = v[2];

}

const mitk::Point3D &mitk::PlaneFit::GetCentroid() const
{
  return m_Centroid;
}

void mitk::PlaneFit::InitializePlane()
{
  m_Plane->InitializePlane (m_Centroid, m_PlaneVector);
}
