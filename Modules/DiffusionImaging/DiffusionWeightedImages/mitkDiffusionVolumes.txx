/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

template<typename TPixelType>
mitk::DiffusionVolumes<TPixelType>::DiffusionVolumes()
{
}

template<typename TPixelType>
mitk::DiffusionVolumes<TPixelType>::~DiffusionVolumes()
{

}

template<typename TPixelType>
bool mitk::DiffusionVolumes<TPixelType>::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

template<typename TPixelType>
void mitk::DiffusionVolumes<TPixelType>::SetRequestedRegion(itk::DataObject *data)
{
}

template<typename TPixelType>
void mitk::DiffusionVolumes<TPixelType>::SetRequestedRegionToLargestPossibleRegion()
{
}

template<typename TPixelType>
bool mitk::DiffusionVolumes<TPixelType>::VerifyRequestedRegion()
{
  return true;
}

template<typename TPixelType>
void mitk::DiffusionVolumes<TPixelType>::DuplicateIfSingleSlice()
{
  // new image
  typename ImageType::Pointer oldImage = m_Image;
  m_Image = ImageType::New();
  m_Image->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
  m_Image->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
  m_Image->SetDirection( oldImage->GetDirection() );  // Set the image direction
  typename ImageType::RegionType region = oldImage->GetLargestPossibleRegion();
  if(region.GetSize(0) == 1)
    region.SetSize(0,3);
  if(region.GetSize(1) == 1)
    region.SetSize(1,3);
  if(region.GetSize(2) == 1)
    region.SetSize(2,3);
  m_Image->SetLargestPossibleRegion( region );
  m_Image->SetVectorLength( m_Directions->size() );
  m_Image->SetBufferedRegion( region );
  m_Image->Allocate();

  // average image data that corresponds to identical directions
  itk::ImageRegionIterator< ImageType > newIt(m_Image, region);
  newIt.GoToBegin();
  itk::ImageRegionIterator< ImageType > oldIt(oldImage, oldImage->GetLargestPossibleRegion());
  oldIt.GoToBegin();

  while(!newIt.IsAtEnd())
  {
    newIt.Set(oldIt.Get());
    ++newIt;
    ++oldIt;
    if(oldIt.IsAtEnd())
      oldIt.GoToBegin();
  }

}

template<typename TPixelType>
void mitk::DiffusionVolumes<TPixelType>::AverageRedundantGradients()
{

  // new direction container
  GradientDirectionContainerType::Pointer oldDirections = m_Directions;
  m_Directions = GradientDirectionContainerType::New();

  // fill new direction container
  for(GradientDirectionContainerType::ConstIterator gdcitOld = oldDirections->Begin();
    gdcitOld != oldDirections->End(); ++gdcitOld)
  {
    // already exists?
    bool found = false;
    for(GradientDirectionContainerType::ConstIterator gdcitNew = m_Directions->Begin();
      gdcitNew != m_Directions->End(); ++gdcitNew)
    {
      if(gdcitNew.Value() == gdcitOld.Value())
      {
        found = true;
        break;
      }
    }

    // if not found, add it to new container
    if(!found)
    {
      m_Directions->push_back(gdcitOld.Value());
    }
  }

  // if sizes equal, we do not need to do anything in this function
  if(m_Directions->size() == oldDirections->size())
    return;

  // new image
  typename ImageType::Pointer oldImage = m_Image;
  m_Image = ImageType::New();
  m_Image->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
  m_Image->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
  m_Image->SetDirection( oldImage->GetDirection() );  // Set the image direction
  m_Image->SetLargestPossibleRegion( oldImage->GetLargestPossibleRegion() );
  m_Image->SetVectorLength( m_Directions->size() );
  m_Image->SetBufferedRegion( oldImage->GetLargestPossibleRegion() );
  m_Image->Allocate();

  // average image data that corresponds to identical directions
  itk::ImageRegionIterator< ImageType > newIt(m_Image, m_Image->GetLargestPossibleRegion());
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
      if(gdcitNew.Value() == gdcitOld.Value())
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
    if(ind1!=ind.m_Index[2])
      std::cout << ind.m_Index[2] << std::endl;
    ind1 = ind.m_Index[2];

    // init new vector with zeros
    newVec.Fill(0.0);

    // the old voxel value with duplicates
    typename ImageType::PixelType oldVec = oldIt.Get();

    for(int i=0; i<dirIndices.size(); i++)
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


