/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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
#include "MitkExtExports.h"
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
class MitkExt_EXPORT CreateSurfaceTool : public SegmentationsProcessingTool
{
  public:
    
    mitkClassMacro(CreateSurfaceTool, SegmentationsProcessingTool);
    itkNewMacro(CreateSurfaceTool);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

    void OnSurfaceCalculationDone();

  protected:
    
    virtual bool ProcessOneWorkingData( DataNode* node );
    virtual std::string GetErrorMessage();

    CreateSurfaceTool(); // purposely hidden
    virtual ~CreateSurfaceTool();
};

} // namespace

#endif

