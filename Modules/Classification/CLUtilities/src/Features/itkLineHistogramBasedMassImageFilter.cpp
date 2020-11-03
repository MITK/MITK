/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ITKLINEHISTOGRAMBASEDMASSIMAGEFILTER_CPP
#define ITKLINEHISTOGRAMBASEDMASSIMAGEFILTER_CPP

#include <itkLineHistogramBasedMassImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkBinaryContourImageFilter.h>
#include <mitkImageCast.h>
#include <mitkIOUtil.h>


template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>
::BeforeThreadedGenerateData()
{

  if(m_ImageMask.IsNull())
  {
    itkExceptionMacro("No binary mask image provided.")
  }

  if(m_BinaryContour.IsNull()){


    typename TMaskImageType::RegionType region = m_ImageMask->GetLargestPossibleRegion();
    unsigned int xdim = region.GetSize(0);
    unsigned int ydim = region.GetSize(1);
    unsigned int zdim = region.GetSize(2);

    // ensure border pixels are zero so that an closed contour is created
    typename TMaskImageType::Pointer mask_copy = TMaskImageType::New();
    mask_copy->SetSpacing(m_ImageMask->GetSpacing());
    mask_copy->SetDirection(m_ImageMask->GetDirection());
    mask_copy->SetOrigin(m_ImageMask->GetOrigin());
    mask_copy->SetRegions(region);
    mask_copy->Allocate();
    mask_copy->FillBuffer(0);

    itk::ImageRegionIteratorWithIndex<TMaskImageType> oit(mask_copy, mask_copy->GetLargestPossibleRegion());
    itk::ImageRegionConstIteratorWithIndex<TMaskImageType> mit(m_ImageMask, m_ImageMask->GetLargestPossibleRegion());
    while(!mit.IsAtEnd())
    {
      if(mit.Value() != 0 //is under the mask
         && mit.GetIndex()[0] != 0 //is not at the border
         && mit.GetIndex()[1] != 0
         && mit.GetIndex()[2] != 0
         && mit.GetIndex()[0] != xdim-1
         && mit.GetIndex()[1] != ydim-1
         && mit.GetIndex()[2] != zdim-1)
      {
        oit.Set(1);
      }
      ++mit;
      ++oit;
    }

    typedef itk::BinaryContourImageFilter<TMaskImageType,TMaskImageType> BinaryContourImagefilterType;
    typename BinaryContourImagefilterType::Pointer filter = BinaryContourImagefilterType::New();
    filter->SetInput(mask_copy);
    filter->SetBackgroundValue(0);
    filter->SetForegroundValue(1);
    filter->SetFullyConnected(true);
    filter->Update();

    m_BinaryContour = filter->GetOutput();
    mitk::Image::Pointer outimg;
    mitk::CastToMitkImage(m_BinaryContour,outimg);
  }

  if(m_BinaryContour.IsNull())
  {
    itkExceptionMacro("No binary contour image provided.")
  }

  m_CenterOfMask = GetCenterOfMass(m_ImageMask);

  if(m_CenterOfMask.is_zero())
  {
    itkExceptionMacro("Center of mass is corrupt.")
  }


}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
vnl_vector<double> itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>
::GetCenterOfMass(const TMaskImageType * maskImage)
{
  mitk::Image::Pointer img;
  mitk::CastToMitkImage(maskImage, img);

  typedef itk::ImageRegionConstIterator< TMaskImageType > IteratorType;
  IteratorType iter( maskImage, maskImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  vnl_vector_fixed<double,3> mean_index;
  mean_index.fill(0);
  unsigned int count = 0;
  while ( !iter.IsAtEnd() )
  {
    if ( iter.Get() != 0 )
    {
      mitk::Point3D current_index_pos;
      img->GetGeometry()->IndexToWorld(iter.GetIndex(),current_index_pos);
      mean_index += current_index_pos.GetVnlVector();
      count++;
    }
    ++iter;
  }

  mean_index /= count;
  return mean_index;
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>
::ThreadedGenerateData(const typename Superclass::OutputImageRegionType &outputRegionForThread, ThreadIdType /*threadId*/)
{

  TOutputImageType * outimage = this->GetOutput();


  itk::ImageRegionConstIteratorWithIndex<TMaskImageType> cit(m_BinaryContour, outputRegionForThread);
  itk::ImageRegionConstIteratorWithIndex<TInputImageType> iit(this->GetInput(), outputRegionForThread);
  itk::ImageRegionIteratorWithIndex<TOutputImageType > oit(outimage,outputRegionForThread);
  typedef typename itk::ImageRegionIteratorWithIndex<TInputImageType >::IndexType IndexType;

  std::vector<IndexType> target_world_indices;

  while(!cit.IsAtEnd())
  {
    if(cit.Value() != 0 )
      target_world_indices.push_back(cit.GetIndex());
    ++cit;
  }

  mitk::Image::Pointer image;
  mitk::CastToMitkImage(this->GetInput(), image);
  mitk::BaseGeometry * transform = image->GetGeometry();

  while(!target_world_indices.empty())
  {
    vnl_vector<double> u_v(3,1), x_v(3,1), p_v(3,1);
    double line_histo_area = 0;

    // w->i skull point
    mitk::Point3D skull_point, tmp_point;
    image->GetGeometry()->IndexToWorld(target_world_indices.back(),skull_point);

    // project centerpoint to slice
    // x = p + s*u
    u_v = skull_point.GetVnlVector() - m_CenterOfMask;
    u_v.normalize();

    p_v = m_CenterOfMask;

    //step width
    double s_s = 0.1;

    // i->w center point
    mitk::Point3D center_point;
    center_point[0] = m_CenterOfMask[0];
    center_point[1] = m_CenterOfMask[1];
    center_point[2] = m_CenterOfMask[2];
    IndexType tmp_index;
    transform->WorldToIndex(center_point,tmp_index);
    oit.SetIndex(tmp_index);

    std::vector<IndexType> under_line_indices;

    while(true)
    {

      // store current_index
      under_line_indices.push_back(tmp_index);

      // set histo value
      iit.SetIndex(tmp_index);
      line_histo_area += iit.Value();

      // break in end reached
      if(tmp_index == target_world_indices.back())
      {
        break;
      }

      // get next voxel index under the line
      while(tmp_index == oit.GetIndex()) // if new voxel index reached brake
      {
        x_v = p_v + s_s * u_v; // next
        tmp_point.GetVnlVector().set(x_v.data_block());
        transform->WorldToIndex(tmp_point, tmp_index);
        s_s+=0.1;
      }

      oit.SetIndex(tmp_index);
    }


    while (!under_line_indices.empty()) {
      IndexType current_index = under_line_indices.back();
      under_line_indices.pop_back();

      oit.SetIndex(current_index);
      oit.Set(line_histo_area / (s_s * u_v).magnitude() );
    }

    target_world_indices.pop_back();

  }

}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::SetImageMask(TMaskImageType * maskimage)
{
  this->m_ImageMask = maskimage;
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
void itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::SetBinaryContour(TMaskImageType * binarycontour)
{
  this->m_BinaryContour = binarycontour;
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::LineHistogramBasedMassImageFilter()
{
  this->SetNumberOfIndexedOutputs(1);
  this->SetNumberOfIndexedInputs(1);

  m_CenterOfMask.fill(0);
}

template< class TInputImageType, class TOutputImageType, class TMaskImageType>
itk::LineHistogramBasedMassImageFilter<TInputImageType,TOutputImageType,TMaskImageType>::~LineHistogramBasedMassImageFilter()
{

}



#endif
