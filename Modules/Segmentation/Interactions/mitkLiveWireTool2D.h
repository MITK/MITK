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
#include <mitkContourModel.h>
#include <mitkDataNode.h>

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
class Segmentation_EXPORT LiveWireTool2D : public SegTool2D
{
  public:
    
    mitkClassMacro(LiveWireTool2D, SegTool2D);
    itkNewMacro(LiveWireTool2D);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    LiveWireTool2D(); // purposely hidden
    virtual ~LiveWireTool2D();

    virtual void Activated();
    virtual void Deactivated();
    
    virtual bool OnInitLiveWire (Action*, const StateEvent*);
    virtual bool OnAddPoint   (Action*, const StateEvent*);
    virtual bool OnMouseMoved(Action*, const StateEvent*);
    virtual bool OnCheckPoint(Action*, const StateEvent*);
    virtual bool OnFinish(Action*, const StateEvent*);
    virtual bool OnCloseContour(Action*, const StateEvent*);


    mitk::ContourModel::Pointer m_Contour;
    mitk::DataNode::Pointer m_ContourModelNode;

    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif


