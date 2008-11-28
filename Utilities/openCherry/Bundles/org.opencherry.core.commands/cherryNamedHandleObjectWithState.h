/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef CHERRYNAMEDHANDLEOBJECTWITHSTATE_H_
#define CHERRYNAMEDHANDLEOBJECTWITHSTATE_H_

#include "common/cherryNamedHandleObject.h"
#include "cherryIObjectWithState.h"

#include <vector>
#include <map>

namespace cherry {

/**
 * <p>
 * A named handle object that can carry state with it. This state can be used to
 * override the name or description.
 * </p>
 * <p>
 * Clients may neither instantiate nor extend this class.
 * </p>
 * 
 * @since 3.2
 */
class CHERRY_COMMANDS NamedHandleObjectWithState : public NamedHandleObject, public IObjectWithState {

public:
  
  cherryClassMacro(NamedHandleObjectWithState);
  
  void AddState(const std::string& stateId, const State::ConstPointer state);

  std::string GetDescription();

  std::string GetName();

  const State GetState(const std::string& stateId);

  std::vector<std::string> GetStateIds();

  void RemoveState(const std::string& id);
  
  private:

    /**
     * The map of states currently held by this command. If this command has no
     * state, then this will be <code>null</code>.
     */
    std::map<std::string, State::Pointer> states;

  protected:
    
    /**
     * Constructs a new instance of <code>NamedHandleObject<WithState/code>.
     * 
     * @param id
     *            The identifier for this handle; must not be <code>null</code>.
     */
    NamedHandleObjectWithState(const std::string& id);

};

}

#endif /*CHERRYNAMEDHANDLEOBJECTWITHSTATE_H_*/
