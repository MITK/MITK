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

#ifndef mitkAutoCropTool_h_Included
#define mitkAutoCropTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"

namespace mitk
{

/**
  \brief Crops selected segmentations.

  \ingroup Reliver
  \sa mitk::Tool
  \sa QmitkSliceBasedSegmentation

  Last contributor: $Author$
*/
class MITK_CORE_EXPORT AutoCropTool : public SegmentationsProcessingTool
{
  public:
    
    mitkClassMacro(AutoCropTool, SegmentationsProcessingTool);
    itkNewMacro(AutoCropTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    virtual bool ProcessOneWorkingData( DataTreeNode* node );
    virtual std::string GetErrorMessage();

    AutoCropTool(); // purposely hidden
    virtual ~AutoCropTool();
};

} // namespace

#endif

