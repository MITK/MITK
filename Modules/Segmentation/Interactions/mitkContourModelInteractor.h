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

#ifndef mitkContourModelInteractor_h_Included
#define mitkContourModelInteractor_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkInteractor.h"
#include <mitkContourModel.h>
#include <mitkDataNode.h>

namespace mitk
{


/**
  \brief

  \sa Interactor

  \ingroup Interaction
  \ingroup ToolManagerEtAl


*/
class Segmentation_EXPORT ContourModelInteractor : public Interactor
{
  public:

    mitkClassMacro(ContourModelInteractor, Interactor);
    mitkNewMacro1Param(Self, DataNode*);


        /**
     * \brief calculates how good the data, this statemachine handles, is hit
     * by the event.
     *
     * overwritten, cause we don't look at the boundingbox, we look at each point
     */
    virtual float CanHandleEvent(StateEvent const* stateEvent) const;

    /**
    *@brief If data changed then initialize according to numbers of loaded points
    **/
    virtual void DataChanged();

  protected:

    ContourModelInteractor(DataNode* dataNode); // purposely hidden
    virtual ~ContourModelInteractor();


    virtual bool OnCheckPointClick (Action*, const StateEvent*);
    virtual bool OnCheckContourClick   (Action*, const StateEvent*);
    virtual bool OnDeletePoint(Action*, const StateEvent*);
    virtual bool OnMovePoint(Action*, const StateEvent*);
    virtual bool OnMoveContour(Action*, const StateEvent*);
    virtual bool OnFinish(Action*, const StateEvent*);

    mitk::Point3D m_lastMousePosition;

};

} // namespace
#endif
