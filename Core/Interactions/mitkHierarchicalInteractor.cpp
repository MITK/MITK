#include "mitkHierarchicalInteractor.h"

mitk::HierarchicalInteractor::HierarchicalInteractor(const char * type, DataTreeNode* dataTreeNode)
:Interactor(type,dataTreeNode)
{
  m_SelectedInteractors.clear();
  m_AllInteractors.clear();
}

bool mitk::HierarchicalInteractor::HandleEvent(StateEvent const* stateEvent, int objectEventId, int groupEventId)
//## as long as an sub-interactor is selected the event will be sent to it. if no one is deselected, then call the original StateMachine::HandleEvent
{
  bool ok = false; //retrun param
  if (!m_SelectedInteractors.empty() && 
    m_Mode == SMSUBSELECTED)
  {
    InteractorListIter it = m_SelectedInteractors.begin();
    InteractorListIter end = m_SelectedInteractors.end();
    while (it != end)
    {
      ok = (*it)->HandleEvent(stateEvent, objectEventId, groupEventId);
      
      mitk::Interactor::ModeType mode = (*it)->GetMode();
      if (mode == SMDESELECTED)
        //if the interactor now is deselected, then erase it from the selection.
      {
        m_SelectedInteractors.erase(it);
        if (m_SelectedInteractors.empty())
        {
          //TODO: write to undo?!?
          m_Mode = SMSELECTED;
          break;
        }
      }
      ++it;//does that work after deletion?
    }
  }
  else
    //send it to the original EventProcess
    ok = StateMachine::HandleEvent(stateEvent, objectEventId, groupEventId);

  return ok;
}

bool mitk::HierarchicalInteractor::AddInteractor(mitk::Interactor::Pointer interactor)
{
  if (interactor.IsNotNull())
  {
    m_AllInteractors.push_back(interactor);
    return true;
  }
  else
    return false;
}