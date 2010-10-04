/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-08-25 13:01:29 +0200 (Mi, 25 Aug 2010) $
Version:   $Revision: 25766 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKMOVESURFACEINTERACTOR_H_HEADER_INCLUDED
#define MITKMOVESURFACEINTERACTOR_H_HEADER_INCLUDED

#include <mitkInteractor.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Interaction to move a surface by the arrow keys. See tutorial step 10 for explanation
   *
   * \ingroup Interaction
   */
  class MITK_CORE_EXPORT MoveSurfaceInteractor : public Interactor
  {
  public:
    mitkClassMacro(MoveSurfaceInteractor, Interactor);
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
    MoveSurfaceInteractor(const char * type, DataNode* dataNode);

    /**
     * \brief Default Destructor
     **/
    virtual ~MoveSurfaceInteractor();

    /**
    * @brief Convert the given Actions to Operations and send to data and UndoController
    **/
    virtual bool ExecuteAction( Action* action, mitk::StateEvent const* stateEvent );
  };
}

/** 
\example mitkMoveSurfaceInteractor.h
 * This is an example of how to implement a new Interactor.
 * See more details about this example in tutorial Step10.
 */

#endif /* MITKMOVESURFACEINTERACTOR_H_HEADER_INCLUDED */
