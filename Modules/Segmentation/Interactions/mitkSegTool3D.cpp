/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkSegTool3D.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"

mitk::SegTool3D::SegTool3D(const char* type) : Tool(type)
{
}

mitk::SegTool3D::~SegTool3D()
{
}

const char* mitk::SegTool3D::GetGroup() const
{
  return "SegTool3D";
}

void mitk::SegTool3D::PasteSegmentation( Image* targetImage, Image* sourceImage, int pixelvalue, int timestep )
{
  if ((!targetImage)|| (!sourceImage)) return;

  try
  {
    AccessFixedDimensionByItk_2( targetImage, ItkPasteSegmentation, 3, sourceImage, pixelvalue );
  }
  catch( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Could not paste segmentation " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    MITK_ERROR << "Could not generate segmentation.";
    m_ToolManager->ActivateTool(-1);
    return;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::SegTool3D::ItkPasteSegmentation(
       itk::Image<TPixel,VImageDimension>* targetImage, const mitk::Image* sourceImage, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> ImageType;

  typename ImageType::Pointer sourceImageITK;
  CastToItkImage( sourceImage, sourceImageITK );

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      TargetIteratorType;

  typename SourceIteratorType sourceIterator( sourceImageITK, sourceImageITK->GetLargestPossibleRegion() );
  typename TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  const int& activePixelValue = m_ToolManager->GetActiveLabel()->GetIndex();

  if (activePixelValue == 0) // if exterior is the active label
  {
    while ( !targetIterator.IsAtEnd() )
    {
      if (sourceIterator.Get() != 0)
      {
        targetIterator.Set( overwritevalue );
      }
      ++targetIterator;
      ++sourceIterator;
    }
  }
  else if (overwritevalue != 0) // if we are not erasing
  {
    while ( !targetIterator.IsAtEnd() )
    {
      const int targetValue = targetIterator.Get();
      if ( sourceIterator.Get() != 0 )
      {
        if (!m_ToolManager->GetLabelLocked(targetValue))
          targetIterator.Set( overwritevalue );
      }

      ++targetIterator;
      ++sourceIterator;
    }
  }
}
