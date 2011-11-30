/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-08 11:19:03 +0100 (Fr, 08 Feb 2008) $
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "mitkImageCast.h"

template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::DiffusionImage()
: m_VectorImage(0), m_Directions(0), m_OriginalDirections(0), m_B_Value(-1.0), m_VectorImageAdaptor(0)
{
  MeasurementFrameType mf;
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
      mf[i][j] = 0;
  for(int i=0; i<3; i++)
    mf[i][i] = 1;
  m_MeasurementFrame = mf;
}

template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::~DiffusionImage()
{

}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>
::InitializeFromVectorImage()
{
  if(!m_VectorImage || !m_Directions || m_B_Value==-1.0)
  {
    MITK_INFO << "DiffusionImage could not be initialized. Set all members first!" << std::endl;
    return;
  }

  // find bzero index
  int firstZeroIndex = -1;
  for(GradientDirectionContainerType::ConstIterator it = m_Directions->Begin();
    it != m_Directions->End(); ++it)
  {
    firstZeroIndex++;
    GradientDirectionType g = it.Value();
    if(g[0] == 0 && g[1] == 0 && g[2] == 0 )
      break;
  }

  typedef itk::Image<TPixelType,3> ImgType;
  typename ImgType::Pointer img = ImgType::New();
  img->SetSpacing( m_VectorImage->GetSpacing() );   // Set the image spacing
  img->SetOrigin( m_VectorImage->GetOrigin() );     // Set the image origin
  img->SetDirection( m_VectorImage->GetDirection() );  // Set the image direction
  img->SetLargestPossibleRegion( m_VectorImage->GetLargestPossibleRegion());
  img->SetBufferedRegion( m_VectorImage->GetLargestPossibleRegion() );
  img->Allocate();

  int vecLength = m_VectorImage->GetVectorLength();
  InitializeByItk( img.GetPointer(), 1, vecLength );

  //for(int i=0; i<vecLength; i++)
  //{
    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw = itw.Begin();

    itk::ImageRegionConstIterator<ImageType> itr (m_VectorImage, m_VectorImage->GetLargestPossibleRegion() );
    itr = itr.Begin();

    while(!itr.IsAtEnd())
    {
      itw.Set(itr.Get().GetElement(firstZeroIndex));
      ++itr;
      ++itw;
    }

    // init
    SetImportVolume(img->GetBufferPointer());//, 0, 0, CopyMemory);
    //SetVolume( img->GetBufferPointer(), i );
  //}

  m_DisplayIndex = firstZeroIndex;
  MITK_INFO << "Diffusion-Image successfully initialized.";

}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>
::SetDisplayIndexForRendering(int displayIndex)
{
  int index = displayIndex;
  int vecLength = m_VectorImage->GetVectorLength();
  index = index > vecLength-1 ? vecLength-1 : index;
  if( m_DisplayIndex != index )
  {
    typedef itk::Image<TPixelType,3> ImgType;
    typename ImgType::Pointer img = ImgType::New();
    CastToItkImage<ImgType>(this, img);

    itk::ImageRegionIterator<ImgType> itw (img, img->GetLargestPossibleRegion() );
    itw = itw.Begin();

    itk::ImageRegionConstIterator<ImageType> itr (m_VectorImage, m_VectorImage->GetLargestPossibleRegion() );
    itr = itr.Begin();

    while(!itr.IsAtEnd())
    {
      itw.Set(itr.Get().GetElement(index));
      ++itr;
      ++itw;
    }
  }

  m_DisplayIndex = index;
}


//template<typename TPixelType>
//bool mitk::DiffusionImage<TPixelType>::RequestedRegionIsOutsideOfTheBufferedRegion()
//{
//  return false;
//}
//
//template<typename TPixelType>
//void mitk::DiffusionImage<TPixelType>::SetRequestedRegion(itk::DataObject * /*data*/)
//{
//}
//
//template<typename TPixelType>
//void mitk::DiffusionImage<TPixelType>::SetRequestedRegionToLargestPossibleRegion()
//{
//}
//
//template<typename TPixelType>
//bool mitk::DiffusionImage<TPixelType>::VerifyRequestedRegion()
//{
//  return true;
//}

//template<typename TPixelType>
//void mitk::DiffusionImage<TPixelType>::DuplicateIfSingleSlice()
//{
//  // new image
//  typename ImageType::Pointer oldImage = m_Image;
//  m_Image = ImageType::New();
//  m_Image->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
//  m_Image->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
//  m_Image->SetDirection( oldImage->GetDirection() );  // Set the image direction
//  typename ImageType::RegionType region = oldImage->GetLargestPossibleRegion();
//  if(region.GetSize(0) == 1)
//    region.SetSize(0,3);
//  if(region.GetSize(1) == 1)
//    region.SetSize(1,3);
//  if(region.GetSize(2) == 1)
//    region.SetSize(2,3);
//  m_Image->SetLargestPossibleRegion( region );
//  m_Image->SetVectorLength( m_Directions->size() );
//  m_Image->SetBufferedRegion( region );
//  m_Image->Allocate();
//
//  // average image data that corresponds to identical directions
//  itk::ImageRegionIterator< ImageType > newIt(m_Image, region);
//  newIt.GoToBegin();
//  itk::ImageRegionIterator< ImageType > oldIt(oldImage, oldImage->GetLargestPossibleRegion());
//  oldIt.GoToBegin();
//
//  while(!newIt.IsAtEnd())
//  {
//    newIt.Set(oldIt.Get());
//    ++newIt;
//    ++oldIt;
//    if(oldIt.IsAtEnd())
//      oldIt.GoToBegin();
//  }
//
//}

template<typename TPixelType>
bool mitk::DiffusionImage<TPixelType>::AreAlike(GradientDirectionType g1,
                                                  GradientDirectionType g2,
                                                  double precision)
{
  GradientDirectionType diff = g1 - g2;
  return diff.two_norm() < precision;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::CorrectDKFZBrokenGradientScheme(double precision)
{
  GradientDirectionContainerType::Pointer directionSet = CalcAveragedDirectionSet(precision, m_Directions);
  if(directionSet->size() < 7)
  {
    MITK_INFO << "Too few directions, assuming and correcting DKFZ-bogus sequence details.";

    double v [7][3] =
     {{ 0,         0,         0        },
      {-0.707057,  0,         0.707057 },
      { 0.707057,  0,         0.707057 },
      { 0,         0.707057,  0.707057 },
      { 0,         0.707057, -0.707057 },
      {-0.707057,  0.707057,  0        },
      { 0.707057,  0.707057,  0        } };

    int i=0;

    for(GradientDirectionContainerType::Iterator it = m_OriginalDirections->Begin();
    it != m_OriginalDirections->End(); ++it)
    {
      it.Value().set(v[i++%7]);
    }
    ApplyMeasurementFrame();
  }
}

template<typename TPixelType>
mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Pointer
mitk::DiffusionImage<TPixelType>::CalcAveragedDirectionSet(double precision, GradientDirectionContainerType::Pointer directions)
{
  // save old and construct new direction container
  GradientDirectionContainerType::Pointer newDirections = GradientDirectionContainerType::New();

  // fill new direction container
  for(GradientDirectionContainerType::ConstIterator gdcitOld = directions->Begin();
  gdcitOld != directions->End(); ++gdcitOld)
  {
    // already exists?
    bool found = false;
    for(GradientDirectionContainerType::ConstIterator gdcitNew = newDirections->Begin();
    gdcitNew != newDirections->End(); ++gdcitNew)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        found = true;
        break;
      }
    }

    // if not found, add it to new container
    if(!found)
    {
      newDirections->push_back(gdcitOld.Value());
    }
  }

  return newDirections;
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::AverageRedundantGradients(double precision)
{

  GradientDirectionContainerType::Pointer newDirs =
      CalcAveragedDirectionSet(precision, m_Directions);

  GradientDirectionContainerType::Pointer newOriginalDirs =
      CalcAveragedDirectionSet(precision, m_OriginalDirections);

  // if sizes equal, we do not need to do anything in this function
  if(m_Directions->size() == newDirs->size() || m_OriginalDirections->size() == newOriginalDirs->size())
    return;

  GradientDirectionContainerType::Pointer oldDirections = m_Directions;
  GradientDirectionContainerType::Pointer oldOriginalDirections = m_OriginalDirections;
  m_Directions = newDirs;
  m_OriginalDirections = newOriginalDirs;

  // new image
  typename ImageType::Pointer oldImage = m_VectorImage;
  m_VectorImage = ImageType::New();
  m_VectorImage->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
  m_VectorImage->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
  m_VectorImage->SetDirection( oldImage->GetDirection() );  // Set the image direction
  m_VectorImage->SetLargestPossibleRegion( oldImage->GetLargestPossibleRegion() );
  m_VectorImage->SetVectorLength( m_Directions->size() );
  m_VectorImage->SetBufferedRegion( oldImage->GetLargestPossibleRegion() );
  m_VectorImage->Allocate();

  // average image data that corresponds to identical directions
  itk::ImageRegionIterator< ImageType > newIt(m_VectorImage, m_VectorImage->GetLargestPossibleRegion());
  newIt.GoToBegin();
  itk::ImageRegionIterator< ImageType > oldIt(oldImage, oldImage->GetLargestPossibleRegion());
  oldIt.GoToBegin();

  // initial new value of voxel
  typename ImageType::PixelType newVec;
  newVec.SetSize(m_Directions->size());
  newVec.AllocateElements(m_Directions->size());

  std::vector<std::vector<int> > dirIndices;
  for(GradientDirectionContainerType::ConstIterator gdcitNew = m_Directions->Begin();
    gdcitNew != m_Directions->End(); ++gdcitNew)
  {
    dirIndices.push_back(std::vector<int>(0));
    for(GradientDirectionContainerType::ConstIterator gdcitOld = oldDirections->Begin();
      gdcitOld != oldDirections->End(); ++gdcitOld)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        dirIndices[gdcitNew.Index()].push_back(gdcitOld.Index());
      }
    }
  }

  int ind1 = -1;
  while(!newIt.IsAtEnd())
  {

    // progress
    typename ImageType::IndexType ind = newIt.GetIndex();
    ind1 = ind.m_Index[2];

    // init new vector with zeros
    newVec.Fill(0.0);

    // the old voxel value with duplicates
    typename ImageType::PixelType oldVec = oldIt.Get();

    for(unsigned int i=0; i<dirIndices.size(); i++)
    {
      // do the averaging
      int numavg = dirIndices[i].size();
      for(int j=0; j<numavg; j++)
      {
        newVec[i] += oldVec[dirIndices[i].at(j)];
      }
      newVec[i] /= numavg;
    }

    newIt.Set(newVec);

    ++newIt;
    ++oldIt;
  }
}

template<typename TPixelType>
void mitk::DiffusionImage<TPixelType>::ApplyMeasurementFrame()
{
  m_Directions = GradientDirectionContainerType::New();
  int c = 0;
  for(GradientDirectionContainerType::ConstIterator gdcit = m_OriginalDirections->Begin();
    gdcit != m_OriginalDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    vec = vec.pre_multiply(m_MeasurementFrame);
    m_Directions->InsertElement(c, vec);
    c++;
  }
}

// returns number of gradients
template<typename TPixelType>
int mitk::DiffusionImage<TPixelType>::GetNumDirections()
{
  int gradients = m_OriginalDirections->Size();
  for (int i=0; i<m_OriginalDirections->Size(); i++)
    if (GetB_Value(i)<=0)
    {
      gradients--;
    }
  return gradients;
}

// returns number of not diffusion weighted images
template<typename TPixelType>
int mitk::DiffusionImage<TPixelType>::GetNumB0()
{
  int b0 = 0;
  for (int i=0; i<m_OriginalDirections->Size(); i++)
    if (GetB_Value(i)<=0)
    {
      b0++;
    }
  return b0;
}

// returns a list of indices belonging to the not diffusion weighted images
template<typename TPixelType>
std::vector<int> mitk::DiffusionImage<TPixelType>::GetB0Indices()
{
  std::vector<int> indices;
  for (int i=0; i<m_OriginalDirections->Size(); i++)
    if (GetB_Value(i)<=0)
    {
      indices.push_back(i);
    }
  return indices;
}

template<typename TPixelType>
bool mitk::DiffusionImage<TPixelType>::IsMultiBval()
{
  int gradients = m_OriginalDirections->Size();

  for (int i=0; i<gradients; i++)
    if (GetB_Value(i)>0 && std::fabs(m_B_Value-GetB_Value(i))>50)
      return true;
  return false;
}
