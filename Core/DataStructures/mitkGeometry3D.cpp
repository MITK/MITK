#include "Geometry3D.h"
#include "PlaneGeometry.h"
#include <vecmath.h>

#ifdef MITK_DICOM_ENABLED
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

//##ModelId=3DCBF50C0377
mitk::Geometry2D::ConstPointer mitk::Geometry3D::GetGeometry2D(int s, int t) const
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
mitk::BoundingBox::ConstPointer mitk::Geometry3D::GetBoundingBox(int t) const
{
  if(m_BoundingBoxes[t].IsNotNull())
    return m_BoundingBoxes[t];

  mitk::BoundingBox::Pointer boundingBox=mitk::BoundingBox::New();

  mitk::BoundingBox::PointsContainer::Pointer pointscontainer=mitk::BoundingBox::PointsContainer::New();
  float nullpoint[]={0,0,0};
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
    float nullpoint[]={0,0,0};
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
*/
mitk::Geometry3D::TransformPointer mitk::Geometry3D::GetTransfrom() const
{
  itkExceptionMacro("Transform not yet supported."); 	
  return NULL;
}

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

//##ModelId=3DCBC65C017C
const float* mitk::Geometry3D::GetSpacing() const
{
  return &m_Spacing.x;
}

//##ModelId=3E15578402BD
bool mitk::Geometry3D::SetGeometry2D(const mitk::Geometry2D* geometry2D, int s, int t)
{
  if(IsValidSlice(s,t))
  {
    m_Geometry2Ds[s*m_Dimensions[2]+t*m_Dimensions[3]]=geometry2D;
    return true;
  }
  return false;
}

//##ModelId=3E155839024F
bool mitk::Geometry3D::SetGeometry2D(ipPicDescriptor* pic, int s, int t)
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
void mitk::Geometry3D::Initialize(unsigned int dimension, const unsigned int* dimensions)
{
  m_Dimension=dimension;
  if(m_Dimensions!=NULL)
    delete m_Dimensions;
  m_Dimensions=new unsigned int[m_Dimension>4?m_Dimension:4];
  memcpy(m_Dimensions, dimensions, sizeof(unsigned int)*m_Dimension);
  if(m_Dimension<4)
  {
    unsigned int i, *p;
    for(i=0,p=m_Dimensions+m_Dimension;i<4-m_Dimension;++i, ++p)
      *p=1;
  }

  unsigned int i;
  for(i=0;i<4;++i)
  {
    m_LargestPossibleRegion.SetIndex(i, 0);
    m_LargestPossibleRegion.SetSize (i, m_Dimensions[i]);
  }

  m_Geometry2Ds.clear();

  Geometry2D::ConstPointer gnull=NULL;
  int num=m_Dimensions[2]*m_Dimensions[3];

  m_Geometry2Ds.reserve(num);
  m_Geometry2Ds.assign(num, gnull);

  //initialize m_TransformOfOrigin and m_Spacing (and m_TransformUnitsToMM/m_TransformMMToUnits).
  m_TransformOfOrigin.setIdentity();
  SetSpacing(Vector3D(1.0,1.0,1.0));

  //initialize bounding box array
  BoundingBox::ConstPointer bnull=NULL;
  m_BoundingBoxes.reserve(m_Dimensions[3]);
  m_BoundingBoxes.assign(num, bnull);

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
mitk::Geometry3D::Geometry3D(unsigned int dimension, const unsigned int* dimensions) : m_Dimension(0), m_Dimensions(NULL), m_EvenlySpaced(true)
{
  Initialize(dimension, dimensions);
}

//##ModelId=3E3452F10253
mitk::Geometry3D::Geometry3D(const mitk::Geometry3D& right) : m_Dimensions(NULL)
{
  Initialize(right.GetDataDimension(),right.GetDimensions());
}

//##ModelId=3E3452F10393
// mitk::Geometry3D& mitk::Geometry3D::operator=(const mitk::Geometry3D& right)
// {
//     return Geometry3D(right);
// }

//##ModelId=3E3456C50067
mitk::Geometry3D::~Geometry3D()
{
  if(m_Dimensions!=NULL)
    delete m_Dimensions;
}

//##ModelId=3E3BE1F10106
bool mitk::Geometry3D::IsValidSlice(int s, int t) const
{
  return ((s>=0) && (s<(int)m_Dimensions[2]) && (t>=0) && (t< (int)m_Dimensions[3]));
}

//##ModelId=3E3BE1F8000C
bool mitk::Geometry3D::IsValidTime(int t) const
{
  return IsValidSlice(0, t);
}

//##ModelId=3E3BE8CF010E
void mitk::Geometry3D::SetSpacing(mitk::Vector3D aSpacing)
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

void mitk::Geometry3D::SetSpacing(const float aSpacing[3])
{
  mitk::Vector3D tmp(aSpacing[0], aSpacing[1], aSpacing[2]);
  SetSpacing(tmp);
}

//##ModelId=3E3C13F802A6
void mitk::Geometry3D::SetEvenlySpaced(bool on)
{
  m_EvenlySpaced=on;
  Modified();
}

//##ModelId=3E3C2C37031B
void mitk::Geometry3D::SetSpacing(ipPicDescriptor* pic)
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
#ifdef MITK_DICOM_ENABLED
  else
  {		
    tsv = ipPicQueryTag( pic, "SOURCE HEADER" );
    if( tsv )
    {
      ipFloat8_t spacing = 0;
      ipFloat8_t thickness = 1;
      ipFloat8_t fx = 1;
      ipFloat8_t fy = 1;

      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0088, &data, &len ) )
      {
        sscanf( (char *) data, "%lf", &spacing );
        itkDebugMacro( "spacing:   %5.2f mm\n" << spacing );
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

      spacing.set(fx, fy,( spacing > 0 ? spacing : thickness));
    }
  }
#endif
  // @FIXME:
  // nur fuer Testdatensatz
  // spacing = Vector3D(1/3.0,1/4.0,1/5.0);
  SetSpacing(spacing);
}
