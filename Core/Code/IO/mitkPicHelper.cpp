/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkConfig.h"
#include "mitkPicHelper.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#ifdef HAVE_IPDICOM
#include "ipDicom/ipDicom.h"
#endif /* HAVE_IPDICOM */

bool mitk::PicHelper::GetSpacing(const mitkIpPicDescriptor* aPic, Vector3D & spacing)
{
  mitkIpPicDescriptor* pic = const_cast<mitkIpPicDescriptor*>(aPic);

  mitkIpPicTSV_t *tsv;
  bool pixelSize = false;

  tsv = mitkIpPicQueryTag( pic, "REAL PIXEL SIZE" );
  if(tsv==NULL)
  {
    tsv = mitkIpPicQueryTag( pic, "PIXEL SIZE" );
    pixelSize = true;
  }
  if(tsv)
  {
    bool tagFound = false;
    if((tsv->dim*tsv->n[0]>=3) && (tsv->type==mitkIpPicFloat))
    {
      if(tsv->bpe==32)
      {
        FillVector3D(spacing,((mitkIpFloat4_t*)tsv->value)[0], ((mitkIpFloat4_t*)tsv->value)[1],((mitkIpFloat4_t*)tsv->value)[2]);
        tagFound = true;
      }
      else
      if(tsv->bpe==64)
      {
        FillVector3D(spacing,((mitkIpFloat8_t*)tsv->value)[0], ((mitkIpFloat8_t*)tsv->value)[1],((mitkIpFloat8_t*)tsv->value)[2]);
        tagFound = true;
      }
    }
    if(tagFound && pixelSize)
    {
      tsv = mitkIpPicQueryTag( pic, "PIXEL SPACING" );
      if(tsv)
      {
        mitk::ScalarType zSpacing = 0;
        if((tsv->dim*tsv->n[0]>=3) && (tsv->type==mitkIpPicFloat))
        {
          if(tsv->bpe==32)
          {
            zSpacing = ((mitkIpFloat4_t*)tsv->value)[2];
          }
          else
          if(tsv->bpe==64)
          {
            zSpacing = ((mitkIpFloat8_t*)tsv->value)[2];
          }
          if(zSpacing != 0)
          {
            spacing[2] = zSpacing;
          }
        }
      }
    }
    if(tagFound) return true;
  }
#ifdef HAVE_IPDICOM
  tsv = mitkIpPicQueryTag( pic, "SOURCE HEADER" );
  if( tsv )
  {
    void *data;
    mitkIpUInt4_t len;
    mitkIpFloat8_t spacing_z = 0;
    mitkIpFloat8_t thickness = 1;
    mitkIpFloat8_t fx = 1;
    mitkIpFloat8_t fy = 1;
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
    }
    else
      ok=false;
    if(ok)
      FillVector3D(spacing, fx, fy,( spacing_z > 0 ? spacing_z : thickness));
    return ok;
  }
#endif /* HAVE_IPDICOM */
  if(spacing[0]<=0 || spacing[1]<=0 || spacing[2]<=0)
  {
    itkGenericOutputMacro(<< "illegal spacing by pic tag: " << spacing << ". Setting spacing to (1,1,1).");
    spacing.Fill(1);
  }
  return false;
}

bool mitk::PicHelper::GetTimeSpacing(const mitkIpPicDescriptor* aPic, float& timeSpacing)
{

  mitkIpPicDescriptor* pic = const_cast<mitkIpPicDescriptor*>(aPic);

  mitkIpPicTSV_t *tsv;

  tsv = mitkIpPicQueryTag( pic, "PIXEL SIZE" );
  if(tsv)
  {
    timeSpacing = ((mitkIpFloat4_t*)tsv->value)[3];
    if( timeSpacing <=0 ) timeSpacing = 1;
  }
  else timeSpacing = 1;
  return true;
}

bool mitk::PicHelper::SetSpacing(const mitkIpPicDescriptor* aPic, SlicedGeometry3D* slicedgeometry)
{
  mitkIpPicDescriptor* pic = const_cast<mitkIpPicDescriptor*>(aPic);

  Vector3D spacing(slicedgeometry->GetSpacing());

  mitkIpPicTSV_t *tsv;
  if ( (tsv = mitkIpPicQueryTag( pic, "REAL PIXEL SIZES" )) != NULL)
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

void mitk::PicHelper::InitializeEvenlySpaced(const mitkIpPicDescriptor* pic, unsigned int slices, SlicedGeometry3D* slicedgeometry)
{
  assert(pic!=NULL);
  assert(slicedgeometry!=NULL);

  mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();

  mitkIpPicTSV_t *geometryTag;
  if ( (geometryTag = mitkIpPicQueryTag( const_cast<mitkIpPicDescriptor*>(pic), "ISG" )) != NULL)
  {    
    mitk::Point3D  origin;
    mitk::Vector3D rightVector;
    mitk::Vector3D downVector;
    mitk::Vector3D spacing;

    mitk::vtk2itk(((float*)geometryTag->value+0), origin);
    mitk::vtk2itk(((float*)geometryTag->value+3), rightVector);
    mitk::vtk2itk(((float*)geometryTag->value+6), downVector);
    mitk::vtk2itk(((float*)geometryTag->value+9), spacing);

    mitk::PlaneGeometry::Pointer planegeometry = PlaneGeometry::New();
    planegeometry->InitializeStandardPlane(pic->n[0], pic->n[1], rightVector, downVector, &spacing);
    planegeometry->SetOrigin(origin);
    slicedgeometry->InitializeEvenlySpaced(planegeometry, slices);    
  }
  else
  {
    Vector3D spacing;
    spacing.Fill(1);
    GetSpacing(pic, spacing);
    planegeometry->InitializeStandardPlane(pic->n[0], pic->n[1], spacing);
    slicedgeometry->InitializeEvenlySpaced(planegeometry, spacing[2], slices);
  }

  if(pic->dim>=4)
  {
    float ts = 0;
    GetTimeSpacing(pic, ts);
    TimeBounds timebounds;
    timebounds[0] = 0.0;
    timebounds[1] = ts;
    slicedgeometry->SetTimeBounds(timebounds);
  }
}

bool mitk::PicHelper::SetGeometry2D(const mitkIpPicDescriptor* aPic, int s, SlicedGeometry3D* slicedgeometry)
{
  mitkIpPicDescriptor* pic = const_cast<mitkIpPicDescriptor*>(aPic);
  if((pic!=NULL) && (slicedgeometry->IsValidSlice(s)))
  {
    //construct standard view
    mitk::Point3D origin;
    mitk::Vector3D rightDV, bottomDV;
    mitkIpPicTSV_t *tsv;
    if ( (tsv = mitkIpPicQueryTag( pic, "REAL PIXEL SIZES" )) != NULL)
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

        if((s==-1) || (slice== (unsigned int) s))
        {
          Vector3D spacing;
          spacing = pixelSize;
          FillVector3D(origin,   0,         0, zPosition);
          FillVector3D(rightDV,  pic->n[0], 0,         0);
          FillVector3D(bottomDV, 0,         pic->n[1], 0);

          mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
          planegeometry->InitializeStandardPlane(pic->n[0], pic->n[1], rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &spacing);
          planegeometry->SetOrigin(origin);
          slicedgeometry->SetGeometry2D(planegeometry, s);
        }

        zPosition += pixelSize[2] / 2.0f;  // second half slice thickness
      } 

    }
    else
    {
      FillVector3D(origin,0,0,s); slicedgeometry->IndexToWorld(origin, origin);
      FillVector3D(rightDV,pic->n[0],0,0);
      FillVector3D(bottomDV,0,pic->n[1],0);

      mitk::PlaneGeometry::Pointer planegeometry=mitk::PlaneGeometry::New();
      planegeometry->InitializeStandardPlane(pic->n[0], pic->n[1], rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &slicedgeometry->GetSpacing());
      planegeometry->SetOrigin(origin);
      slicedgeometry->SetGeometry2D(planegeometry, s);
    }
    return true;
  }
  return false;
}
