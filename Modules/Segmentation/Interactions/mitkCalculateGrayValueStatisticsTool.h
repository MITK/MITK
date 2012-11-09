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

#ifndef mitkCalculateGrayValueStatisticsTool_h_Included
#define mitkCalculateGrayValueStatisticsTool_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkSegmentationsProcessingTool.h"

#ifndef __itkHistogram_h
#include <itkHistogram.h>
#endif

#include <itkImage.h>
#include <sstream>

namespace mitk
{

/**
 \brief Calculates some gray value statistics for segmentations.

 \ingroup ToolManagerEtAl
 \sa mitk::Tool
 \sa QmitkInteractiveSegmentation

 Last contributor: $Author$
*/
class Segmentation_EXPORT CalculateGrayValueStatisticsTool : public SegmentationsProcessingTool
{
  public:

  Message<> StatisticsCompleted;

    mitkClassMacro(CalculateGrayValueStatisticsTool, SegmentationsProcessingTool);
    itkNewMacro(CalculateGrayValueStatisticsTool);

  virtual const char** GetXPM() const;
  virtual const char* GetName() const;

  virtual std::string GetReport() const;
  //
  // Insight/Code/Review/Algorithms version of Histogram takes
  // only one template parameter, and the 'release' version
  // takes 2, but the default value for the second, 1, is what
  // was specified here.
  typedef itk::Statistics::Histogram<double> HistogramType;
  HistogramType::Pointer m_ITKHistogram;

  HistogramType::ConstPointer GetHistogram();

  typedef HistogramType::MeasurementType HistogramMeasurementType;

protected:

  CalculateGrayValueStatisticsTool(); // purposely hidden
  virtual ~CalculateGrayValueStatisticsTool();

  virtual void StartProcessingAllData();
    virtual bool ProcessOneWorkingData( DataNode* node );
  virtual void FinishProcessingAllData();

  virtual std::string GetErrorMessage();

  /**
   Calculates the minimum and maximum of the pixelvalues. They have to be known to initialize the histogram.
   */
  template<typename TPixel, unsigned int VImageDimension>
  void CalculateMinMax(itk::Image<TPixel, VImageDimension>* referenceImage, Image* segmentation,
      TPixel& minimum, TPixel& maximum);

  /**
   - initializes and fills the histogram
   - calculates mean, sd and quantiles
   */
  template<typename TPixel, unsigned int VImageDimension>
  void ITKHistogramming(itk::Image<TPixel, VImageDimension>* referenceImage, Image* segmentation,
      std::stringstream& report);

  std::stringstream m_CompleteReport;

};

} // namespace

#endif

