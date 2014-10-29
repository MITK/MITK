/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __mitkExtendedLabelStatisticsImageFilter_h
#define __mitkExtendedLabelStatisticsImageFilter_h

#include "itkLabelStatisticsImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itksys/hash_map.hxx"
#include "itkHistogram.h"
#include "itkFastMutexLock.h"
#include <vector>
#include "itkDataObject.h"

#include "ImageStatisticsExports.h"
#include "mitkCommon.h"

namespace itk
{
/** \class LabelStatisticsImageFilter
 ,Get statistical properties of labeled regions in an image}
 * \endwiki
 */
template< class TInputImage, class TLabelImage >
class /*ImageStatistics_EXPORT*/ ExtendedLabelStatisticsImageFilter : public LabelStatisticsImageFilter< TInputImage,  TLabelImage >
{
public:

  typedef ExtendedLabelStatisticsImageFilter             Self;
  typedef LabelStatisticsImageFilter < TInputImage, TLabelImage > Superclass;
  typedef SmartPointer< Self >                           Pointer;
  typedef SmartPointer< const Self >                     ConstPointer;


  itkNewMacro(Self);


  itkTypeMacro(ExtendedLabelStatisticsImageFilter, LabelStatisticsImageFilter);


//  typedef typename Superclass::DataObjectPointer DataObjectPointer;

  //RealType         m_Kurtosis;
 // RealType         m_Skewness;

  class Variables
  {
  public:
    double m_Kurtosis;
    double m_Skewness;

    ~Variables(){};
    Variables(){

      m_Kurtosis = 0;
      m_Skewness = 0;


    };

  };


 typedef std::map< LabelPixelType, Variables >        MapType2;
 typedef typename std::map< LabelPixelType, Variables >::const_iterator MapConstIterator2;




  RealType GetSkewness(LabelPixelType label) const;

  RealType GetKurtosis(LabelPixelType label) const;






  void computeTheSkewnessAndCurtosis();


protected:

  ExtendedLabelStatisticsImageFilter();

  /*virtual ~ExtendedLabelStatisticsImageFilter(){};*/

  void AfterThreadedGenerateData();



//ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  //itkConceptMacro( InputHasNumericTraitsCheck,
                 //  ( Concept::HasNumericTraits< PixelType > ) );
  /** End concept checking */
//#endif

//protected:



private:


 MapType2         m_LabelStatistics2;



}; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkLabelStatisticsImageFilter.hxx"
#endif

#endif
