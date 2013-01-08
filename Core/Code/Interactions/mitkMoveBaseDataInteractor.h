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


#ifndef MITKMoveBaseDataInteractor_H_HEADER_INCLUDED
#define MITKMoveBaseDataInteractor_H_HEADER_INCLUDED

#include <mitkInteractor.h>
#include <mitkColorProperty.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Interaction to move a surface by the arrow keys. See tutorial step 10 for explanation
   *
   * \ingroup Interaction
   */
  class MITK_CORE_EXPORT MoveBaseDataInteractor : public Interactor
  {
  public:
    mitkClassMacro(MoveBaseDataInteractor, Interactor);
    mitkNewMacro2Param(Self, const char*, DataNode*);

    /**
     * \brief check how good an event can be handled
     */
    //virtual float CanHandleEvent(StateEvent const* stateEvent) const;
    //used from mitkInteractor

    /**
    *@brief Gets called when mitk::DataNode::SetData() is called
    *
    * No need to use it here, because the pattern won't be complex
    * and we can take care of unexpected data change
    **/
    //virtual void DataChanged(){};

  protected:
    /**
     * \brief Constructor
     */
    MoveBaseDataInteractor(const char * type, DataNode* dataNode);

    /**
     * \brief Default Destructor
     **/
    virtual ~MoveBaseDataInteractor();

    /**
    * @brief Convert the given Actions to Operations and send to data and UndoController
    **/
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );

    mitk::ColorProperty::Pointer m_NodeColor;
  };
}

/**
\example mitkMoveBaseDataInteractor.h
 * This is an example of how to implement a new Interactor.
 * See more details about this example in tutorial Step10.
 */

#endif /* MITKMoveBaseDataInteractor_H_HEADER_INCLUDED */
