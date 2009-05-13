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

#ifndef mitkCalculateVolumetryTool_h_Included
#define mitkCalculateVolumetryTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"

namespace mitk
{

/**
  \brief Calculates the segmented volumes for binary images.

  \ingroup Reliver
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
*/
class MITKEXT_CORE_EXPORT CalculateVolumetryTool : public SegmentationsProcessingTool
{
  public:
    
    mitkClassMacro(CalculateVolumetryTool, SegmentationsProcessingTool);
    itkNewMacro(CalculateVolumetryTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    virtual bool ProcessOneWorkingData( DataTreeNode* node );
    virtual std::string GetErrorMessage();

    virtual void FinishProcessingAllData();

    CalculateVolumetryTool(); // purposely hidden
    virtual ~CalculateVolumetryTool();
};

} // namespace

#endif

