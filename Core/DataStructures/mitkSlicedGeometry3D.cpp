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
mitk::Geometry2D* mitk::SlicedGeometry3D::GetGeometry2D(int s) const
{
  mitk::Geometry2D::Pointer geometry2d = NULL;
  
  if(IsValidSlice(s))
  {
    geometry2d = m_Geometry2Ds[s];
    //If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored 
    //for the requested slice, and (c) the first slice (s=0) 
    //is a PlaneGeometry instance, then we calculate the geometry of the
    //requested as the plane of the first slice shifted by m_Spacing.z*s
    //in the direction of m_DirectionVector.
    if((m_EvenlySpaced) && (geometry2d.IsNull()))
    {
      PlaneGeometry* firstslice=dynamic_cast<PlaneGeometry*> (m_Geometry2Ds[0].GetPointer());
      if(firstslice != NULL)
      {
        mitk::PlaneView view=firstslice->GetPlaneView();

        if((m_DirectionVector.x==0) && (m_DirectionVector.y==0) && (m_DirectionVector.z==0))
        {
          m_DirectionVector=view.normal;
          m_DirectionVector.normalize();
        }
        Vector3D direction;
        direction = m_DirectionVector*m_Spacing.z;

        mitk::PlaneGeometry::Pointer requestedslice;
        requestedslice = mitk::PlaneGeometry::New();
        requestedslice->Initialize();
        view.point+=direction*s;
        requestedslice->SetPlaneView(view);
        requestedslice->SetThickness(firstslice->GetThickness());
        geometry2d = requestedslice;
        m_Geometry2Ds[s] = geometry2d;
      }
    }
  }
  else
    return NULL;
  return geometry2d;
}

//##ModelId=3DCBF5D40253
const mitk::BoundingBox* mitk::SlicedGeometry3D::GetBoundingBox() const
{
  mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();
 
  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  mitk::ScalarType nullpoint[]={0,0,0};
  mitk::BoundingBox::PointType p(nullpoint);

  unsigned int s;

  mitk::Geometry2D* geometry2d;
  mitk::BoundingBox::ConstPointer nextBoundingBox;
  mitk::BoundingBox::PointIdentifier pointid=0;

  for(s=0; s<m_Slices; ++s)
  {    
    geometry2d = GetGeometry2D(s);
    assert(geometry2d!=NULL);
    nextBoundingBox = geometry2d->GetBoundingBox();
    const mitk::BoundingBox::PointsContainer * nextPoints = nextBoundingBox->GetPoints();
    if(nextPoints!=NULL)
    {
      mitk::BoundingBox::PointsContainer::ConstIterator pointsIt = nextPoints->Begin();

      while (pointsIt != nextPoints->End() )
      {
        pointscontainer->InsertElement( pointid++, pointsIt->Value());
        ++pointsIt;
      }
    }
  }

  boundingBox->SetPoints(pointscontainer);

  boundingBox->ComputeBoundingBox();

  m_BoundingBox=boundingBox;

  return boundingBox.GetPointer();
}

//##ModelId=3DCBC65C017C
const float* mitk::SlicedGeometry3D::GetSpacing() const
{
  return &m_Spacing.x;
}

//##ModelId=3E15578402BD
bool mitk::SlicedGeometry3D::SetGeometry2D(mitk::Geometry2D* geometry2D, int s)
{
  if(IsValidSlice(s))
  {
    m_Geometry2Ds[s]=geometry2D;
    return true;
  }
  return false;
}

//##ModelId=3E155839024F
bool mitk::SlicedGeometry3D::SetGeometry2D(ipPicDescriptor* pic, int s)
{
  if((pic!=NULL) && (IsValidSlice(s)))
  {
    //construct standard view
    mitk::Point3D origin, right, bottom;
    origin.set(0,0,s);         UnitsToMM(origin, origin);
    right.set(pic->n[0],0,0);  UnitsToMM(right, right);
    bottom.set(0,pic->n[1],0); UnitsToMM(bottom, bottom);
    
    PlaneView view_std(origin, right, bottom);
    
    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetThicknessBySpacing(&m_Spacing);
    planegeometry->SetSizeInUnits(pic->n[0], pic->n[1]);
    SetGeometry2D(planegeometry, s);
    return true;
  }
  return false;
}

//##ModelId=3E3453C703AF
void mitk::SlicedGeometry3D::Initialize(unsigned int slices)
{
  Superclass::Initialize();
  m_Slices = slices;

  m_Geometry2Ds.clear();
 
  Geometry2D::Pointer gnull=NULL;
 
  m_Geometry2Ds.reserve(m_Slices);
  m_Geometry2Ds.assign(m_Slices, gnull);
 
  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();
  SetSpacing(Vector3D(1.0,1.0,1.0));
}

//##ModelId=3E15572E0269
mitk::SlicedGeometry3D::SlicedGeometry3D() : m_Slices(0), m_EvenlySpaced(true)
{
  Initialize(m_Slices);
}

//##ModelId=3E3456C50067
mitk::SlicedGeometry3D::~SlicedGeometry3D()
{

}

//##ModelId=3E3BE1F10106
bool mitk::SlicedGeometry3D::IsValidSlice(int s) const
{
  return ((s>=0) && (s<(int)m_Slices));
}

//##ModelId=3E3BE8CF010E
void mitk::SlicedGeometry3D::SetSpacing(mitk::Vector3D aSpacing)
{
  bool hasEvenlySpacedPlaneGeometry=false;
  mitk::Point3D origin, rightDV, bottomDV;
  unsigned int width, height;
  mitk::ScalarType thickness;
  //in case of evenly-spaced data: re-initialize instances of Geometry2D, since the spacing influences them
  if((m_EvenlySpaced) && (m_Geometry2Ds.size()>0))
  {
    mitk::Geometry2D::ConstPointer firstGeometry = m_Geometry2Ds[0].GetPointer();

    const PlaneGeometry* constplanegeometry=dynamic_cast<const PlaneGeometry*>(firstGeometry.GetPointer());
    if(constplanegeometry != NULL)
    {
      MMToUnits(constplanegeometry->GetPlaneView().point, origin);
      MMToUnits(constplanegeometry->GetPlaneView().getOrientation1(), rightDV);
      MMToUnits(constplanegeometry->GetPlaneView().getOrientation2(), bottomDV);
      width  = constplanegeometry->GetWidthInUnits();
      height = constplanegeometry->GetHeightInUnits();
      thickness = constplanegeometry->GetThickness();
      hasEvenlySpacedPlaneGeometry=true;
    }
  }

  m_Spacing = aSpacing;
  
  m_TransformUnitsToMM=m_TransformOfOrigin;
  
  mitk::Vector4D col;
  m_TransformUnitsToMM.getColumn(0, &col); col*=aSpacing.x; m_TransformUnitsToMM.setColumn(0, col);
  m_TransformUnitsToMM.getColumn(1, &col); col*=aSpacing.y; m_TransformUnitsToMM.setColumn(1, col);
  m_TransformUnitsToMM.getColumn(2, &col); col*=aSpacing.z; m_TransformUnitsToMM.setColumn(2, col);
  
  m_TransformMMToUnits.invert(m_TransformUnitsToMM);
  
  //re-initialize the bounding box, since the spacing influences the size of the bounding box
  m_BoundingBox = NULL;
  
  mitk::Geometry2D::Pointer firstGeometry;

  //in case of evenly-spaced data: re-initialize instances of Geometry2D, since the spacing influences them
  if(hasEvenlySpacedPlaneGeometry)
  {
    //create planegeometry according to new spacing
    UnitsToMM(origin, origin);
    UnitsToMM(rightDV, rightDV);
    UnitsToMM(bottomDV, bottomDV);

    PlaneView view_std(origin, origin+rightDV, origin+bottomDV);

    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
    planegeometry->SetPlaneView(view_std);
    planegeometry->SetSizeInUnits(width, height);
    planegeometry->SetThickness(thickness);
    firstGeometry = planegeometry;
  }
  else
  if((m_EvenlySpaced) && (m_Geometry2Ds.size()>0))
    firstGeometry = m_Geometry2Ds[0].GetPointer();

  //clear and reserve
  m_Geometry2Ds.clear();

  Geometry2D::Pointer gnull=NULL;
  
  m_Geometry2Ds.reserve(m_Slices);
  m_Geometry2Ds.assign(m_Slices, gnull);

  if(m_Slices>0)
    m_Geometry2Ds[0] = firstGeometry;
 
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

void mitk::SlicedGeometry3D::SetDirectionVector(const mitk::Vector3D& directionVector)
{
  if(Vector3D(m_DirectionVector-directionVector).lengthSquared()>=0.0001)
  {
    m_DirectionVector = directionVector;
    m_DirectionVector.normalize();
    Modified();
  }
}

mitk::Geometry3D::Pointer mitk::SlicedGeometry3D::Clone() const
{
  mitk::SlicedGeometry3D::Pointer newGeometry = SlicedGeometry3D::New();
  newGeometry->Initialize(m_Slices);
  newGeometry->GetTransform()->SetMatrix(m_Transform->GetMatrix());
  //newGeometry->GetRelativeTransform()->SetMatrix(m_RelativeTransform->GetMatrix());
  newGeometry->SetEvenlySpaced(m_EvenlySpaced);
  newGeometry->SetSpacing(newGeometry->GetSpacing());
  unsigned int s;
  for(s=0; s<m_Slices; ++s)
  {
    newGeometry->SetGeometry2D(m_Geometry2Ds[s], s);
  }
  return newGeometry.GetPointer();
}

