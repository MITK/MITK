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

void mitk::SeedsImage::Initialize() 
{
  m_Radius = -1;
  std::cout << "seeds image radius reset" << std::endl;
}


void mitk::SeedsImage::ExecuteOperation(mitk::Operation* operation)
{
  //mitkCheckOperationTypeMacro(SeedsOperation, operation, seedsOp);
  m_Spacing = this->GetGeometry()->GetSpacing();
  for(unsigned int i=0; i<this->GetDimension(); i++)
    orig_size[i] = this->GetDimension(i);

  mitk::DrawOperation * seedsOp = dynamic_cast<mitk::DrawOperation*>( operation );

  if ( seedsOp != NULL )
  {
    m_DrawState = seedsOp->GetDrawState();
  
    if (m_Radius != seedsOp->GetRadius())
    {
      m_Radius = seedsOp->GetRadius();
      CreateBrush();
    }

    switch (operation->GetOperationType())
    {
    case mitk::OpADD:
      { 
        m_Point = seedsOp->GetPoint();
        m_LastPoint = m_Point;
        AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpMOVE:
      {
        m_Point = seedsOp->GetPoint();
        AccessByItk(this, AddSeedPoint);
        AccessByItk(this, PointInterpolation);
        m_LastPoint = m_Point;
        break;
      }
    case mitk::OpUNDOADD:
      {
        m_Point = seedsOp->GetPoint();
        m_LastPoint = m_Point;
        m_DrawState = 0;
        m_Radius = m_Radius+4;  // todo - operation is not equal with its inverse operation - possible approximation problems in the function PointInterpolation()
        AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpUNDOMOVE:
      {
        m_Point = seedsOp->GetPoint();
        m_DrawState = 0;
        m_Radius = m_Radius+4; // todo - operation is not equal with its inverse operation - possible approximation problems in the function PointInterpolation()
        AccessByItk(this, AddSeedPoint);
        AccessByItk(this, PointInterpolation);
        m_LastPoint = m_Point;
        break;
      }
    }
    
    //*todo has to be done here, cause of update-pipeline not working yet
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    //mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

    this->Modified();
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

  typedef SeedsImageType::PixelType CoordType;

  itk::Point<CoordType, dimension> p;
  p[0] = (CoordType) m_Point[0];
  p[1] = (CoordType) m_Point[1];
  p[2] = (CoordType) m_Point[2];
  itk::ContinuousIndex<typename SeedsImageType::PixelType, dimension> contIndex;
  itkImage->TransformPhysicalPointToContinuousIndex(p, contIndex);

  GetGeometry()->WorldToIndex(m_Point, baseIndex); // commented out because of the slices problem
//  for (int i=0; i<3; i++) baseIndex[i] = (int)ceil((point[i]/m_Spacing[i])-(itkImage->GetOrigin()[i]/m_Spacing[i]));
  
  // setting a sphere around the point
  if(dimension==2){
    int radius[dimension];
    for(unsigned int i=0; i<dimension; i++)
      radius[i] = int(m_Radius/m_Spacing[i]);
    //FillVector2D(radius, ((ScalarType)m_Radius)/m_Spacing[0], ((ScalarType)m_Radius)/m_Spacing[1]);
      for(int y = (int)(contIndex[1] - radius[1]); y <= (int)(contIndex[1] + radius[1]); ++y){
        for(int x = (int)(contIndex[0] - radius[0]); x <= (int)(contIndex[0] + radius[0]); ++x){
          delta_x = fabs((float)(x - contIndex[0]))*m_Spacing[0];
          delta_y = fabs((float)(y - contIndex[1]))*m_Spacing[1];
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
    MaskImageType::IndexType getIndex;
//    Vector3D radius;
//    FillVector3D(radius, m_Radius/m_Spacing[0], m_Radius/m_Spacing[1], m_Radius/m_Spacing[2]);
    int radius[dimension];
    for(unsigned int i=0; i<dimension; i++)
      radius[i] = int(m_Radius);//int(m_Radius/m_Spacing[i]);
    for(int z = (int)(contIndex[2] - radius[2]); z <= (int)(contIndex[2] + radius[2]); ++z){
      for(int y = (int)(contIndex[1] - radius[1]); y <= (int)(contIndex[1] + radius[1]); ++y){
        for(int x = (int)(contIndex[0] - radius[0]); x <= (int)(contIndex[0] + radius[0]); ++x){
          delta_x = fabs((float)x - contIndex[0])*m_Spacing[0];
          delta_y = fabs((float)y - contIndex[1])*m_Spacing[1];
          delta_z = fabs((float)z - contIndex[2])*m_Spacing[2];
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
                getIndex[2]=(MaskImageType::IndexType::IndexValueType) (z + m_Radius -contIndex[2]);
                getIndex[1]=(MaskImageType::IndexType::IndexValueType) (y + m_Radius -contIndex[1]);
                getIndex[0]=(MaskImageType::IndexType::IndexValueType) (x + m_Radius -contIndex[0]);
                float val = m_Brush->GetPixel(getIndex);
                if (m_DrawState == -1) 
                {
                  val *= -1.0;
                }
                val += iterator.Get();
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
  //itk::Index<dimension> pointIndex;
  //itk::Index<dimension> last_pointIndex;
  itk::Index<dimension> baseIndex;
  itk::Index<dimension> setIndex;
  float point_distance;
  int distance_step;
  int distance_iterator;
  float t;

  itk::Point<typename SeedsImageType::PixelType, dimension> p;
  p[0] = m_Point[0];
  p[1] = m_Point[1];
  p[2] = m_Point[2];
  itk::ContinuousIndex<typename SeedsImageType::PixelType, dimension> pointIndex;
  itkImage->TransformPhysicalPointToContinuousIndex(p, pointIndex);
  p[0] = m_LastPoint[0];
  p[1] = m_LastPoint[1];
  p[2] = m_LastPoint[2];
  itk::ContinuousIndex<typename SeedsImageType::PixelType, dimension> last_pointIndex;
  itkImage->TransformPhysicalPointToContinuousIndex(p, last_pointIndex);

  // coordinate transformation from physical coordinates to index coordinates
  //GetGeometry()->WorldToIndex(m_Point, pointIndex);
  //GetGeometry()->WorldToIndex(m_LastPoint, last_pointIndex);

//  for (int i=0; i<3; i++) pointIndex[i] = (int)ceil((point[i]/m_Spacing[i])-(itkImage->GetOrigin()[i]/m_Spacing[i]));
//  for (int i=0; i<3; i++) last_pointIndex[i] = (int)ceil((last_point[i]/m_Spacing[i])-(itkImage->GetOrigin()[i]/m_Spacing[i]));

  delta_x = fabsf(m_LastPoint[0] - m_Point[0]);
  delta_y = fabsf(m_LastPoint[1] - m_Point[1]);
  delta_z = fabsf(m_LastPoint[2] - m_Point[2]);            

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
          radius[i] = int(m_Radius/m_Spacing[i]);
        for(int y = baseIndex[1] - radius[1]; y <= baseIndex[1] + radius[1]; ++y){
          for(int x = baseIndex[0] - radius[0]; x <= baseIndex[0] + radius[0]; ++x){
            delta_x = fabsf(x - baseIndex[0])*m_Spacing[0];
            delta_y = fabsf(y - baseIndex[1])*m_Spacing[1];
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
    MaskImageType::IndexType getIndex;

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
//        FillVector3D(radius, m_Radius/m_Spacing[0], m_Radius/m_Spacing[1], m_Radius/m_Spacing[2]);
        int radius[dimension];
        for(unsigned int i=0; i<dimension; i++)
          radius[i] = int(m_Radius);
        for(int z = baseIndex[2] - radius[2]; z <= baseIndex[2] + radius[2]; ++z){
          for(int y = baseIndex[1] - radius[1]; y <= baseIndex[1] + radius[1]; ++y){
            for(int x = baseIndex[0] - radius[0]; x <= baseIndex[0] + radius[0]; ++x){
              delta_x = fabsf(x - baseIndex[0])*m_Spacing[2];
              delta_y = fabsf(y - baseIndex[1])*m_Spacing[2];
              delta_z = fabsf(z - baseIndex[2])*m_Spacing[2];
              sphere_distance = (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z);
              if (sphere_distance <= (m_Radius*m_Radius)){
                // is the point inside the image?
                if ((x>=0) && (x<=orig_size[0]) && (y>=0) && (y<=orig_size[1]) && (z>=0) && (z<orig_size[2])){
                  setIndex[2]=z;
                  setIndex[1]=y;
                  setIndex[0]=x;
                  iterator.SetIndex(setIndex);
                  if (m_DrawState == -1 || m_DrawState == 1)
                  {  
                    getIndex[2]= (MaskImageType::IndexType::IndexValueType) (z + m_Radius -baseIndex[2]);
                    getIndex[1]= (MaskImageType::IndexType::IndexValueType) (y + m_Radius -baseIndex[1]);
                    getIndex[0]= (MaskImageType::IndexType::IndexValueType) (x + m_Radius -baseIndex[0]);
                    float val = m_Brush->GetPixel(getIndex);
                    if (m_DrawState == -1) 
                    {
                      val *= -1.0;
                    }
                    val += iterator.Get();
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
      distance_iterator = distance_iterator + distance_step;
      }
    }
  }
}

void mitk::SeedsImage::ClearBuffer()
{
  AccessByItk(this, ClearBuffer);
}

template < typename SeedsImageType >
void mitk::SeedsImage::ClearBuffer(SeedsImageType* itkImage)
{
  itkImage->FillBuffer(0);
}


void mitk::SeedsImage::CreateBrush()
{
  m_Brush =  MaskImageType::New();
  MaskImageType::SpacingType spacing;
  spacing.Fill(1);
  double origin[3] = {m_Spacing[0],m_Spacing[1],m_Spacing[2]};
  m_Brush->SetOrigin(origin);
  MaskImageType::SizeType size;
  size.Fill(2*m_Radius+1);
  MaskImageType::IndexType start;
  start.Fill(0);
  MaskImageType::RegionType region;
  region.SetIndex(start);
  region.SetSize(size);
  m_Brush->SetRegions(region);
  m_Brush->Allocate();
  m_Brush->FillBuffer(0);
  MaskImageType::IndexType idx;
  idx.Fill(m_Radius);

  MaskImageType::IndexType center;
  center.Fill(m_Radius);
  MaskImageType::IndexType half;
  half.Fill( (unsigned long)(ceil(m_Radius/2.0)) );

  for (idx[0] = center[0]-half[0]; idx[0] < center[0]+half[0]; idx[0]++)   
    for (idx[1] = center[1]-half[1]; idx[1] < center[1]+half[1]; idx[1]++)
      for (idx[2] = center[2]-half[2]; idx[2] < center[2]+half[2]; idx[2]++)
  {
    m_Brush->SetPixel(idx, 100);
  }

  //m_Brush->SetPixel(idx, 100);
  //idx[0] += 1; m_Brush->SetPixel(idx, 100);
  //idx[0] -= 2; m_Brush->SetPixel(idx, 100);idx[0] += 1;
  //idx[1] += 1; m_Brush->SetPixel(idx, 100);
  //idx[1] -= 2; m_Brush->SetPixel(idx, 100);idx[1] += 1;
  //idx[2] += 1; m_Brush->SetPixel(idx, 100);
  //idx[2] -= 2; m_Brush->SetPixel(idx, 100);idx[2] += 1;

  typedef itk::DiscreteGaussianImageFilter<MaskImageType,MaskImageType> BlurFT;
  BlurFT::Pointer blurring = BlurFT::New();
  blurring->SetInput( m_Brush );
  float variance[3] = {m_Radius,m_Radius,m_Radius};
  blurring->SetVariance( variance );
  blurring->Update();

  m_Brush = blurring->GetOutput();
  m_Brush->DisconnectPipeline();
}
