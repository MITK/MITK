/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkCalculateGrayValueStatisticsTool_h_Included
#define mitkCalculateGrayValueStatisticsTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"

#include <itkImage.h>
#include <sstream>

namespace mitk
{

/**
  \brief Calculates some gray value statistics for segmentations.

  \ingroup Reliver
  \sa mitk::Tool
  \sa QmitkSliceBasedSegmentation

  Last contributor: $Author$
*/
class MITK_CORE_EXPORT CalculateGrayValueStatisticsTool : public SegmentationsProcessingTool
{
  public:
  
    Message StatisticsCompleted;
    
    mitkClassMacro(CalculateGrayValueStatisticsTool, SegmentationsProcessingTool);
    itkNewMacro(CalculateGrayValueStatisticsTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    virtual std::string GetReport() const;

  protected:
    
    CalculateGrayValueStatisticsTool(); // purposely hidden
    virtual ~CalculateGrayValueStatisticsTool();
    
    virtual void StartProcessingAllData();
    virtual bool ProcessOneWorkingData( DataTreeNode* node );
    virtual void FinishProcessingAllData();

    virtual std::string GetErrorMessage();

    template <typename TPixel, unsigned int VImageDimension>
    void ITKHistogramming( itk::Image<TPixel, VImageDimension>* referenceImage, Image* segmentation, std::stringstream& report );

    std::stringstream m_CompleteReport;
};

MITK_TOOL_FACTORY_MACRO(MITK_CORE_EXPORT, CalculateGrayValueStatisticsTool, "Calculates some gray value statistics for segmentations")

} // namespace

#endif

