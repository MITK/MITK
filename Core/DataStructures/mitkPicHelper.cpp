#include "mitkPicHelper.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#ifdef MBI_INTERNAL
#include "ipDicom/ipDicom.h"
#endif

bool mitk::PicHelper::GetSpacing(ipPicDescriptor* pic, Vector3D & spacing)
{
  ipPicTSV_t *tsv;

  tsv = ipPicQueryTag( pic, "REAL PIXEL SIZE" );
  if(tsv==NULL)
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
        sscanf( (char *) data, "%lf", &spacing_z );
//        itkGenericOutputMacro( "spacing:  " << spacing_z << " mm");
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0018, 0x0050, &data, &len ) )
      {
        ok=true;
        sscanf( (char *) data, "%lf", &thickness );
//        itkGenericOutputMacro( "thickness: " << thickness << " mm");

        if( thickness == 0 )
          thickness = 1;
      }
      if( dicomFindElement( (unsigned char *) tsv->value, 0x0028, 0x0030, &data, &len )
        && len>0 && ((char *)data)[0] )
      {
        sscanf( (char *) data, "%lf\\%lf", &fy, &fx );    // row / column value 
//        itkGenericOutputMacro( "fx, fy: " << fx << "/" << fy  << " mm");
        return false;
      }
      if(ok)
        FillVector3D(spacing, fx, fy,( spacing_z > 0 ? spacing_z : thickness));
      return ok;
    }
  }
#endif
  if(spacing[0]<=0 || spacing[1]<=0 || spacing[2]<=0)
  {
    itkGenericOutputMacro(<< "illegal spacing by pic tag: " << spacing << ". Setting spacing to (1,1,1).");
    spacing.Fill(1);
  }
  return false;
}

bool mitk::PicHelper::SetSpacing(ipPicDescriptor* pic, SlicedGeometry3D* slicedgeometry)
{
  Vector3D spacing(slicedgeometry->GetSpacing());

  ipPicTSV_t *tsv;
  if ( (tsv = ipPicQueryTag( pic, "REAL PIXEL SIZES" )) != NULL)
  {    
    int count = tsv->n[1];
    float* value = (float*) tsv->value;
    mitk::Vector3D pixelSize;
    spacing.Fill(0);

    for ( int s=0; s < count; s++ ) 
    {
      pixelSize[0] = (ScalarType) *value++;
      pixelSize[1] = (ScalarType) *value++;
      pixelSize[2] = (ScalarType) *value++;
      spacing += pixelSize;
    }
    spacing *= 1.0f/count;
    slicedgeometry->SetSpacing(spacing);

    itkGenericOutputMacro(<< "the slices are inhomogeneous" );
  }
  else
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

  if(pic->dim>=4)
  {
    TimeBounds timebounds;
    timebounds[0] = 0.0;
    timebounds[1] = 1.0;
    slicedgeometry->SetTimeBoundsInMS(timebounds);
  }
}

bool mitk::PicHelper::SetGeometry2D(ipPicDescriptor* pic, int s, SlicedGeometry3D* slicedgeometry)
{
  if((pic!=NULL) && (slicedgeometry->IsValidSlice(s)))
  {
    //construct standard view
    mitk::Point3D origin;
    mitk::Vector3D rightDV, bottomDV;
    ipPicTSV_t *tsv;
    if ( (tsv = ipPicQueryTag( pic, "REAL PIXEL SIZES" )) != NULL)
    {    
      unsigned int count = (unsigned int) tsv->n[1];
      float* value = (float*) tsv->value;
      mitk::Vector3D pixelSize;
      ScalarType zPosition = 0.0f;

      if(s >= 0)
      {
        if(count < (unsigned int) s)
          return false;
        count = s;
      }
      else
      {
        if(count < slicedgeometry->GetSlices())
          return false;
        count = slicedgeometry->GetSlices();
      }

      unsigned int slice;
      for (slice=0; slice < count; ++slice ) 
      {
        pixelSize[0] = (ScalarType) *value++;
        pixelSize[1] = (ScalarType) *value++;
        pixelSize[2] = (ScalarType) *value++;

        zPosition += pixelSize[2] / 2.0f;    // first half slice thickness

        if((s==-1) || (slice==s))
        {
          Vector3D spacing;
          spacing = pixelSize;
          FillVector3D(origin,   0,         0, zPosition);
          FillVector3D(rightDV,  pic->n[0], 0,         0);
          FillVector3D(bottomDV, 0,         pic->n[1], 0);

          mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
          planegeometry->InitializeStandardPlane(rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &spacing);
          planegeometry->SetOrigin(origin);
          slicedgeometry->SetGeometry2D(planegeometry, s);
        }

        zPosition += pixelSize[2] / 2.0f;  // second half slice thickness
      } 

    }
    else
    {
      FillVector3D(origin,0,0,s); slicedgeometry->UnitsToMM(origin, origin);
      FillVector3D(rightDV,pic->n[0],0,0);
      FillVector3D(bottomDV,0,pic->n[1],0);

      mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
      planegeometry->InitializeStandardPlane(rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &slicedgeometry->GetSpacing());
      planegeometry->SetOrigin(origin);
      slicedgeometry->SetGeometry2D(planegeometry, s);
    }
    return true;
  }
  return false;
}
