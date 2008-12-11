/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkCalculateGrayValueStatisticsTool.h"

#include "mitkCalculateGrayValueStatisticsTool.xpm"

#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageCast.h"
#include "mitkToolManager.h"

#include <itkCommand.h>
#include <itkMapContainer.h>

mitk::CalculateGrayValueStatisticsTool::CalculateGrayValueStatisticsTool()
{
}

mitk::CalculateGrayValueStatisticsTool::~CalculateGrayValueStatisticsTool()
{
}

const char** mitk::CalculateGrayValueStatisticsTool::GetXPM() const
{
  return mitkCalculateGrayValueStatisticsTool_xpm;
}

const char* mitk::CalculateGrayValueStatisticsTool::GetName() const
{
  return "Statistics";
}

std::string mitk::CalculateGrayValueStatisticsTool::GetErrorMessage()
{
  return "No statistics generated for these segmentations:";
}

void mitk::CalculateGrayValueStatisticsTool::StartProcessingAllData()
{
  // clear/prepare report
  m_CompleteReport.str("");
}
    
bool mitk::CalculateGrayValueStatisticsTool::ProcessOneWorkingData( DataTreeNode* node )
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
    if (image.IsNull()) return false;
  
    DataTreeNode* referencenode = m_ToolManager->GetReferenceData(0);
    if (!referencenode) return false;
      
    try
    {
      ProgressBar::GetInstance()->AddStepsToDo(1);

      // add to report
      std::string nodename("structure");
      node->GetName(nodename);
      
      std::string message = "Calculating statistics for ";
      message += nodename;

      StatusBar::GetInstance()->DisplayText(message.c_str());

      Image::Pointer refImage = dynamic_cast<Image*>( referencenode->GetData() );
      Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
            
      m_CompleteReport << "======== Gray value analysis of " << nodename << " ========\n";
        
      if (image.IsNotNull() && refImage.IsNotNull() )
      {
        for (unsigned int timeStep = 0; timeStep < image->GetTimeSteps(); ++timeStep)
        {
          ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
          timeSelector->SetInput( refImage );
          timeSelector->SetTimeNr( timeStep );
          timeSelector->UpdateLargestPossibleRegion();
          Image::Pointer refImage3D = timeSelector->GetOutput();

          ImageTimeSelector::Pointer timeSelector2 = ImageTimeSelector::New();
          timeSelector2->SetInput( image );
          timeSelector2->SetTimeNr( timeStep );
          timeSelector2->UpdateLargestPossibleRegion();
          Image::Pointer image3D = timeSelector2->GetOutput();

          if (image3D.IsNotNull() && refImage3D.IsNotNull() )
          {
            m_CompleteReport << "=== " << nodename << ", time step " << timeStep << " ===\n";
            AccessFixedDimensionByItk_2( refImage3D, ITKHistogramming, 3, image3D, m_CompleteReport );
          }
        }
      }
            
      m_CompleteReport << "======== End of analysis for " << nodename << " ===========\n\n\n";

      ProgressBar::GetInstance()->Progress();
    }
    catch(...)
    {
      return false;
    }
  }

  return true;
}

void mitk::CalculateGrayValueStatisticsTool::FinishProcessingAllData()
{
  SegmentationsProcessingTool::FinishProcessingAllData();

  // show/send results
  StatisticsCompleted.Send();
  //std::cout << m_CompleteReport.str() << std::endl;
}

#define ROUND_P(x) ((int)((x)+0.5))
    
template <typename TPixel, unsigned int VImageDimension>
void mitk::CalculateGrayValueStatisticsTool::ITKHistogramming( itk::Image<TPixel, VImageDimension>* referenceImage, Image* segmentation, std::stringstream& report )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;

  typename SegmentationType::Pointer segmentationItk;
  CastToItkImage( segmentation, segmentationItk );
  
  // generate histogram
  typename SegmentationType::RegionType segmentationRegion = segmentationItk->GetLargestPossibleRegion();

  segmentationRegion.Crop( referenceImage->GetLargestPossibleRegion() );
    
  itk::ImageRegionConstIteratorWithIndex< SegmentationType > segmentationIterator( segmentationItk, segmentationRegion);
  itk::ImageRegionConstIteratorWithIndex< ImageType >        referenceIterator( referenceImage, segmentationRegion);

  segmentationIterator.GoToBegin();
  referenceIterator.GoToBegin();
    
  typedef itk::MapContainer<TPixel, long int> HistogramType;
  TPixel minimum = std::numeric_limits<TPixel>::max();
  TPixel maximum = std::numeric_limits<TPixel>::min();
  typename HistogramType::Pointer histogram = HistogramType::New();
    
  double mean(0.0);
  double sd(0.0);
  double voxelCount(0.0);

  // first pass for mean, min, max, histogram values
  while ( !segmentationIterator.IsAtEnd() )
  {
    itk::Point< float, 3 > pt;
    segmentationItk->TransformIndexToPhysicalPoint( segmentationIterator.GetIndex(), pt );

    typename ImageType::IndexType ind;
    itk::ContinuousIndex<float, 3> contInd;
    if (referenceImage->TransformPhysicalPointToContinuousIndex(pt, contInd))
    {
      for (unsigned int i = 0; i < 3; ++i) ind[i] = ROUND_P(contInd[i]);
      
      referenceIterator.SetIndex( ind );

      if ( segmentationIterator.Get() > 0 )
      {
        //++(*histogram)[referenceIterator.Get()];
        ++(histogram->ElementAt( referenceIterator.Get() ));
        if (referenceIterator.Get() < minimum) minimum = referenceIterator.Get();
        if (referenceIterator.Get() > maximum) maximum = referenceIterator.Get();
      
        mean =   (mean * ( static_cast<double>(voxelCount) / static_cast<double>(voxelCount+1) ) )  // 3 points:   old center * 2/3 + currentPoint * 1/3;
           + static_cast<double>(referenceIterator.Get() ) / static_cast<double>(voxelCount+1);

        voxelCount += 1.0;
      }
    }

    ++segmentationIterator;
  }

  // second pass for SD
  segmentationIterator.GoToBegin();
  referenceIterator.GoToBegin();
  while ( !segmentationIterator.IsAtEnd() )
  {
    itk::Point< float, 3 > pt;
    segmentationItk->TransformIndexToPhysicalPoint( segmentationIterator.GetIndex(), pt );

    typename ImageType::IndexType ind;
    itk::ContinuousIndex<float, 3> contInd;
    if (referenceImage->TransformPhysicalPointToContinuousIndex(pt, contInd))
    {
      for (unsigned int i = 0; i < 3; ++i) ind[i] = ROUND_P(contInd[i]);
      
      referenceIterator.SetIndex( ind );

      if ( segmentationIterator.Get() > 0 )
      {
        sd += ((static_cast<double>(referenceIterator.Get() ) - mean)*(static_cast<double>(referenceIterator.Get() ) - mean));
      }
    }

    ++segmentationIterator;
  }

  sd /= static_cast<double>(voxelCount - 1);
  sd = sqrt( sd );

  // evaluate histogram, generate quantiles
  long int totalCount(0);

  for ( typename HistogramType::iterator iter = histogram->begin();
        iter != histogram->end();
        ++iter )
  {
    totalCount += iter->second;
  }

  TPixel histogramQuantileValues[102];

  double quantiles[102];
  for (unsigned int i = 0; i < 102; ++i) quantiles[i] = static_cast<double>(i) / 100.0; quantiles[102-1] = 2.0;

  for (unsigned int i = 0; i < 102; ++i) histogramQuantileValues[i] = 0;

  int currentQuantile(0);

  double relativeCurrentCount(0.0);

  for ( typename HistogramType::iterator iter = histogram->begin();
        iter != histogram->end();
        ++iter )
  {
    TPixel grayvalue = iter->first;
    long int count = iter->second;

    double relativeCount = static_cast<double>(count) / static_cast<double>(totalCount);

    relativeCurrentCount += relativeCount;

    while ( relativeCurrentCount >= quantiles[currentQuantile] )
    {
      histogramQuantileValues[currentQuantile] = grayvalue;
      ++currentQuantile;
    }
  }

  // report histogram values
  report << "        Minimum: " << (double)minimum
         << "\n  5% quantile: " << (double)histogramQuantileValues[5]
         << "\n 25% quantile: " << (double)histogramQuantileValues[25]
         << "\n 50% quantile: " << (double)histogramQuantileValues[50]
         << "\n 75% quantile: " << (double)histogramQuantileValues[75]
         << "\n 95% quantile: " << (double)histogramQuantileValues[95]
         << "\n      Maximum: " << (double)maximum
         << "\n         Mean: " << mean
         << "\n           SD: " << sd
         << "\n";
}

std::string mitk::CalculateGrayValueStatisticsTool::GetReport() const
{
  return m_CompleteReport.str();
}

