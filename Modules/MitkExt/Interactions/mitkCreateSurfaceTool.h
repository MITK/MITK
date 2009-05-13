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

#ifndef mitkCreateSurfaceTool_h_Included
#define mitkCreateSurfaceTool_h_Included

#include "mitkCommon.h"
#include "mitkSegmentationsProcessingTool.h"

namespace mitk
{

/**
  \brief Creates surface models from segmentations.
  \ingroup Reliver
  \sa mitk::Tool
  \sa QmitkInteractiveSegmentation

  Last contributor: $Author$
*/
class MITKEXT_CORE_EXPORT CreateSurfaceTool : public SegmentationsProcessingTool
{
  public:
    
    mitkClassMacro(CreateSurfaceTool, SegmentationsProcessingTool);
    itkNewMacro(CreateSurfaceTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    void OnSurfaceCalculationDone();

  protected:
    
    virtual bool ProcessOneWorkingData( DataTreeNode* node );
    virtual std::string GetErrorMessage();

    CreateSurfaceTool(); // purposely hidden
    virtual ~CreateSurfaceTool();
};

} // namespace

#endif

