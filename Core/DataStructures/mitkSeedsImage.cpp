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

#include <itkNeighborhoodIterator.h>
#include <itkLineConstIterator.h>


mitk::SeedsImage::SeedsImage()
{
  m_GaussianFunction3D = GaussianFunction3DType::New();
  m_GaussianFunction2D = GaussianFunction2DType::New();
}

mitk::SeedsImage::~SeedsImage()
{
}

void mitk::SeedsImage::Initialize() 
{
  m_Radius = 1;
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
      m_Radius = 1;
      //CreateBrush();
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
  typedef itk::NeighborhoodIterator< SeedsImageType >
    NeighborhoodIteratorType;
  typedef NeighborhoodIteratorType::IndexType IndexType;

  NeighborhoodIteratorType& nit = this->GetNit< SeedsImageType >( itkImage );

  const unsigned int dimension = ::itk::GetImageDimension<SeedsImageType>::ImageDimension;

  mitk::Point3D index;
  this->GetGeometry()->WorldToIndex( m_Point, index );

  IndexType itkIndex;
  int d;
  for ( d = 0; d < dimension; ++d )
  {
    itkIndex[d] = (int)(index[d] + 0.5);
  }
  nit.SetLocation( itkIndex );

  
  int i;
  for ( i = 0; i < nit.Size(); ++i )
  {
    if ( nit[i] != 0 )
    {
      nit.SetPixel( i, m_DrawState );
    }
  }
}


template < typename SeedsImageType >
void mitk::SeedsImage::PointInterpolation(SeedsImageType* itkImage)
{
  typedef itk::NeighborhoodIterator< SeedsImageType >
    NeighborhoodIteratorType;
  typedef NeighborhoodIteratorType::IndexType IndexType;

  NeighborhoodIteratorType& nit = this->GetNit< SeedsImageType >( itkImage );

  const unsigned int dimension = ::itk::GetImageDimension<SeedsImageType>::ImageDimension;

  mitk::Point3D indexBegin, indexEnd;
  this->GetGeometry()->WorldToIndex( m_Point, indexBegin );
  this->GetGeometry()->WorldToIndex( m_LastPoint, indexEnd );

  IndexType itkIndexBegin, itkIndexEnd;
  int d;
  for ( d = 0; d < dimension; ++d )
  {
    itkIndexBegin[d] = (int)(indexBegin[d] + 0.5);
    itkIndexEnd[d] = (int)(indexEnd[d] + 0.5);
  }


  typedef itk::LineConstIterator< SeedsImageType > LineIteratorType;
  LineIteratorType lit( itkImage, itkIndexBegin, itkIndexEnd );

  for ( lit.GoToBegin(); !lit.IsAtEnd(); ++lit )
  {
    nit.SetLocation( lit.GetIndex() );

    int i;
    for ( i = 0; i < nit.Size(); ++i )
    {
      if ( nit[i] != 0 )
      {
        nit.SetPixel( i, m_DrawState );
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

template < int Dimension >
itk::Neighborhood< float, Dimension >&
mitk::SeedsImage::GetBrush()
{
  static initialized = false;
  static itk::Neighborhood< float, Dimension > brush;

  if ( ! initialized )
  {

    initialized = true;
  }
  return brush;
}

template < typename SeedsImageType >
itk::NeighborhoodIterator< SeedsImageType >&
mitk::SeedsImage::GetNit( SeedsImageType* image )
{
  typedef itk::NeighborhoodIterator< SeedsImageType >
    NeighborhoodIteratorType;
  typedef NeighborhoodIteratorType::OffsetType OffsetType;
  typedef NeighborhoodIteratorType::SizeType SizeType;
  typedef itk::GaussianSpatialFunction< int, SeedsImageType::ImageDimension >
    GaussianFunctionType;

  static initialized = false;
  static SeedsImageType* iteratedImage = 0;
  static NeighborhoodIteratorType nit;
  static GaussianFunctionType::Pointer gaussianFunction
    = GaussianFunctionType::New();

  if ( iteratedImage != image )
  {
    SizeType radius;
    radius.Fill( m_Radius);
    nit.Initialize( radius, image, image->GetBufferedRegion() );
    iteratedImage = image;
  }

  if ( !initialized )
  {
    nit.SetRadius( m_Radius );

    int i;
    for ( i = 0; i < nit.GetCenterNeighborhoodIndex()*2+1; ++i )
    {
      OffsetType offset = nit.GetOffset( i );

      GaussianFunctionType::InputType point;
      double dist = 0;
      int d;
      for ( d = 0; d < SeedsImageType::ImageDimension; ++d )
      {
        point[d] = offset[d];
        dist += offset[d] * offset[d];
      }
      /*
      if ( dist <= m_Radius*m_Radius )
      {
        nit.ActivateOffset( nit.GetOffset( i ) );
        //*nit[i] = gaussianFunction->Evaluate( point );
      }
      else
      {
        nit.DeactivateOffset( nit.GetOffset( i ) );
        //*nit[i] = 0;
      }
      */
    }
    initialized = true;
  }

  return nit;
}

void mitk::SeedsImage::CreateBrush()
{
  // Initializie structuring element (brush form)
  m_StructuringElement3D.SetRadius( m_Radius );
  m_StructuringElement3D.CreateStructuringElement();

  m_StructuringElement2D.SetRadius( m_Radius );
  m_StructuringElement2D.CreateStructuringElement();

  // Create brushes
  m_Brush3D.SetRadius( m_Radius );

  GaussianFunction3DType::ArrayType sigma3D;
  sigma3D.Fill( m_Radius );
  GaussianFunction3DType::ArrayType mean3D;
  mean3D.Fill( m_Radius );

  m_GaussianFunction3D->SetScale( 100.0 );
  m_GaussianFunction3D->SetSigma( sigma3D );
  m_GaussianFunction3D->SetMean( mean3D );

  m_Brush2D.SetRadius( m_Radius );

  GaussianFunction2DType::ArrayType sigma2D;
  sigma2D.Fill( m_Radius );
  GaussianFunction2DType::ArrayType mean2D;
  mean2D.Fill( m_Radius );

  m_GaussianFunction2D->SetScale( 100.0 );
  m_GaussianFunction2D->SetSigma( sigma2D );
  m_GaussianFunction2D->SetMean( mean2D );

  Brush3DType::OffsetType offset3D;
  GaussianFunction3DType::InputType point3D;
  Brush2DType::OffsetType offset2D;
  GaussianFunction2DType::InputType point2D;
  int x, y, z;
  for ( x = 0; x < m_Radius*2+1; ++x )
  {
    offset3D[0] = point3D[0] = x;
    offset2D[0] = point2D[0] = x;
    for ( y = 0; y < m_Radius*2+1; ++y )
    {
      offset3D[1] = point3D[1] = y;
      offset2D[1] = point2D[1] = y;
      m_Brush2D[offset2D] = m_GaussianFunction2D->Evaluate( point2D );
      for ( z = 0; z < m_Radius*2+1; ++z )
      {
        offset3D[2] = z;
        point3D[2] = z;
        m_Brush3D[offset3D] = m_GaussianFunction3D->Evaluate( point3D );
      }
    }
  }


  
  /*
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

  typedef itk::DiscreteGaussianImageFilter<MaskImageType,MaskImageType> BlurFT;
  BlurFT::Pointer blurring = BlurFT::New();
  blurring->SetInput( m_Brush );
  float variance[3] = {m_Radius,m_Radius,m_Radius};
  blurring->SetVariance( variance );
  blurring->Update();

  m_Brush = blurring->GetOutput();
  m_Brush->DisconnectPipeline();
  */
}
