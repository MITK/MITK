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

#ifndef __itkMITKScalarImageToHistogramGenerator_h
#define __itkMITKScalarImageToHistogramGenerator_h


#include <itkImageToListSampleAdaptor.h>
#include <itkSampleToHistogramFilter.h>
#include <itkObject.h>


namespace itk {
namespace Statistics {

template <class TImageType, class TMeasurementType = typename TImageType::PixelType>
class MITKScalarImageToHistogramGenerator : public Object
{
public:
  /** Standard typedefs */
  typedef MITKScalarImageToHistogramGenerator  Self;
  typedef Object                               Superclass;
  typedef SmartPointer<Self>                   Pointer;
  typedef SmartPointer<const Self>             ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(MITKScalarImageToHistogramGenerator, Object);

  /** standard New() method support */
  itkNewMacro(Self);

  typedef TImageType ImageType;
  typedef itk::Statistics::ImageToListSampleAdaptor<ImageType> AdaptorType;
  typedef typename AdaptorType::Pointer AdaptorPointer;
  typedef typename ImageType::PixelType PixelType;

  typedef itk::Statistics::Histogram<TMeasurementType, itk::Statistics::DenseFrequencyContainer2> HistogramType;
  typedef itk::Statistics::SampleToHistogramFilter<AdaptorType, HistogramType> GeneratorType;

  typedef typename GeneratorType::Pointer GeneratorPointer;

  typedef typename HistogramType::Pointer HistogramPointer;
  typedef typename HistogramType::ConstPointer HistogramConstPointer;

public:

  /** Triggers the Computation of the histogram */
  void Compute( void );

  /** Connects the input image for which the histogram is going to be computed */
  void SetInput( const ImageType * );

  /** Return the histogram. o
   \warning This output is only valid after the Compute() method has been invoked
   \sa Compute */
  const HistogramType * GetOutput() const;

  /** Set number of histogram bins */
  void SetNumberOfBins( unsigned int numberOfBins );

  /** Set marginal scale value to be passed to the histogram generator */
  void SetMarginalScale( double marginalScale );


protected:
  MITKScalarImageToHistogramGenerator();
  virtual ~MITKScalarImageToHistogramGenerator() {};
  void PrintSelf(std::ostream& os, Indent indent) const;


private:

  AdaptorPointer      m_ImageToListAdaptor;

  GeneratorPointer    m_HistogramGenerator;

};


} // end of namespace Statistics
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMITKScalarImageToHistogramGenerator.txx"
#endif

#endif
