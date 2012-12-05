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


  \warning Make sure the working image is properly set, otherwise the algorithm for computing livewire contour segments will not work!

  */
  class Segmentation_EXPORT ContourModelLiveWireInteractor : public ContourModelInteractor
  {
  public:

    mitkClassMacro(ContourModelLiveWireInteractor, ContourModelInteractor);
    mitkNewMacro1Param(Self, DataNode*);


    virtual void SetWorkingImage (mitk::Image* _arg)
    {
      if (this->m_WorkingImage != _arg)
      {
        this->m_WorkingImage = _arg;
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

    int SplitContourFromSelectedVertex(mitk::ContourModel* sourceContour,
      mitk::ContourModel* destinationContour,
      bool fromSelectedUpwards,
      int timestep);

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;
    mitk::Image::Pointer m_WorkingImage;

    mitk::Point3D m_NextActiveVertexDown;
    mitk::Point3D m_NextActiveVertexUp;
    mitk::ContourModel::Pointer m_ContourLeft;
    mitk::ContourModel::Pointer m_ContourRight;

  };

} // namespace

#endif
