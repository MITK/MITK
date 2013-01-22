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

#ifndef MITKEVENTINTERACTOR_H_
#define MITKEVENTINTERACTOR_H_

#include "itkObject.h"
#include "itkSmartPointer.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkEventStateMachine.h"
#include <string.h>

namespace mitk
{
  /** Base class from with interactors that handle DataNodes are to be derived.
   * Provides an interface that is relevant for the interactor to work together with the dispatcher.
   * To implement a new interactor overwrite the ConnectActionsAndFunctions to connect the actions.
   */

  class DataNode;
  class MITK_CORE_EXPORT DataInteractor: public EventStateMachine
  {

  public:
    typedef itk::SmartPointer<DataNode> NodeType;mitkClassMacro(DataInteractor, EventStateMachine)
    ;itkNewMacro(Self)
    ;

    void SetDataNode(NodeType);
    /**
     * Sets the maximum distance that is accepted when looking for a point at a certain position using the GetPointIndexByPosition function.
     */
    void SetAccuracy(float accuracy);

    /**
     * @brief Returns the mode the DataInteractor currently is in. See in mitkDispatcher the description of m_ProcessingMode for further details.
     */
    std::string GetMode();

    NodeType GetDataNode();
    int GetLayer();

  protected:
    DataInteractor();
    virtual ~DataInteractor();
    /**
     * @brief Overwrite this function to connect actions from StateMachine description with functions.
     *
     * Following example shows how to connect the 'addpoint' action from the StateMachine XML description using the CONNECT_FUNCTION macro
     * with the AddPoint() function in the TestInteractor.
     * @code
     * void mitk::TestInteractor::ConnectActionsAndFunctions()
     {
     CONNECT_FUNCTION("addpoint", AddPoint);
     }
     * @endcode
     */
    virtual void ConnectActionsAndFunctions();

    /**
     * @ brief Return index in PointSet of the point that is within given accuracy to the provided position.
     *
     * Assumes that the DataNode contains a PointSet, if so it iterates over all points
     * in the DataNode to check if it contains a point near the pointer position.
     * If a point is found its index-position is returned, else -1 is returned.
     */
    virtual int GetPointIndexByPosition(Point3D position, int time = 0);

    /** \brief Is called when a DataNode is initially set or changed
     *  To be implemented by sub-classes for initialization code which require a DataNode.
     *  \note New DataInteractors usually are expected to have the focus, but this only works if they have the highest Layer,
     *  since empty DataNodes have a layer of -1, the DataNode must be filled here in order to get a layer assigned.
     *  \note Is also called when the DataNode is set to NULL.
     *
     */
    virtual void DataNodeChanged();

    /** \brief Is called when a DataNode is initially set or changed
     *
     *  To be implemented by sub-classes for initialization code which require a DataNode.
     *  \note New DataInteractors usually are expected to have the focus, but this only works if they have the highest Layer,
     *  since empty DataNodes have a layer of -1, the DataNode must be filled here in order to get a layer assigned.
     *  \note Is also called when the DataNode is set to NULL.
     *
     */

  private:
    NodeType m_DataNode;
    float m_SelectionAccuracy;
  };

} /* namespace mitk */
#endif /* MITKEVENTINTERACTOR_H_ */
