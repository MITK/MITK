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

    /* adding an internal class which invokes a class with the new calculated coefficients; Skewness and Curtosis*/
    class CoefficientsClass
    {
    public:
      /* the new memeber coefficients*/
      double m_Kurtosis;
      double m_Skewness;

      ~CoefficientsClass(){};
      CoefficientsClass(){

        m_Kurtosis = 0;
        m_Skewness = 0;

      };

    };

    typedef std::map< LabelPixelType, CoefficientsClass >        CoefficientsMap;
    typedef typename std::map< LabelPixelType, CoefficientsClass >::const_iterator CoefficientsMapConstIterator;

    /*getter method for the new coefficients*/
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;

    /* this method will calculate the new coefficients with sigma and mean value of the threaded generate data of the base class*/
    void ComputeTheSkewnessAndCurtosis();

  protected:

    ExtendedLabelStatisticsImageFilter();

    virtual ~ExtendedLabelStatisticsImageFilter(){};

    void AfterThreadedGenerateData();


  private:

    CoefficientsMap         m_LabelStatisticsCoefficients;


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedLabelStatisticsImageFilter.hxx"
#endif

#endif
