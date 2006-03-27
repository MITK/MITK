/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkSeedsImage.h"

#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkDrawOperation.h"
#include "mitkImageAccessByItk.h"
#include "mitkInteractionConst.h"

#include "mitkRenderingManager.h"
#include <itkDiscreteGaussianImageFilter.h>

void mitk::SeedsImage::ExecuteOperation(mitk::Operation* operation)
{
  //mitkCheckOperationTypeMacro(SeedsOperation, operation, seedsOp);
  spacing = this->GetGeometry()->GetSpacing();
  for(unsigned int i=0; i<this->GetDimension(); i++)
    orig_size[i] = this->GetDimension(i);

  mitk::DrawOperation * seedsOp = dynamic_cast<mitk::DrawOperation*>( operation );
  if ( seedsOp != NULL )
  {
    m_DrawState = seedsOp->GetDrawState();
    m_Radius = seedsOp->GetRadius();

    switch (operation->GetOperationType())
    {
    case mitk::OpADD:
      { 
        point = seedsOp->GetPoint();
        last_point = point;
        AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpMOVE:
      {
        point = seedsOp->GetPoint();
        AccessByItk(this, AddSeedPoint);
        AccessByItk(this, PointInterpolation);
        last_point = point;
        break;
      }
    case mitk::OpUNDOADD:
      {
        point = seedsOp->GetPoint();
        last_point = point;
        m_DrawState = 0;
        m_Radius = m_Radius+4;  // todo - operation is not equal with its inverse operation - possible approximation problems in the function PointInterpolation()
        AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpUNDOMOVE:
      {
        point = seedsOp->GetPoint();
        m_DrawState = 0;
        m_Radius = m_Radius+4; // todo - operation is not equal with its inverse operation - possible approximation problems in the function PointInterpolation()
        AccessByItk(this, AddSeedPoint);
        AccessByItk(this, PointInterpolation);
        last_point = point;
        break;
      }
    }
    this->Modified();
    
    //*todo has to be done here, cause of update-pipeline not working yet
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    //mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}


template < typename SeedsImageType >
void mitk::SeedsImage::AddSeedPoint(SeedsImageType* itkImage)
{ 
  itk::ImageRegionIterator<SeedsImageType>
  iterator(itkImage, itkImage->GetRequestedRegion());
  const unsigned int dimension = ::itk::GetImageDimension<SeedsImageType>::ImageDimension;
  itk::Index<dimension> baseIndex;
  itk::Index<dimension> setIndex;
  itk::Point<typename SeedsImageType::PixelType, dimension> p;
  p[0] = point[0];
  p[1] = point[1];
  p[2] = point[2];
  itk::ContinuousIndex<typename SeedsImageType::PixelType, dimension> contIndex;
  itkImage->TransformPhysicalPointToContinuousIndex(p, contIndex);
  typename MaskImageType::IndexType getIndex;
  typename MaskImageType::Pointer mask =  MaskImageType::New();

  if (m_DrawState == -1 || m_DrawState == 1)
  {  
  typename MaskImageType::SpacingType spacing;
  spacing.Fill(1);
  double origin[3] = {0,0,0};
  mask->SetOrigin(origin);
  typename MaskImageType::SizeType size;
  size.Fill(2*m_Radius+1);
  typename MaskImageType::IndexType start;
  start.Fill(0);
  typename MaskImageType::RegionType region;
  region.SetIndex(start);
  region.SetSize(size);
  mask->SetRegions(region);
  mask->Allocate();
  mask->FillBuffer(0);
  typename MaskImageType::IndexType idx;
  idx.Fill(m_Radius);
 
  if (m_DrawState == -1)
  {
    mask->SetPixel(idx, -100);
    idx[0] += 1; mask->SetPixel(idx, -100);
    idx[0] -= 2; mask->SetPixel(idx, -100);idx[0] += 1;
    idx[1] += 1; mask->SetPixel(idx, -100);
    idx[1] -= 2; mask->SetPixel(idx, -100);idx[1] += 1;
    idx[2] += 1; mask->SetPixel(idx, -100);
    idx[2] -= 2; mask->SetPixel(idx, -100);idx[2] += 1;
  }
  else if (m_DrawState == 1)
  {
    mask->SetPixel(idx, 100);
    idx[0] += 1; mask->SetPixel(idx, 100);
    idx[0] -= 2; mask->SetPixel(idx, 100);idx[0] += 1;
    idx[1] += 1; mask->SetPixel(idx, 100);
    idx[1] -= 2; mask->SetPixel(idx, 100);idx[1] += 1;
    idx[2] += 1; mask->SetPixel(idx, 100);
    idx[2] -= 2; mask->SetPixel(idx, 100);idx[2] += 1;
  }

    typedef itk::DiscreteGaussianImageFilter<MaskImageType,MaskImageType> BlurFT;
    typename BlurFT::Pointer blurring = BlurFT::New();
    blurring->SetInput( mask );
    float variance[3] = {m_Radius,m_Radius,m_Radius};
    blurring->SetVariance( variance );
    blurring->Update();

    mask = blurring->GetOutput();
    mask->DisconnectPipeline();
  }

  GetGeometry()->WorldToIndex(point, baseIndex); // commented out because of the slices problem
//  for (int i=0; i<3; i++) baseIndex[i] = (int)ceil((point[i]/spacing[i])-(itkImage->GetOrigin()[i]/spacing[i]));
  
  // setting a sphere around the point
  if(dimension==2){
    int radius[dimension];
    for(unsigned int i=0; i<dimension; i++)
      radius[i] = int(m_Radius/spacing[i]);
    //FillVector2D(radius, ((ScalarType)m_Radius)/spacing[0], ((ScalarType)m_Radius)/spacing[1]);
      for(int y = contIndex[1] - radius[1]; y <= contIndex[1] + radius[1]; ++y){
        for(int x = contIndex[0] - radius[0]; x <= contIndex[0] + radius[0]; ++x){
          delta_x = abs(x - contIndex[0])*spacing[0];
          delta_y = abs(y - contIndex[1])*spacing[1];
          sphere_distance = (delta_x * delta_x) + (delta_y * delta_y);
          if (sphere_distance <= (m_Radius*m_Radius)){
            // check -> is the point inside the image?
            if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1])){
              setIndex[1]=y;
              setIndex[0]=x;
              iterator.SetIndex(setIndex);
              iterator.Set(m_DrawState);
            }
          }
        }
      }
  }
  else
  {
//    Vector3D radius;
//    FillVector3D(radius, m_Radius/spacing[0], m_Radius/spacing[1], m_Radius/spacing[2]);
    int radius[dimension];
    for(unsigned int i=0; i<dimension; i++)
      radius[i] = int(m_Radius);//int(m_Radius/spacing[i]);
    for(int z = contIndex[2] - radius[2]; z <= contIndex[2] + radius[2]; ++z){
      for(int y = contIndex[1] - radius[1]; y <= contIndex[1] + radius[1]; ++y){
        for(int x = contIndex[0] - radius[0]; x <= contIndex[0] + radius[0]; ++x){
          delta_x = abs(x - contIndex[0]);//spacing[0];
          delta_y = abs(y - contIndex[1]);//spacing[1];
          delta_z = abs(z - contIndex[2]);//spacing[2];
          sphere_distance = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);
          if (sphere_distance <= (m_Radius*m_Radius)){
            // check -> is the point inside the image?
            if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1]) && (z>=0) && (z<orig_size[2])){
              setIndex[2]=z;
              setIndex[1]=y;
              setIndex[0]=x;
              iterator.SetIndex(setIndex);
              if (m_DrawState == -1 || m_DrawState == 1)
              {  
                getIndex[2]=z + m_Radius -contIndex[2] ;
                getIndex[1]=y + m_Radius -contIndex[1];
                getIndex[0]=x + m_Radius -contIndex[0];
                float val = iterator.Get() + mask->GetPixel(getIndex) ;
                if (val > 128) val = 128; 
                if (val < -128) val = 128; 
                iterator.Set( val );
              }
              else
              {
                iterator.Set(m_DrawState);
              }
            }
          }
        }
      }
    }
  }
}


template < typename SeedsImageType >
void mitk::SeedsImage::PointInterpolation(SeedsImageType* itkImage)
{
  itk::ImageRegionIterator<SeedsImageType >
  iterator(itkImage, itkImage->GetRequestedRegion());
  const unsigned int dimension = ::itk::GetImageDimension<SeedsImageType>::ImageDimension;
  itk::Index<dimension> pointIndex;
  itk::Index<dimension> last_pointIndex;
  itk::Index<dimension> baseIndex;
  itk::Index<dimension> setIndex;
  float point_distance;
  int distance_step;
  int distance_iterator;
  float t;

  // coordinate transformation from physical coordinates to index coordinates
  GetGeometry()->WorldToIndex(point, pointIndex);
  GetGeometry()->WorldToIndex(last_point, last_pointIndex);

//  for (int i=0; i<3; i++) pointIndex[i] = (int)ceil((point[i]/spacing[i])-(itkImage->GetOrigin()[i]/spacing[i]));
//  for (int i=0; i<3; i++) last_pointIndex[i] = (int)ceil((last_point[i]/spacing[i])-(itkImage->GetOrigin()[i]/spacing[i]));

  delta_x = fabsf(last_point[0] - point[0]);
  delta_y = fabsf(last_point[1] - point[1]);
  delta_z = fabsf(last_point[2] - point[2]);            

  // calculation of the distance between last_point and point
  if(dimension==2){
    point_distance = (delta_x * delta_x) + (delta_y * delta_y);   
    distance_step = m_Radius*m_Radius;
    distance_iterator = distance_step;

    // check - is there a gap between the points?
    if(point_distance > distance_step){
      // fill the gap
      while (distance_iterator < point_distance){
        t = distance_iterator/point_distance;
        // interpolation between the points
        for (unsigned int i=0; i<dimension; i++) baseIndex[i] = (int)(((1-t)*last_pointIndex[i]) + (t*pointIndex[i]));

        int radius[dimension];
        for(unsigned int i=0; i<dimension; i++)
          radius[i] = int(m_Radius/spacing[i]);
        for(int y = baseIndex[1] - radius[1]; y <= baseIndex[1] + radius[1]; ++y){
          for(int x = baseIndex[0] - radius[0]; x <= baseIndex[0] + radius[0]; ++x){
            delta_x = fabsf(x - baseIndex[0])*spacing[0];
            delta_y = fabsf(y - baseIndex[1])*spacing[1];
            sphere_distance = (delta_x * delta_x) + (delta_y * delta_y);
            if (sphere_distance <= (m_Radius*m_Radius)){
              // is the point inside the image?
              if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1])){
                setIndex[1]=y;
                setIndex[0]=x;
                iterator.SetIndex(setIndex);
                iterator.Set(m_DrawState);
              }
            }
          }
        }     
      distance_iterator = distance_iterator + distance_step;
      }
    }
  }
  else{ 
    point_distance = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);   
    distance_step = m_Radius*m_Radius;
    distance_iterator = distance_step;

    // check - is there a gap between the points?
    if(point_distance > distance_step){
      // fill the gap
      while (distance_iterator < point_distance){
        t = distance_iterator/point_distance;
        // interpolation between the points
        for (unsigned int i=0; i<dimension; i++) baseIndex[i] = (int)(((1-t)*last_pointIndex[i]) + (t*pointIndex[i]));

//        Vector3D radius;
//        FillVector3D(radius, m_Radius/spacing[0], m_Radius/spacing[1], m_Radius/spacing[2]);
        int radius[dimension];
        for(unsigned int i=0; i<dimension; i++)
          radius[i] = int(m_Radius);
        for(int z = baseIndex[2] - radius[2]; z <= baseIndex[2] + radius[2]; ++z){
          for(int y = baseIndex[1] - radius[1]; y <= baseIndex[1] + radius[1]; ++y){
            for(int x = baseIndex[0] - radius[0]; x <= baseIndex[0] + radius[0]; ++x){
              delta_x = fabsf(x - baseIndex[0]);
              delta_y = fabsf(y - baseIndex[1]);
              delta_z = fabsf(z - baseIndex[2]);
              sphere_distance = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);
              if (sphere_distance <= (m_Radius*m_Radius)){
                // is the point inside the image?
                if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1]) && (z>=0) && (z<orig_size[2])){
                  setIndex[2]=z;
                  setIndex[1]=y;
                  setIndex[0]=x;
                  iterator.SetIndex(setIndex);
                  iterator.Set(m_DrawState);
                }
              }
            }
          }
        }     
      distance_iterator = distance_iterator + distance_step;
      }
    }
  }
}

