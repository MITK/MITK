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
*/
class Segmentation_EXPORT LiveWireTool2D : public SegTool2D
{
  public:
    
    mitkClassMacro(LiveWireTool2D, SegTool2D);
    itkNewMacro(LiveWireTool2D);

    virtual const char** GetXPM() const;
    virtual const char* GetName() const;

  protected:
    
    LiveWireTool2D();
    virtual ~LiveWireTool2D();

    virtual void Activated();
    virtual void Deactivated();
    
    //initialize tool
    virtual bool OnInitLiveWire (Action*, const StateEvent*);

    //add a control point and finish current segment
    virtual bool OnAddPoint   (Action*, const StateEvent*);

    // actual LiveWire computation
    virtual bool OnMouseMoved(Action*, const StateEvent*);

    //check double click on first control point to finish the LiveWire tool
    virtual bool OnCheckPoint(Action*, const StateEvent*);

    //finish LiveWire tool
    virtual bool OnFinish(Action*, const StateEvent*);

    //close the contour
    virtual bool OnCloseContour(Action*, const StateEvent*);


    //the contour already set by the user
    mitk::ContourModel::Pointer m_Contour;
    //the corresponding datanode
    mitk::DataNode::Pointer m_ContourModelNode;

    //the current LiveWire computed contour
    mitk::ContourModel::Pointer m_LiveWireContour;
    //the corresponding datanode
    mitk::DataNode::Pointer m_LiveWireContourNode;

    //the current reference image
    Image::Pointer m_WorkingSlice;
};

} // namespace

#endif


