#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include <vecmath.h>
#include <vtkTransform.h> 

#ifdef MBI_INTERNAL
extern "C"
{
#include "ipDicom.h"
}
#endif

//##ModelId=3DCBF3AD0110
const unsigned int *mitk::Geometry3D::GetDimensions() const
{
  return m_Dimensions;
}

//##ModelId=3DDE649E00A3
unsigned int mitk::Geometry3D::GetDataDimension() const
{
  return m_Dimension;
}

//##ModelId=3DCBF5D40253
mitk::BoundingBox::ConstPointer mitk::Geometry3D::GetBoundingBox(int t) const
{
  assert(m_BoundingBoxes[t].IsNotNull());

  return m_BoundingBoxes[t];
}

//##ModelId=3ED91D050299
void mitk::Geometry3D::SetBoundingBox(const mitk::BoundingBox* boundingBox,  int t)
{
  if(IsValidTime(t))
    m_BoundingBoxes[t]=boundingBox;
  else
    itkExceptionMacro("tried to set boundingbox for an invalid point of time (t:"<<t<<")");
}

//##ModelId=3ED91D050305
void mitk::Geometry3D::SetBoundingBox(const float bounds[6],  int t)
{
  if(IsValidTime(t))
  {
    mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();
    
    mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
    mitk::ScalarType nullpoint[]={0,0,0};
    mitk::BoundingBox::PointType p(nullpoint);
    
    mitk::BoundingBox::PointIdentifier pointid=0;
    
    p[0]=bounds[0]; p[1]=bounds[2]; p[2]=bounds[4];
    pointscontainer->InsertElement(pointid++, p);
    
    p[0]=bounds[1]; p[2]=bounds[3]; p[2]=bounds[5];
    pointscontainer->InsertElement(pointid++, p);
    
    boundingBox->SetPoints(pointscontainer);
    
    boundingBox->ComputeBoundingBox();
    
    m_BoundingBoxes[t]=boundingBox;
  }
  else
    itkExceptionMacro("tried to set boundingbox for an invalid point of time (t:"<<t<<")");
}

//##ModelId=3DCBF5E9037F
/*!
\todo use parameter t or removed it!!!
*/
double mitk::Geometry3D::GetTime(int t) const
{
  itkExceptionMacro("GetTime not yet supported."); 	
  return 0;
}

//##ModelId=3DE763C500C4
/*!
\todo use parameter t or removed it!!!
\deprecated New method returns a vtkTransform
*/

//##ModelId=3DDE65D1028A
/*!
\todo use parameter t or removed it!!! 
*/
void mitk::Geometry3D::MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units, float t) const
{
  m_TransformMMToUnits.transform(pt_mm, &pt_units);
}

//##ModelId=3DDE65DC0151
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm, float t) const
{
  m_TransformUnitsToMM.transform(pt_units, &pt_mm);
}

//##ModelId=3E3B986602CF
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units, float t) const
{
  m_TransformMMToUnits.transform(vec_mm, &vec_units);
}

//##ModelId=3E3B987503A3
/*!
\todo use parameter t or removed it!!!
*/
void mitk::Geometry3D::UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm, float t) const
{
  m_TransformUnitsToMM.transform(vec_units, &vec_mm);
}

//##ModelId=3E3453C703AF
void mitk::Geometry3D::Initialize(unsigned int dimension, const unsigned int* dimensions)
{
  unsigned int* tmpDimensions=new unsigned int[dimension>4?dimension:4];
  memcpy(tmpDimensions, dimensions, sizeof(unsigned int)*dimension);
  if(dimension<4)
  {
    unsigned int i, *p;
    for(i=0,p=tmpDimensions+dimension;i<4-dimension;++i, ++p)
      *p=1;
  }
  
  m_Dimension=dimension;
  if(m_Dimensions!=NULL)
    delete [] m_Dimensions;
  m_Dimensions = tmpDimensions;

  unsigned int i;
  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, m_Dimensions[i]);
  }
  
  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();
  
  //initialize bounding box array
  int num=m_Dimensions[3];
  BoundingBox::ConstPointer bnull=NULL;
  m_BoundingBoxes.reserve(m_Dimensions[3]);
  m_BoundingBoxes.assign(num, bnull);
  
  //New
  m_Transform = vtkTransform::New();  
  m_BaseGeometry = NULL; // there is no base geometry, this one is independend (until SetBaseGeometry() is called)
}

// Standard Constructor for the new makro. sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D(): m_Dimension(0), m_Dimensions(NULL)
{
  unsigned int * dims = new unsigned int[3];
  dims[0] = dims[1] = dims[2] = 1;
  Initialize(3, dims);
  delete [] dims;                        
}

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{
  if(m_Dimensions!=NULL)
    delete m_Dimensions;  
}

//##ModelId=3E3BE1F8000C
bool mitk::Geometry3D::IsValidTime(int t) const
{
  return (t>=0) && (t< (int)m_Dimensions[3]);
}

//New:

/*!
    /brief Returns the actual global transformation
    There are 4 cases:
    - the local transformation has not changed and the transformation of the base geometry has not changed
    - the local transformation has not changed but the transformation of the base geometry has changed
    - the local transformation has changed and the transformation of the base geometry has not changed
    - both the local and the base transformations have changed    
*/
vtkTransform* mitk::Geometry3D::GetTransform()
{
/*  bool baseGeometryChanged = (m_BaseTransformTimeStamp != m_BaseGeometry->GetTransform()->GetMTime());
  bool ownGeometryChanged =  (m_TransformTimeStamp != m_RelativeTransform->GetMTime());
  if (ownGeometryChanged)
    if (baseGeometryChanged)
    { // case 4
      // must update relative tranform, so that basetransform * relativetransform = own transform
      // and must update own transform somehow...
    }
    else
    { // case 3
      // must update relative tranform, so that basetransform * relativetransform = own transform

    }
  else  //own Geometry has not changed
    if (baseGeometryChanged)
    { // case 2
      // must update own transform, so that basetransform * relative transform = own transform
      m_Transform = m_BaseGeometry->GetTransform()
      return m_Transform;  
    }
    else
    { // case 1
      // nothing has changed, just return the transform
      return m_Transform;
    }
*/
  return m_Transform;
}

void mitk::Geometry3D::SetBaseGeometry(mitk::Geometry3D* base)
{
  m_Transform->SetInput(base->GetTransform());
  m_BaseGeometry = base;
}

void mitk::Geometry3D::SetMasterTransform(const vtkTransform * transform)
{

}

mitk::Geometry3D::Pointer mitk::Geometry3D::Clone()
{
  mitk::Geometry3D::Pointer newGeometry = Geometry3D::New();
  newGeometry->Initialize(m_Dimension, m_Dimensions);
  newGeometry->GetTransform()->SetMatrix(m_Transform->GetMatrix());
  //newGeometry->GetRelativeTransform()->SetMatrix(m_RelativeTransform->GetMatrix());
  return newGeometry;

}