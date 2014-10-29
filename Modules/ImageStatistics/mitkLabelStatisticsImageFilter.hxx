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
#ifndef _mitkExtendedLabelStatisticsImageFilter_h
#define _mitkExtendedLabelStatisticsImageFilter_h
#include "mitkLabelStatisticsImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"

#include "itkImageFileWriter.h"

namespace itk
{
  template< class TInputImage , class TLabelImage>
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::ExtendedLabelStatisticsImageFilter()
    : LabelStatisticsImageFilter()
  {
    m_LabelStatistics2;
  }


  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetKurtosis(LabelPixelType label) const
  {
    MapConstIterator2 mapIt;

    mapIt = m_LabelStatistics2.find(label);
    if ( mapIt == m_LabelStatistics2.end() )
    {
      // label does not exist, return a default value
      return NumericTraits< PixelType >::Zero;
    }
    else
    {
      return ( *mapIt ).second.m_Kurtosis;
    }
  }



  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetSkewness(LabelPixelType label) const
  {
    MapConstIterator2 mapIt;

    mapIt = m_LabelStatistics2.find(label);
    if ( mapIt == m_LabelStatistics2.end() )
    {
      // label does not exist, return a default value
      return NumericTraits< PixelType >::Zero;
    }
    else
    {
      return ( *mapIt ).second.m_Skewness;
    }
  }




  template< class TInputImage, class TLabelImage >
  void
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    computeTheSkewnessAndCurtosis()
  {



    MapIterator mapIt;
    TLabelImage::RegionType Subregion;

    // MaskImageType::RegionType aa = labelStatisticsFilter->GetRegion(*it);
    double baseOfSkewnessAndCurtosis;
    double Kurtosis = 0.0;
    double Skewness = 0.0;

    //ImageRegionConstIteratorWithIndex< TInputImage > it (this->GetInput(),
    // outputRegionForThread);
    //ImageRegionConstIterator< TLabelImage > labelIt (this->GetLabelInput(),
    //outputRegionForThread);

    //   mapIt = m_LabelStatistics2.find( ( *threadIt ).first );
    //typedef typename MapType::value_type MapValueType;
    //  typedef typename MapType::value_type MapValueType;

    std::list< LabelPixelType> relevantLabels;
    bool maskNonEmpty = false;
    LabelPixelType i;
    for ( i = 1; i < 100; ++i )
    {
      if ( this->HasLabel( i ) )
      {
        relevantLabels.push_back( i );
        maskNonEmpty = true;
        m_LabelStatistics2.insert( std::make_pair(i, Variables()) );
      }
    }

    if ( maskNonEmpty )
    {
      std::list< LabelPixelType >::iterator it;
      for ( it = relevantLabels.begin(), i = 0;
        it != relevantLabels.end();
        ++it, ++i )
      {
        double Sigma = GetSigma( *it );
        double Mean     = GetMean( *it );
        Subregion = Superclass::GetRegion(*it);

        std::cout << "Sigma: " << Sigma << std::endl;
        std::cout << "Mean: "  << Mean  << std::endl;

        ImageRegionConstIteratorWithIndex< TInputImage > it1 (this->GetInput(),
          Subregion);
        ImageRegionConstIterator< TLabelImage > labelIt (this->GetLabelInput(),
          Subregion);

        itk::ImageFileWriter<TLabelImage>:: Pointer writer6= itk::ImageFileWriter<TLabelImage>::New();
        // writer->SetImageIO(nrrdImageIO);
        writer6->SetFileName("C:\\Users\\tmueller\\Documents\\TestPics\\SeedsFG_TEST22222.nrrd");
        writer6->SetInput(GetLabelInput());
        writer6->Update();


        itk::ImageFileWriter<TInputImage>:: Pointer writer7 = itk::ImageFileWriter<TInputImage>::New();
        // writer->SetImageIO(nrrdImageIO);
        writer7->SetFileName("C:\\Users\\tmueller\\Documents\\TestPics\\SeedsFG_TEST111.nrrd");
        writer7->SetInput(GetInput());
        writer7->Update();

        int sss = 0;

        for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1, ++labelIt)
        {
          if (labelIt.Get() == *it)
          {
            //std::cout <<  it1.Get()  << "..." << Mean << "..." << Sigma << std::endl;
            baseOfSkewnessAndCurtosis = (  it1.Get() -Mean ) / Sigma ;
            // std::cout << baseOfSkewnessAndCurtosis << std::endl;
            Kurtosis += baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis;
            Skewness += baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis;
            sss++;
          }
        }
        std::cout.precision(10);
        std::cout << "Skew:  " << Skewness << std::endl;
        std::cout << "GetCount(*it):  " << GetCount(*it) << ".." << sss <<std::endl;
        std::cout << "m_Skewness:  " << double(Skewness)/double(GetCount(*it)) << std::endl;
        std::cout << "m_Kurtosis:  " << double(Kurtosis)/double(GetCount(*it)) << std::endl;
        //     m_LabelStatistics2.insert(*it, Variables() );
        m_LabelStatistics2[*it].m_Skewness = double(Skewness/GetCount(*it));//.second.m_Skewness = Skewness;
        m_LabelStatistics2[*it].m_Kurtosis = double(Kurtosis/GetCount(*it)); //.second.m_Kurtosis = Kurtosis;

        //      MaskImageType::RegionType aa = this->GetRegion(*it);

        //      std::cout << "Index:  " << aa.GetIndex () << std::endl;
        //     std::cout << "Size: "   << aa.GetSize ()  << std::endl;

        //      std::cout << this->GetNumberOfLabels() << std::endl;



        /*  itk::NrrdImageIO::Pointer nrrdImageIO = itk::NrrdImageIO::New();

        itk::ImageFileWriter<MaskImageType>:: Pointer writer = itk::ImageFileWriter<MaskImageType>::New();
        // writer->SetImageIO(nrrdImageIO);
        writer->SetFileName("C:\\Users\\tmueller\\Documents\\SeedsFG_TEST.nrrd");
        writer->SetInput(adaptedMaskImage);
        writer->Update();
        //std::cout << "Wrote: " << outputImageFile << std::endl;

        itk::ImageFileWriter<ImageType>:: Pointer writer2 = itk::ImageFileWriter<ImageType>::New();
        // writer->SetImageIO(nrrdImageIO);
        writer2->SetFileName("C:\\Users\\tmueller\\Documents\\SeedsFG_TEST2.nrrd");
        writer2->SetInput(adaptedImage);
        writer2->Update();
        //std::cout << "Wrote: " << outputImageFile << std::endl;

        itk::ImageFileWriter<MaskImageType>:: Pointer writer3 = itk::ImageFileWriter<MaskImageType>::New();
        // writer->SetImageIO(nrrdImageIO);
        writer3->SetFileName("C:\\Users\\tmueller\\Documents\\SeedsFG_TEST2.nrrd");
        writer3->SetInput( labelStatisticsFilter->GetLabelInput () );
        writer3->Update();
        //std::cout << "Wrote: " << outputImageFile << std::endl;*/


      }
    }
  }


  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {
    Superclass::AfterThreadedGenerateData();

    computeTheSkewnessAndCurtosis();
  }




} // end namespace itk

#endif