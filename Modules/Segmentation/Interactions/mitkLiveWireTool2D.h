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

#ifndef mitkCorrectorTool2D_h_Included
#define mitkCorrectorTool2D_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkSegTool2D.h"

namespace mitk
{

class Image;

/**
  \brief 

  \sa SegTool2D
  
  \ingroup Interaction
  \ingroup ToolManagerEtAl

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class Segmentation_EXPORT LiveWireTool2D.cpp : public SegTool2D
{
  public:
    
    mitkClassMacro(LiveWireTool2D, SegTool2D);
    itkNewMacro(LiveWireTool2D);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    LiveWireTool2D(int paintingPixelValue = 1); // purposely hidden
    virtual ~LiveWireTool2D();

    virtual void Activated();
    virtual void Deactivated();
    
    virtual bool OnMousePressed (Action*, const StateEvent*);
    virtual bool OnMouseMoved   (Action*, const StateEvent*);
    virtual bool OnMouseReleased(Action*, const StateEvent*);

    int m_PaintingPixelValue;

    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif


