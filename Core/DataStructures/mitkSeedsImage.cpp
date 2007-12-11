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
}


void mitk::SeedsImage::ExecuteOperation(mitk::Operation* operation)
{
  //mitkCheckOperationTypeMacro(SeedsOperation, operation, seedsOp);
  m_Spacing = this->GetGeometry()->GetSpacing();
  for(unsigned int i=0; i<this->GetDimension(); i++)
    orig_size[i] = this->GetDimension(i);

  mitk::DrawOperation * seedsOp = 
    dynamic_cast< mitk::DrawOperation * >( operation );

  if ( seedsOp != NULL )
  {
    m_DrawState = seedsOp->GetDrawState();
  
    if (m_Radius != seedsOp->GetRadius())
    {
      m_Radius = seedsOp->GetRadius();
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
        
        // todo - operation is not equal with its inverse operation - possible 
        // approximation problems in the function PointInterpolation()
        m_Radius = m_Radius+4;
        AccessByItk(this, AddSeedPoint);
        break;
      }
    case mitk::OpUNDOMOVE:
      {
        m_Point = seedsOp->GetPoint();
        m_DrawState = 0;
        
        // todo - operation is not equal with its inverse operation - possible 
        // approximation problems in the function PointInterpolation()
        m_Radius = m_Radius+4;
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
  typedef typename NeighborhoodIteratorType::IndexType IndexType;

  NeighborhoodIteratorType& nit = this->GetNit< SeedsImageType >( itkImage );

  const unsigned int dimension =
    ::itk::GetImageDimension<SeedsImageType>::ImageDimension;

  mitk::Point3D index;
  this->GetGeometry()->WorldToIndex( m_Point, index );

  IndexType itkIndex;
  unsigned int d;
  for ( d = 0; d < dimension; ++d )
  {
    itkIndex[d] = (int)(index[d] + 0.5);
  }
  nit.SetLocation( itkIndex );

  
  unsigned int i;
  for ( i = 0; i < nit.Size(); ++i )
  {
    if ( nit[i] != 0 )
    {
      try
      {
        nit.SetPixel( i, m_DrawState );
      }
      catch( itk::RangeError & )
      {
      }
    }
  }
}


template < typename SeedsImageType >
void mitk::SeedsImage::PointInterpolation(SeedsImageType* itkImage)
{
  typedef itk::NeighborhoodIterator< SeedsImageType >
    NeighborhoodIteratorType;
  typedef typename NeighborhoodIteratorType::IndexType IndexType;


  NeighborhoodIteratorType& nit = this->GetNit< SeedsImageType >( itkImage );

  const unsigned int dimension = 
    ::itk::GetImageDimension<SeedsImageType>::ImageDimension;

  mitk::Point3D indexBegin, indexEnd;
  this->GetGeometry()->WorldToIndex( m_Point, indexBegin );
  this->GetGeometry()->WorldToIndex( m_LastPoint, indexEnd );

  IndexType itkIndexBegin, itkIndexEnd;
  unsigned int d;
  for ( d = 0; d < dimension; ++d )
  {
    itkIndexBegin[d] = (int)(indexBegin[d] + 0.5);
    itkIndexEnd[d] = (int)(indexEnd[d] + 0.5);
  }


  typedef itk::LineConstIterator< SeedsImageType > LineIteratorType;
  LineIteratorType lit( itkImage, itkIndexBegin, itkIndexEnd );

  // Disable warnings (which would otherwise be displayed if line leaves the
  // region).
  bool warningDisplay = itk::Object::GetGlobalWarningDisplay();
  itk::Object::GlobalWarningDisplayOff();
  for ( lit.GoToBegin(); !lit.IsAtEnd(); ++lit )
  {
    nit.SetLocation( lit.GetIndex() );

    unsigned int i;
    for ( i = 0; i < nit.Size(); ++i )
    {
      if ( nit[i] != 0 )
      {
        try
        {
          nit.SetPixel( i, m_DrawState );
        }
        catch( itk::RangeError & )
        {
        }
      }
    }
  }
  itk::Object::SetGlobalWarningDisplay( warningDisplay );
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

template < typename SeedsImageType >
itk::NeighborhoodIterator< SeedsImageType >&
mitk::SeedsImage::GetNit( SeedsImageType* image )
{
  typedef itk::NeighborhoodIterator< SeedsImageType >
    NeighborhoodIteratorType;
  typedef typename NeighborhoodIteratorType::OffsetType OffsetType;
  typedef typename NeighborhoodIteratorType::SizeType SizeType;
  typedef itk::GaussianSpatialFunction< int, SeedsImageType::ImageDimension >
    GaussianFunctionType;

  static SeedsImageType* iteratedImage = 0;
  static NeighborhoodIteratorType nit;
  static typename GaussianFunctionType::Pointer gaussianFunction
    = GaussianFunctionType::New();

  if ( iteratedImage != image )
  {
    SizeType radius;
    radius.Fill( m_Radius);
    nit.Initialize( radius, image, image->GetBufferedRegion() );
    iteratedImage = image;
  }

  nit.SetRadius( m_Radius );

  unsigned int i;
  for ( i = 0; i < nit.GetCenterNeighborhoodIndex()*2+1; ++i )
  {
    OffsetType offset = nit.GetOffset( i );

    typename GaussianFunctionType::InputType point;
    double dist = 0;
    unsigned int d;
    for ( d = 0; d < SeedsImageType::ImageDimension; ++d )
    {
      point[d] = offset[d];
      dist += offset[d] * offset[d];
    }
  }

  return nit;
}

