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
#include <MitkSegmentationExports.h>
#include "mitkContourModelInteractor.h"

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
  class MITKSEGMENTATION_EXPORT ContourModelLiveWireInteractor : public ContourModelInteractor
  {
  public:

    mitkClassMacro(ContourModelLiveWireInteractor, ContourModelInteractor);
    mitkNewMacro1Param(Self, DataNode*);

    virtual void SetEditingContourModelNode (mitk::DataNode* _arg);

    virtual void SetWorkingImage (mitk::Image* _arg);

  protected:

    ContourModelLiveWireInteractor(DataNode* dataNode);
    virtual ~ContourModelLiveWireInteractor();

    virtual bool OnDeletePoint(Action*, const StateEvent*) override;
    virtual bool OnMovePoint(Action*, const StateEvent*) override;
    virtual bool OnCheckPointClick( Action* action, const StateEvent* stateEvent) override;
    virtual bool OnFinishEditing( Action* action, const StateEvent* stateEvent) override;

    int SplitContourFromSelectedVertex(mitk::ContourModel* srcContour,
                                       mitk::ContourModel* destContour,
                                       bool fromSelectedUpwards,
                                       int timestep);

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;
    mitk::Image::Pointer m_WorkingSlice;

    mitk::Point3D m_NextActiveVertexDown;
    mitk::Point3D m_NextActiveVertexUp;

    mitk::ContourModel::VertexIterator m_NextActiveVertexDownIter;
    mitk::ContourModel::VertexIterator m_NextActiveVertexUpIter;

    std::vector < itk::Index<2> > m_ContourBeingModified;

    mitk::DataNode::Pointer m_EditingContourNode;
    mitk::ContourModel::Pointer m_ContourLeft;
    mitk::ContourModel::Pointer m_ContourRight;

  };

} // namespace mitk

#endif // mitkContourModelLiveWireInteractor_h_Included
