#include "mitkPicHelper.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#ifdef MBI_INTERNAL
#include "ipDicom.h"
#endif

bool mitk::PicHelper::GetSpacing(ipPicDescriptor* pic, Vector3D & spacing)
{
  ipPicTSV_t *tsv;
  
  tsv = ipPicQueryTag( pic, "PIXEL SIZE" );
  if(tsv)
  {
    if((tsv->dim*tsv->n[0]>=3) && (tsv->type==ipPicFloat))
    {
      if(tsv->bpe==32)
      {
        FillVector3D(spacing,((ipFloat4_t*)tsv->value)[0], ((ipFloat4_t*)tsv->value)[1],((ipFloat4_t*)tsv->value)[2]);
        return true;
      }
      else
      if(tsv->bpe==64)
      {
        FillVector3D(spacing,((ipFloat8_t*)tsv->value)[0], ((ipFloat8_t*)tsv->value)[1],((ipFloat8_t*)tsv->value)[2]);
        return true;
      }
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
      bool ok=false;
      
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0088, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &spacing );
        itkGenericOutputMacro( "spacing:   %5.2f mm\n" << spacing_z );
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0050, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &thickness );
        itkGenericOutputMacro( "thickness: %5.2f mm\n" << thickness );
        
        if( thickness == 0 )
          thickness = 1;
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0028, 0x0030, &data, &len )
        && len>0 && ((char *)data)[0] )
      {
        sscanf( (char *) data, "%lf\\%lf", &fy, &fx );    // row / column value 
        itkGenericOutputMacro( "fx, fy:    %5.2f/%5.2f mm\n" << fx << fy );
        return false;
      }
      if(ok)
        FillVector3D(spacing, fx, fy,( spacing_z > 0 ? spacing_z : thickness));
      return ok;
    }
  }
#endif
  // @FIXME:
  // nur fuer Testdatensatz
  // spacing = Vector3D(1/3.0,1/4.0,1/5.0);
  return false;
}

bool mitk::PicHelper::SetSpacing(ipPicDescriptor* pic, SlicedGeometry3D* slicedgeometry)
{
  Vector3D spacing(slicedgeometry->GetSpacing());
  if(GetSpacing(pic, spacing))
  {
    slicedgeometry->SetSpacing(spacing);
    return true;
  }
  return false;
}

void mitk::PicHelper::InitializeEvenlySpaced(ipPicDescriptor* pic, unsigned int slices, SlicedGeometry3D* slicedgeometry)
{
  assert(pic!=NULL);
  assert(slicedgeometry!=NULL);
  Vector3D spacing;
  spacing.Fill(1);
  GetSpacing(pic, spacing);

  mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
  planegeometry->InitializeStandardPlane(pic->n[0], pic->n[1], spacing);
  slicedgeometry->InitializeEvenlySpaced(planegeometry, spacing[2], slices);
}

bool mitk::PicHelper::SetGeometry2D(ipPicDescriptor* pic, int s, SlicedGeometry3D* slicedgeometry)
{
  if((pic!=NULL) && (slicedgeometry->IsValidSlice(s)))
  {
    //construct standard view
    mitk::Point3D origin;
    mitk::Vector3D rightDV, bottomDV;
    FillVector3D(origin,0,0,s); slicedgeometry->UnitsToMM(origin, origin);
    FillVector3D(rightDV,pic->n[0],0,0);
    FillVector3D(bottomDV,0,pic->n[1],0);
    
    mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();

    planegeometry->InitializeStandardPlane(rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &slicedgeometry->GetSpacing());
    planegeometry->SetOrigin(origin);

    slicedgeometry->SetGeometry2D(planegeometry, s);
    return true;
  }
  return false;
}

//FIXME: einbauen!!!
//bool mitk::PicHelper::SetGeometry2D(ipPicDescriptor* pic, int s, SlicedGeometry3D* slicedgeometry)
//{
//  if((pic!=NULL) && ((IsValidSlice(s)) || (s == -1)))
//  {
//    ipPicTSV_t *tsv;
//    if ( tsv = ipPicQueryTag( pic, "REAL PIXEL SIZES" ) )
//    {    
//      if(s == -1)
//      {
//        m_Geometry2Ds.clear();
//        Geometry2D::Pointer gnull=NULL;
//        m_Geometry2Ds.reserve(m_Slices);
//        m_Geometry2Ds.assign(m_Slices, gnull);       
//      }
//
//      int count = tsv->n[1];
//      float* value = (float*) tsv->value;
//      mitk::Point3D pixelSize;
//      ScalarType zPosition = 0.0f;
//
//      if(s != -1)
//      {
//        if(count < s)
//          return false;
//        count = s;
//      }
//      else
//      {
//        if(count < m_Slices)
//          return false;
//        count = m_Slices;
//      }
//
//      int slice;
//      for (slice=0; slice < count; ++slice ) 
//      {
//        pixelSize.x = (ScalarType) *value++;
//        pixelSize.y = (ScalarType) *value++;
//        pixelSize.z = (ScalarType) *value++;
//
//        zPosition += pixelSize.z / 2.0f;    // first half slice thickness
//
//        if((s==-1) || (slice==s))
//        {
//          mitk::Point3D origin, right, bottom;
//          origin.set( 0, 0, zPosition );         
//          right.set( pic->n[0] * pixelSize.x, 0, zPosition );  
//          bottom.set( 0, pic->n[1] * pixelSize.y, zPosition ); 
//
//          PlaneView planeView( origin, right, bottom );
//          mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
//          planegeometry->SetPlaneView( planeView );    
//          planegeometry->SetThickness( pixelSize.z );
//          SetGeometry2D( planegeometry, slice );
//        }
//
//        zPosition += pixelSize.z / 2.0f;  // second half slice thickness
//      } 
//
//      itkWarningMacro(<< "the sclices are inhomogeneous" );
//      m_EvenlySpaced = false;
//    }
//    else
//    {
//      //construct standard view
//      mitk::Point3D origin, right, bottom;
//
//      origin.set( 0, 0, m_Spacing.z*(s+ 0.5f));
//      right.set( pic->n[0] * m_Spacing.x, 0, m_Spacing.z / 2.0f );  
//      bottom.set( 0, pic->n[1] * m_Spacing.y, m_Spacing.z / 2.0f ); 
//
//      PlaneView planeView( origin, right, bottom );
//      mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
//      planegeometry->SetPlaneView( planeView );    
//      planegeometry->SetThickness( m_Spacing.z );
//      if(s==-1)
//      {
//        m_Geometry2Ds.clear();
//        Geometry2D::Pointer gnull=NULL;
//        m_Geometry2Ds.reserve(m_Slices);
//        m_Geometry2Ds.assign(m_Slices, gnull);       
//        s = 0;
//        m_EvenlySpaced = true;
//      }
//      SetGeometry2D(planegeometry, s);
//    }
//    return true;
//  }
//  return false;
//}

////##ModelId=3E3C2C37031B
//void mitk::SlicedGeometry3D::SetSpacing(ipPicDescriptor* pic)
//{
//  ipPicTSV_t *tsv;
//  mitk::Point3D origin, right, bottom;
//  
//  tsv = ipPicQueryTag( pic, "REAL PIXEL SIZE" );
//
//  if ( tsv ) 
//  {
//    SetSpacing( tsv );
//    m_EvenlySpaced = true;
//  }
//  else if ( tsv = ipPicQueryTag( pic, "REAL PIXEL SIZES" ) )
//  {    
//    int count = tsv->n[1];
//    float* value = (float*) tsv->value;
//    mitk::Point3D pixelSize;
//    mitk::Point3D summedPixelSize(0,0,0);
//
//    for ( int s=0; s < count; s++ ) 
//    {
//			pixelSize.x = (ScalarType) *value++;
//			pixelSize.y = (ScalarType) *value++;
//			pixelSize.z = (ScalarType) *value++;
//      summedPixelSize += pixelSize;
//    }
//    summedPixelSize *= 1.0f/count;
//    SetSpacing(summedPixelSize);
//
//    itkWarningMacro(<< "the slices are inhomogeneous" );
//    m_EvenlySpaced = false;
//  }
//  else if ( tsv = ipPicQueryTag( pic, "PIXEL SIZE" ) )
//  {
//    SetSpacing( tsv );    
//    m_EvenlySpaced = true;
//    itkDebugMacro(<< "using the tag PIXEL SIZE: " << m_Spacing << " this may not the correct thickness of the slice");
//  }
//#ifdef MBI_INTERNAL
//  else if( tsv = ipPicQueryTag( pic, "SOURCE HEADER" ) )
//  {		
//    void *data;
//    ipUInt4_t len;
//    ipFloat8_t spacing_z = 0;
//    ipFloat8_t thickness = 1;
//    ipFloat8_t fx = 1;
//    ipFloat8_t fy = 1;
//    
//    if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0088, &data, &len ) )
//    {
//      sscanf( (char *) data, "%lf", &m_Spacing );
//      itkDebugMacro( "spacing:   %5.2f mm\n" << spacing_z );
//    }
//    if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0050, &data, &len ) )
//    {
//      sscanf( (char *) data, "%lf", &thickness );
//      itkDebugMacro( "thickness: %5.2f mm\n" << thickness );
//      
//      if( thickness == 0 )
//        thickness = 1;
//    }
//    if( dicomFindElement( (unsigned char *) tsv->value, 0x0028, 0x0030, &data, &len )
//      && len>0 && ((char *)data)[0] )
//    {
//      sscanf( (char *) data, "%lf\\%lf", &fy, &fx );    // row / column value 
//      itkDebugMacro( "fx, fy:    %5.2f/%5.2f mm\n" << fx << fy );
//    }
//    
//    Vector3D spacing(fx, fy,( spacing_z > 0 ? spacing_z : thickness));
//    SetSpacing(spacing);
//
//    itkWarningMacro(<< "using dicom spacing information: " << m_Spacing << " this may not the correct thickness of the slice");    
//
//    m_EvenlySpaced = true;
//  }
//#endif
//  else
//  {
//    Vector3D spacing(1,1,1);
//    SetSpacing(spacing);
//    itkWarningMacro(<< "no spacing information found in pic-desciptor");
//  }
//  // @FIXME:
//  // nur fuer Testdatensatz
//  // spacing = Vector3D(1/3.0,1/4.0,1/5.0);
//}

