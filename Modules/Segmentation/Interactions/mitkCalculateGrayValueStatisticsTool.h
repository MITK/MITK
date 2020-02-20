/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCalculateGrayValueStatisticsTool_h_Included
#define mitkCalculateGrayValueStatisticsTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"
#include <MitkSegmentationExports.h>

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
  class MITKSEGMENTATION_EXPORT CalculateGrayValueStatisticsTool : public SegmentationsProcessingTool
  {
  public:
    Message<> StatisticsCompleted;

    mitkClassMacro(CalculateGrayValueStatisticsTool, SegmentationsProcessingTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char **GetXPM() const override;
    const char *GetName() const override;

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
    ~CalculateGrayValueStatisticsTool() override;

    void StartProcessingAllData() override;
    bool ProcessOneWorkingData(DataNode *node) override;
    void FinishProcessingAllData() override;

    std::string GetErrorMessage() override;

    /**
     Calculates the minimum and maximum of the pixelvalues. They have to be known to initialize the histogram.
     */
    template <typename TPixel, unsigned int VImageDimension>
    void CalculateMinMax(itk::Image<TPixel, VImageDimension> *referenceImage,
                         Image *segmentation,
                         TPixel &minimum,
                         TPixel &maximum);

    /**
     - initializes and fills the histogram
     - calculates mean, sd and quantiles
     */
    template <typename TPixel, unsigned int VImageDimension>
    void ITKHistogramming(itk::Image<TPixel, VImageDimension> *referenceImage,
                          Image *segmentation,
                          std::stringstream &report);

    std::stringstream m_CompleteReport;
  };

} // namespace

#endif
