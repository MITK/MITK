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

#include "mitkBasicCombinationOpenCVImageFilter.h"

#include "cv.h"

namespace mitk {

bool BasicCombinationOpenCVImageFilter::OnFilterImage( cv::Mat& image )
{
  int imageId = this->GetCurrentImageId();

  // go through the list of all filters
  for ( auto it
        = m_FilterList.begin(); it != m_FilterList.end(); ++it )
  {
    // apply current filter and return false if the filter returned false
    if (! (*it)->FilterImage(image, imageId) ) { return false; }
  }

  return true;
}

void BasicCombinationOpenCVImageFilter::PushFilter( AbstractOpenCVImageFilter::Pointer filter )
{
  m_FilterList.push_back(filter);
}

AbstractOpenCVImageFilter::Pointer BasicCombinationOpenCVImageFilter::PopFilter( )
{
  AbstractOpenCVImageFilter::Pointer lastFilter = m_FilterList.at(m_FilterList.size()-1);
  m_FilterList.pop_back();
  return lastFilter;
}

bool BasicCombinationOpenCVImageFilter::RemoveFilter( AbstractOpenCVImageFilter::Pointer filter )
{
  for ( auto it
        = m_FilterList.begin(); it != m_FilterList.end(); it++ )
  {
    if (*it == filter) {
      m_FilterList.erase(it);
      return true;
    }
  }

  return false;
}

bool BasicCombinationOpenCVImageFilter::GetIsFilterOnTheList( AbstractOpenCVImageFilter::Pointer filter )
{
  return std::find(m_FilterList.begin(), m_FilterList.end(), filter) != m_FilterList.end();
}

bool BasicCombinationOpenCVImageFilter::GetIsEmpty()
{
  return m_FilterList.empty();
}
} // namespace mitk
