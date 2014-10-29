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
#ifndef __mitkExtendedStatisticsImageFilter_h
#define __mitkExtendedStatisticsImageFilter_h

#include "itkStatisticsImageFilter.h"

#include "itkDataObject.h"

#include "ImageStatisticsExports.h"

#include "mitkCommon.h"

namespace itk
{
/**
* \class ExtendedStatisticsImageFilter
* \brief
*
*/
template< class TInputImage >
class ImageStatistics_EXPORT ExtendedStatisticsImageFilter : public StatisticsImageFilter< TInputImage >
{
public:
  /** Standard Self typedef */
  typedef ExtendedStatisticsImageFilter                    Self;
  typedef StatisticsImageFilter< TInputImage >             Superclass;
  typedef SmartPointer< Self >                             Pointer;
  typedef SmartPointer< const Self >                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Runtime information support. */
  itkTypeMacro(ExtendedStatisticsImageFilter, StatisticsImageFilter);

  typedef typename Superclass::DataObjectPointer DataObjectPointer;

  /** Return the computed Skewness. */
  RealType GetSkewness() const
  {
    return this->GetSkewnessOutput()->Get();
  }
  RealObjectType* GetSkewnessOutput();

  const RealObjectType* GetSkewnessOutput() const;


  /** Return the computed Kurtosis. */
  RealType GetKurtosis() const
  {
    return this->GetKurtosisOutput()->Get();
  }

  RealObjectType* GetKurtosisOutput();

  const RealObjectType* GetKurtosisOutput() const;

  //using Superclass::MakeOutput;

  virtual DataObject::Pointer MakeOutput( ProcessObject::DataObjectPointerArraySizeType idx );

  /* Compute the Skewness Curtosis*/
  void computeTheSkewnessAndCurtosis();



protected:

  ExtendedStatisticsImageFilter();

  virtual ~ExtendedStatisticsImageFilter(){};

  void AfterThreadedGenerateData();


private:

//  Array< RealType >       m_ThreadSum;


}; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
  #include "mitkStatisticsImageFilter.hxx"
#endif

#endif
