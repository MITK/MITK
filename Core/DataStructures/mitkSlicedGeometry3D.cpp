#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include <vecmath.h>
#include <vtkTransform.h> 

#ifdef MBI_INTERNAL
extern "C"
{
#include "ipDicom.h"
}
#endif

//##ModelId=3DCBF50C0377
mitk::Geometry2D::ConstPointer mitk::SlicedGeometry3D::GetGeometry2D(int s, int t) const
{
  mitk::Geometry2D::ConstPointer geometry2d = NULL;
  
  if(IsValidSlice(s, t))
  {
    int pos=s+t*m_Dimensions[2];
    geometry2d = m_Geometry2Ds[pos];
    //if (a) we don't have a Geometry2D stored for the requested slice, 
    //(b) m_EvenlySpaced is activated and (c) the first slice (s=0,t=0) 
    //is a PlaneGeometry instance, then we calculate geometry of the requested
    //as the plane of the first slice shifted by m_Spacing*s.
    if((m_EvenlySpaced) && (geometry2d.IsNull()))
    {
      const PlaneGeometry* firstslice=dynamic_cast<const PlaneGeometry*> (m_Geometry2Ds[0].GetPointer());
      if(firstslice != NULL)
      {
        mitk::PlaneView view=firstslice->GetPlaneView();
        
        Vector3D n=view.normal;
        n.normalize();
        
        Vector3D zStep;
        zStep=n.dot(m_Spacing)*n;
        
        mitk::PlaneGeometry::Pointer requestedslice;
        requestedslice = mitk::PlaneGeometry::New();
        view.point+=zStep*s;
        requestedslice->SetPlaneView(view);
        geometry2d = requestedslice;
        m_Geometry2Ds[pos] = geometry2d;
      }
    }
  }
  return geometry2d;
}

//##ModelId=3DCBF5D40253
mitk::BoundingBox::ConstPointer mitk::SlicedGeometry3D::GetBoundingBox(int t) const
{
  if(m_BoundingBoxes[t].IsNotNull())
    return m_BoundingBoxes[t];
  
  mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();
  
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);
  
  assert(m_EvenlySpaced);
  
  int s, slices;
  mitk::BoundingBox::PointIdentifier pointid=0;
  
  slices=( GetDataDimension()<=2 ? 1 : GetDimensions()[2] );
  
  for(s=0;s<slices;++s)
  {
    const PlaneGeometry* planegeometry =
      dynamic_cast<const PlaneGeometry *>(GetGeometry2D(s,t).GetPointer());
    assert(planegeometry != NULL);
    
    const PlaneView& planeview=planegeometry->GetPlaneView();
    Point3D pt;
    
    Vector3D n=planeview.normal;
    n.normalize();
    
    Vector3D zStep;
    zStep=n.dot(m_Spacing)*n;
    
    //add the four edge points of the upper AND lower boundery of the plane geometry to the bounding box calculator.
    pt=planeview.point;
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    pt+=zStep;
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    
    pt=planeview.point+planeview.getOrientation1(); 
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    pt+=zStep;
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    
    pt+=planeview.getOrientation2();
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    pt+=zStep;
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    
    pt=planeview.point+planeview.getOrientation2();
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
    pt+=zStep;
    p[0]=pt.x; p[1]=pt.y; p[2]=pt.z;
    pointscontainer->InsertElement(pointid++, p);
  }
  
  boundingBox->SetPoints(pointscontainer);
  
  boundingBox->ComputeBoundingBox();
  
  m_BoundingBoxes[t]=boundingBox;
  
  return boundingBox.GetPointer();
}

//##ModelId=3DCBF5E9037F
/*!
\todo use parameter t or removed it!!!
*/
double mitk::SlicedGeometry3D::GetTime(int t) const
{
  itkExceptionMacro("GetTime not yet supported."); 	
  return 0;
}

//##ModelId=3DCBC65C017C
const float* mitk::SlicedGeometry3D::GetSpacing() const
{
  return &m_Spacing.x;
}

//##ModelId=3E15578402BD
bool mitk::SlicedGeometry3D::SetGeometry2D(const mitk::Geometry2D* geometry2D, int s, int t)
{
  if(IsValidSlice(s,t))
  {
    m_Geometry2Ds[s*m_Dimensions[2]+t*m_Dimensions[3]]=geometry2D;
    return true;
  }
  return false;
}

//##ModelId=3E155839024F
bool mitk::SlicedGeometry3D::SetGeometry2D(ipPicDescriptor* pic, int s, int t)
{
  if((pic!=NULL) && (IsValidSlice(s,t)))
  {
    //construct standard view
    mitk::Point3D origin, right, bottom;
    origin.set(0,0,s);               UnitsToMM(origin, origin);
    right.set(m_Dimensions[0],0,0);  UnitsToMM(right, right);
    bottom.set(0,m_Dimensions[1],0); UnitsToMM(bottom, bottom);
    
    PlaneView view_std(origin, right, bottom);
    
    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetSizeInUnits(m_Dimensions[0], m_Dimensions[1]);
    SetGeometry2D(planegeometry, s, t);
    return true;
  }
  return false;
}

//##ModelId=3E3453C703AF
void mitk::SlicedGeometry3D::Initialize(unsigned int dimension, const unsigned int* dimensions)
{
  Geometry3D::Initialize(dimension, dimensions);

  m_Geometry2Ds.clear();
  
  Geometry2D::ConstPointer gnull=NULL;
  int num=m_Dimensions[2]*m_Dimensions[3];
  
  m_Geometry2Ds.reserve(num);
  m_Geometry2Ds.assign(num, gnull);
  
  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();
  SetSpacing(Vector3D(1.0,1.0,1.0));
  
  //does the Geometry has 2D slices?
  if(num>0)
  {
    //construct standard view
    mitk::Point3D right, bottom;
    right.set(m_Dimensions[0],0,0); UnitsToMM(right, right);
    bottom.set(0,m_Dimensions[1],0); UnitsToMM(bottom, bottom);
    PlaneView view_std(mitk::Point3D(0,0,0), right, bottom);
    
    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    m_Geometry2Ds[0]=planegeometry;
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetSizeInUnits(m_Dimensions[0], m_Dimensions[1]);
  }
}

//##ModelId=3E15572E0269
mitk::SlicedGeometry3D::SlicedGeometry3D() : m_EvenlySpaced(true)
{
  Initialize(m_Dimension, m_Dimensions);
}

//##ModelId=3E3456C50067
mitk::SlicedGeometry3D::~SlicedGeometry3D()
{

}

//##ModelId=3E3BE1F10106
bool mitk::SlicedGeometry3D::IsValidSlice(int s, int t) const
{
  return ((s>=0) && (s<(int)m_Dimensions[2]) && (t>=0) && (t< (int)m_Dimensions[3]));
}

//##ModelId=3E3BE8CF010E
void mitk::SlicedGeometry3D::SetSpacing(mitk::Vector3D aSpacing)
{
  m_Spacing = aSpacing;
  
  m_TransformUnitsToMM=m_TransformOfOrigin;
  
  mitk::Vector4D col;
  m_TransformUnitsToMM.getColumn(0, &col); col*=aSpacing.x; m_TransformUnitsToMM.setColumn(0, col);
  m_TransformUnitsToMM.getColumn(1, &col); col*=aSpacing.y; m_TransformUnitsToMM.setColumn(1, col);
  m_TransformUnitsToMM.getColumn(2, &col); col*=aSpacing.z; m_TransformUnitsToMM.setColumn(2, col);
  
  m_TransformMMToUnits.invert(m_TransformUnitsToMM);
  
  //re-initialize bounding box array, since the spacing influences the size of the bounding box
  int num=m_Dimensions[2]*m_Dimensions[3];
  BoundingBox::ConstPointer bnull=NULL;
  m_BoundingBoxes.clear();
  m_BoundingBoxes.reserve(m_Dimensions[3]);
  m_BoundingBoxes.assign(num, bnull);
  
  //in case of evenly-spaced data: re-initialize instances of Geometry2D, since the spacing influences them
  if(m_EvenlySpaced)
  {
    m_Geometry2Ds.clear();
    
    Geometry2D::ConstPointer gnull=NULL;
    m_Geometry2Ds.reserve(num);
    m_Geometry2Ds.assign(num, gnull);
    
    //does the Geometry has 2D slices?
    if(num>0)
    {
      //construct standard view
      mitk::Point3D right, bottom;
      right.set(m_Dimensions[0],0,0);  UnitsToMM(right, right);
      bottom.set(0,m_Dimensions[1],0); UnitsToMM(bottom, bottom);
      PlaneView view_std(mitk::Point3D(0,0,0), right, bottom);
      
      mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
      m_Geometry2Ds[0]=planegeometry;
      planegeometry->SetPlaneView(view_std);
      planegeometry->SetSizeInUnits(m_Dimensions[0], m_Dimensions[1]);
    }
  }
  
  Modified();
}

void mitk::SlicedGeometry3D::SetSpacing(const float aSpacing[3])
{
  mitk::Vector3D tmp(aSpacing[0], aSpacing[1], aSpacing[2]);
  SetSpacing(tmp);
}

//##ModelId=3E3C13F802A6
void mitk::SlicedGeometry3D::SetEvenlySpaced(bool on)
{
  m_EvenlySpaced=on;
  Modified();
}

//##ModelId=3E3C2C37031B
void mitk::SlicedGeometry3D::SetSpacing(ipPicDescriptor* pic)
{
  Vector3D spacing(m_Spacing);
  
  ipPicTSV_t *tsv;
  
  tsv = ipPicQueryTag( pic, "PIXEL SIZE" );
  if(tsv)
  {
    if((tsv->dim*tsv->n[0]>=3) && (tsv->type==ipPicFloat))
    {
      if(tsv->bpe==32)
        spacing.set(((ipFloat4_t*)tsv->value)[0], ((ipFloat4_t*)tsv->value)[1],((ipFloat4_t*)tsv->value)[2]);
      else
        if(tsv->bpe==64)
          spacing.set(((ipFloat8_t*)tsv->value)[0], ((ipFloat8_t*)tsv->value)[1],((ipFloat8_t*)tsv->value)[2]);
    }
  }
#ifdef MBI_INTERNAL
  else
  {		
    tsv = ipPicQueryTag( pic, "SOURCE HEADER" );
    if( tsv )
    {
      void *data;
      ipUInt4_t len;
      ipFloat8_t spacing_z = 0;
      ipFloat8_t thickness = 1;
      ipFloat8_t fx = 1;
      ipFloat8_t fy = 1;
      
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0088, &data, &len ) )
      {
        sscanf( (char *) data, "%lf", &spacing );
        itkDebugMacro( "spacing:   %5.2f mm\n" << spacing_z );
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0050, &data, &len ) )
      {
        sscanf( (char *) data, "%lf", &thickness );
        itkDebugMacro( "thickness: %5.2f mm\n" << thickness );
        
        if( thickness == 0 )
          thickness = 1;
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0028, 0x0030, &data, &len )
        && len>0 && ((char *)data)[0] )
      {
        sscanf( (char *) data, "%lf\\%lf", &fy, &fx );    // row / column value 
        itkDebugMacro( "fx, fy:    %5.2f/%5.2f mm\n" << fx << fy );
      }
      
      spacing.set(fx, fy,( spacing_z > 0 ? spacing_z : thickness));
    }
  }
#endif
  // @FIXME:
  // nur fuer Testdatensatz
  // spacing = Vector3D(1/3.0,1/4.0,1/5.0);
  SetSpacing(spacing);
}

mitk::Geometry3D::Pointer mitk::SlicedGeometry3D::Clone()
{
  mitk::SlicedGeometry3D::Pointer newGeometry = SlicedGeometry3D::New();
  newGeometry->Initialize(m_Dimension, m_Dimensions);
  newGeometry->GetTransform()->SetMatrix(m_Transform->GetMatrix());
  //newGeometry->GetRelativeTransform()->SetMatrix(m_RelativeTransform->GetMatrix());
  return newGeometry.GetPointer();
}