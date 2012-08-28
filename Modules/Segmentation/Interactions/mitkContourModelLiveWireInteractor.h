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

#ifndef mitkContourModelLiveWireInteractor_h_Included
#define mitkContourModelLiveWireInteractor_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModelInteractor.h"
#include <mitkContourModel.h>
#include <mitkDataNode.h>

#include <mitkImageLiveWireContourModelFilter.h>

namespace mitk
{


  /**
  \brief 

  \sa Interactor
  \sa ContourModelInteractor

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  \warning Only to be instantiated by mitk::ToolManager.

  */
  class Segmentation_EXPORT ContourModelLiveWireInteractor : public ContourModelInteractor
  {
  public:

    mitkClassMacro(ContourModelLiveWireInteractor, ContourModelInteractor);
    mitkNewMacro1Param(Self, DataNode*);


    itkSetObjectMacro(WorkingImage, mitk::Image);
    virtual void SetWorkingImage (mitk::Image* _arg)
    { 
      if (this->m_SetWorkingImage != _arg)
      { 
        this->m_SetWorkingImage = _arg;
        this->m_LiveWireFilter->SetInput(this->m_WorkingImage);
        this->Modified(); 
      } 
    } 

  protected:

    ContourModelLiveWireInteractor(DataNode* dataNode);
    virtual ~ContourModelLiveWireInteractor();


    virtual bool OnDeletePoint(Action*, const StateEvent*);
    virtual bool OnMovePoint(Action*, const StateEvent*);
    virtual bool OnCheckPointClick( Action* action, const StateEvent* stateEvent);

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;
    mitk::Image::Pointer m_WorkingImage;

  };

} // namespace

#endif


