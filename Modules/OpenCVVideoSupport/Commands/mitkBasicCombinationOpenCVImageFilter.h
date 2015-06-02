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

#ifndef mitkBasicCombinationOpenCVImageFilter_h
#define mitkBasicCombinationOpenCVImageFilter_h

#include "mitkAbstractOpenCVImageFilter.h"

//itk headers
#include "itkObjectFactory.h"

namespace mitk {
class MITKOPENCVVIDEOSUPPORT_EXPORT BasicCombinationOpenCVImageFilter : public AbstractOpenCVImageFilter
{
public:
  mitkClassMacro(BasicCombinationOpenCVImageFilter, AbstractOpenCVImageFilter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
    * \brief Apply list of filters to the given image.
    * \return false if one of the filter returns false. Be careful, that the image could already be changed when the function returned false.
    */
  bool OnFilterImage( cv::Mat& image ) override;

  /**
    * \brief Push an additional filter to the list of filters for applying to an image.
    */
  void PushFilter( AbstractOpenCVImageFilter::Pointer filter );

  /**
    * \brief Remove and return the last filter added to the list of filters.
    * \return last filter added to the list of filters
    */
  AbstractOpenCVImageFilter::Pointer PopFilter( );

  /**
    * \brief Remove the given filter from the list of filters.
    * \return true if the filter was on the list, false if it wasn't
    */
  bool RemoveFilter( AbstractOpenCVImageFilter::Pointer filter );

  /**
   * \brief Get the information if the given filter is on the filter list.
   * \return true if the filter is on the list, false otherwise
   */
  bool GetIsFilterOnTheList( AbstractOpenCVImageFilter::Pointer filter );

  /**
   * \brief Get the information if no filter is on the filter list.
   * \return true if the filter list if empty, false otherwise
   */
  bool GetIsEmpty();

protected:
  /**
    * \brief All filters applied to the given image during call of mitk::mitkBasicCombinationOpenCVImageFilter::FilterImage().
    */
  std::vector<AbstractOpenCVImageFilter::Pointer> m_FilterList;
};

} // namespace mitk

#endif // mitkBasicCombinationOpenCVImageFilter_h
