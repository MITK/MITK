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
#ifndef __itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter_h
#define __itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter_h

#include "itkHistogram.h"
#include "itkMacro.h"
#include "itkProcessObject.h"
#include "itkSimpleDataObjectDecorator.h"

namespace itk {
  namespace Statistics {
    /** \class EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter
    *  \brief This class computes texture feature coefficients from a grey level
    * Zone-length matrix.
    *
    * By default, Zone length features are computed for each spatial
    * direction and then averaged afterward, so it is possible to access the
    * standard deviations of the texture features. These values give a clue as
    * to texture anisotropy. However, doing this is much more work, because it
    * involved computing one for each offset given. To compute a single matrix
    * using the first offset, call FastCalculationsOn(). If this is called,
    * then the texture standard deviations will not be computed (and will be set
    * to zero), but texture computation will be much faster.
    *
    * This class is templated over the input histogram type.
    *
    * Print references:
    * M. M. Galloway. Texture analysis using gray level Zone lengths. Computer
    * Graphics and Image Processing, 4:172-179, 1975.
    *
    * A. Chu, C. M. Sehgal, and J. F. Greenleaf. Use of gray value distribution of
    * Zone lengths for texture analysis.  Pattern Recognition Letters, 11:415-420,
    * 1990.
    *
    * B. R. Dasarathy and E. B. Holder. Image characterizations based on joint
    * gray-level Zone-length distributions. Pattern Recognition Letters, 12:490-502,
    * 1991.
    *
    * IJ article: http://hdl.handle.net/1926/1374
    *
    * \sa ScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter
    * \sa ScalarImageToNeighbourhoodGreyLevelDifferenceMatrixFilter
    * \sa EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter
    *
    * \author: Nick Tustison
    * \ingroup ITKStatistics
    */

    template< typename THistogram >
    class EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter : public ProcessObject
    {
    public:
      /** Standard typedefs */
      typedef EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter     Self;
      typedef ProcessObject                          Superclass;
      typedef SmartPointer<Self>                     Pointer;
      typedef SmartPointer<const Self>               ConstPointer;

      /** Zone-time type information (and related methods). */
      itkTypeMacro( EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter, ProcessObject );

      /** standard New() method support */
      itkNewMacro( Self );

      typedef THistogram                                      HistogramType;
      typedef typename HistogramType::Pointer                 HistogramPointer;
      typedef typename HistogramType::ConstPointer            HistogramConstPointer;
      typedef typename HistogramType::MeasurementType         MeasurementType;
      typedef typename HistogramType::MeasurementVectorType   MeasurementVectorType;
      typedef typename HistogramType::IndexType               IndexType;
      typedef typename HistogramType::
        TotalAbsoluteFrequencyType                            FrequencyType;

      /** Method to Set/Get the input Histogram */
      using Superclass::SetInput;
      void SetInput (const HistogramType * histogram );
      const HistogramType * GetInput() const;

      /** Smart Pointer type to a DataObject. */
      typedef DataObject::Pointer                   DataObjectPointer;

      /** Type of DataObjects used for scalar outputs */
      typedef SimpleDataObjectDecorator<MeasurementType>     MeasurementObjectType;

      /** Methods to return the short Zone emphasis. */
      MeasurementType GetCoarseness() const;
      const MeasurementObjectType* GetCoarsenessOutput() const;

      /** Methods to return the long Zone emphasis. */
      MeasurementType GetContrast() const;
      const MeasurementObjectType* GetContrastOutput() const;

      /** Methods to return the grey level nonuniformity. */
      MeasurementType GetBusyness() const;
      const MeasurementObjectType* GetBusynessOutput() const;

      /** Methods to return the Zone length nonuniformity. */
      MeasurementType GetComplexity() const;
      const MeasurementObjectType* GetComplexityOutput() const;

      /** Methods to return the low grey level Zone emphasis. */
      MeasurementType GetStrength() const;
      const MeasurementObjectType* GetStrengthOutput() const;

      itkGetMacro( TotalNumberOfValidVoxels, unsigned long );

      itkGetConstMacro(NumberOfVoxels, unsigned long);
      itkSetMacro(NumberOfVoxels, unsigned long);

      void SetSiMatrix(double* matrix)
      {
        m_siMatrix = matrix;
      }

      /** Zone-length feature types */
      typedef enum
      {
        Coarseness,
        Contrast,
        Busyness,
        Complexity,
        Strength
      }  NeighbourhoodGreyLevelDifferenceFeatureName;

      /** convenience method to access the Zone length values */
      MeasurementType GetFeature( NeighbourhoodGreyLevelDifferenceFeatureName name );

    protected:
      EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter();
      ~EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter() {};
      virtual void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

      /** Make a DataObject to be used for output output. */
      typedef ProcessObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;
      using Superclass::MakeOutput;
      virtual DataObjectPointer MakeOutput( DataObjectPointerArraySizeType ) ITK_OVERRIDE;

      virtual void GenerateData() ITK_OVERRIDE;

    private:
      EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter(const Self&); //purposely not implemented
      void operator=(const Self&); //purposely not implemented

      unsigned long                           m_TotalNumberOfValidVoxels;
      unsigned long                           m_NumberOfVoxels;

      double*                                 m_siMatrix;
    };
  } // end of namespace Statistics
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter.hxx"
#endif

#endif
