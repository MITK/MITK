#ifndef MITKFOCUSMANAGER_H_HEADER_INCLUDED_C135A197
#define MITKFOCUSMANAGER_H_HEADER_INCLUDED_C135A197

#include "mitkCommon.h"
#include "BaseRenderer.h"
#include <vector>
#include <itkEventObject.h>

namespace mitk {

//##ModelId=3EDCAF46031B
//##Documentation
//## @brief manages a list of BaseRenderer.
//## @ingroup Interaction
//## 
//## A focuspointer can be set and read.
//## GoToNext can be used to switch through the list.
//## if the switch m_Loop is set to true, GetNext loops through the list; after
//## the last it goes to the first.
//## if it is not set, it returnes NULL if it steps behind the last Widget.
  class FocusManager : public itk::Object
{
  public:
  mitkClassMacro(FocusManager, itk::LightObject);

  //##ModelId=3F01770802B7
	typedef mitk::BaseRenderer FocusElement;
	//##ModelId=3F01770802C7
	typedef std::vector<FocusElement*> FocusElementList;
	//##ModelId=3F01770802D7
	typedef std::vector<FocusElement*>::iterator FocusListIterator;
	
  //##Documentation
  //## Destructor
  ~FocusManager();

  //##ModelId=3EDCAF7901C3
  //##Documentation
  //## Adds the widget into the set of managed Widgets after the focused
  //## widget and sets the focus to the added one
  bool AddElement(FocusElement *element);

  //##ModelId=3EDCAF790230
	//##Documentation
  //## removes the given widget from the list.
	//## true if found and removed, else false
	//## afterwards the focused widget is the one behind the deleted
	//## or if the deleted was the last, then the one before the deleted
	//## that way you can delete sequentialy from the back on or from front to back
	bool RemoveElement(FocusElement *element);

  //##ModelId=3EDCAF7902BD
  //##Documentation
  //## returns the focused Widget
  FocusElement const* GetFocused() const;

  //##ModelId=3EDCAF7902FB
  //##Documentation
  //## searches the given Widget in List;
  //## if found, sets the focus to this widget and returns true
  bool SetFocused(FocusElement *element);

  //##ModelId=3EDCAF790378
  //##Documentation
  //## returns, if this focused widget points behind the end of the List
  bool IsLast();

  //##ModelId=3EDCAF7903A7
  //##Documentation
  //## returns true, if the focused widget is the first in the list
  bool IsFirst();

  //##ModelId=3EDCAF7903D6
  //##Documentation
  //## returns the first widget in list
  FocusElement const* GetFirst() const;

  //##ModelId=3EDCAF7A002D
  //##Documentation
  //## returns the last widget in list
  FocusElement const* GetLast() const;

	//##ModelId=3EDCAF7A007B
  //##Documentation
  //## sets the focus to the next in list
  //## loops the list, if switch loop is true
	//## returns true if successful, else false
  bool GoToNext();
    
  //##ModelId=3EDCAF7A00AA
	//##Documentation
  //## returns an iterator, that points to the 
	//## beginning of the list
	//## no changes are made to the current focused element
	FocusListIterator GetIter();

  //##ModelId=3EDCAF7A00D8
	//##Documentation
  //## Sets the LoopMode.
	//## if set to true-> the one after the last is the first
	void SetLoop(bool loop);

  friend class GlobalInteraction;	
protected:
  //##ModelId=3EDCAF790194
	//##Documentation
  //## Constructor
  FocusManager();

private:
  //##ModelId=3F0185C802C7
	//##Documentation
  //## stores the Widgets
	FocusElementList m_FocusList;

  //##ModelId=3F0185C802D7
	//##Documentation
  //## holds the focused Widget
	FocusElement *m_FocElement;

  //##ModelId=3EDCAF790155
	//##Documentation
  //## switch which sets the LoopMode.
	//## if true, then the next after the last one is the first
	bool m_Loop;
};

//##Documentation
//## @brief connect to this Event to get noticed when the focus changes
itkEventMacro( FocusEvent , itk::AnyEvent );

} // namespace mitk



#endif /* MITKFOCUSMANAGER_H_HEADER_INCLUDED_C135A197 */
