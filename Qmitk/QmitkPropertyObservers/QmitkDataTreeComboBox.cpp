#include <QmitkDataTreeComboBox.h>
#include <QmitkPropertyViewFactory.h>

#include <itkCommand.h>
#include <mitkDataTreeFilterEvents.h>
#include <mitkDataTreeFilterFunctions.h>
#include <mitkImage.h>

#include <qobjectlist.h>

#include <stdexcept>

/**
  Initializes QmitkDataTreeComboBox from nothing. Results in an empty widget. Call SetDataTree or GetFilter later to fill widget with items.

  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeComboBox::QmitkDataTreeComboBox(QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL),
  m_CurrentNode(NULL)
{
  initialize();
}

/**
  Initializes DataTreeComboBox from a mitk::DataTreeFilter. 

  \param filter pointer to the mitk::DataTreeFilter to be displayed
  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeFilter* filter,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(filter),
  m_CurrentNode(NULL)
{
  initialize();
  SetFilter(filter);
}

/**
  Initializes DataTreeComboBox from a mitk::DataTreeBase. Will create a private mitk::DataTreeFilter, containing only the "name" and "visible" properties of anything

  \param tree pointer to the mitk::DataTreeBase to be displayed (after creating a private filter)
  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeBase* tree,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL),
  m_CurrentNode(NULL)
{
  initialize();
  SetDataTree(tree);
}

/**
  Initializes DataTreeListView from a mitk::DataTreeIteratorBase. Will create a private mitk::DataTreeFilter, containing only the "name" and "visible" properties of anything

  \param iterator  pointer to the mitk::DataTreeIteratorBase, which points anywhere into the data tree to be displayed (after creating a private filter)
  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeIteratorBase* iterator,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL),
  m_CurrentNode(NULL)
{
  initialize();
  SetDataTree(iterator);
}

/**
  This is called by all constructors. Initializes some members.
*/
void QmitkDataTreeComboBox::initialize()
{
  // member initializations that are equal for all constructors
  m_SkipItem = NULL;
  m_SkipItemParent = NULL;
  m_SelfCall = false;
  m_DisplayedProperty.clear();
  m_UserSetDisplayedProperty.clear();
  m_AutoUpdate = false;

  connect(this, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}

/**
  Minor cleanup. Disconnect all itk-observers.
*/
QmitkDataTreeComboBox::~QmitkDataTreeComboBox()
{
  disconnectNotifications();
}

/**
  Creates new itk-observers for the notifications of mitk::DataTreeFilter. Connects them to ...Handler(itk::EventObject) functions.
*/
void QmitkDataTreeComboBox::connectNotifications()
{
  if (!m_DataTreeFilter) return;
  
  // connect to our filter's notifications
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command1 = itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command1->SetCallbackFunction(this, &QmitkDataTreeComboBox::removeItemHandler);
  m_RemoveItemConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveItemEvent(), command1);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command2= itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command2->SetCallbackFunction(this, &QmitkDataTreeComboBox::removeChildrenHandler);
  m_RemoveChildrenConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveChildrenEvent(), command2);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command3 = itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command3->SetCallbackFunction(this, &QmitkDataTreeComboBox::removeAllHandler);
  m_RemoveAllConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveAllEvent(), command3);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command4 = itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command4->SetCallbackFunction(this, &QmitkDataTreeComboBox::selectionChangedHandler);
  m_SelectionChangedConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterSelectionChangedEvent(), command4);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command5 = itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command5->SetCallbackFunction(this, &QmitkDataTreeComboBox::itemAddedHandler);
  m_ItemAddedConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterItemAddedEvent(), command5);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::Pointer command6 = itk::ReceptorMemberCommand<QmitkDataTreeComboBox>::New();
  command6->SetCallbackFunction(this, &QmitkDataTreeComboBox::updateAllHandler);
  m_UpdateAllConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterUpdateAllEvent(), command6);
  }
}

/**
  removes all itk-Observers
*/
void QmitkDataTreeComboBox::disconnectNotifications()
{
  if (!m_DataTreeFilter) return;

  m_DataTreeFilter->RemoveObserver( m_RemoveItemConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveChildrenConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveAllConnection );
  m_DataTreeFilter->RemoveObserver( m_SelectionChangedConnection );
  m_DataTreeFilter->RemoveObserver( m_ItemAddedConnection );
  m_DataTreeFilter->RemoveObserver( m_UpdateAllConnection );
} 

/**
  User provides only a data tree and does not tell, what should be displayed. So this method creates a private
  mitk::DataTreeFilter, determines only "name" to be visible.
  Then the pointer m_DataTreeFilter is set to this private filter.

  \param tree Tree to display
*/
void QmitkDataTreeComboBox::SetDataTree(mitk::DataTreeBase* tree)
{
  if (
       tree 
       && 
       (   ( m_DataTreeFilter.IsNull() )
         ||
           ( m_DataTreeFilter->GetDataTree() != tree )
       )
     )
  {
    disconnectNotifications(); 

    // create default filter with visibility (editable) and name (non-editable)
    m_PrivateFilter = mitk::DataTreeFilter::New(tree);
    m_PrivateFilter->SetAutoUpdate( m_AutoUpdate ); 
    m_PrivateFilter->SetFilter( mitk::IsBaseDataType<mitk::Image>() );
    m_PrivateFilter->SetSelectionMode(mitk::DataTreeFilter::SINGLE_SELECT);
    mitk::DataTreeFilter::PropertyList visible;
    visible.push_back("name");
    m_PrivateFilter->SetVisibleProperties(visible);
    
    m_DataTreeFilter = m_PrivateFilter;
    connectNotifications(); // add observers
  
    SetDisplayedProperty("name");
    generateItems();
  }
  else if (tree)
  {
    m_DataTreeFilter->Update();
  }
  else if (!tree)
  {
    disconnectNotifications(); 
    m_DataTreeFilter = NULL;
    m_PrivateFilter = NULL;
    generateItems();
  }
}

/**
  Get the tree behind the iterator and call the other SetDataTree()

  \param iterator  pointer to the mitk::DataTreeIteratorBase, which points anywhere into the data tree to be displayed (after creating a private filter)
*/
void QmitkDataTreeComboBox::SetDataTree(mitk::DataTreeIteratorBase* iterator)
{
  // create default filter if neccessary
  if (iterator && iterator->GetTree())
    SetDataTree(iterator->GetTree());
}

/**
  Display items from the mitk::DataTreeFilter provided in \a filter. Warn if filter's selection mode is not SINGLE_SELECT,
  because a combobox can only mark one object as selected.
  
  \param filter pointer to the mitk::DataTreeFilter to be displayed
*/
void QmitkDataTreeComboBox::SetFilter(mitk::DataTreeFilter* filter)
{
  disconnectNotifications();
  m_DataTreeFilter = filter;
  // in the case that somebody first sets a datatree and then a filter
  // destroy the default filter that was created in SetDataTree
  if (filter != m_PrivateFilter.GetPointer())
    m_PrivateFilter = NULL;
  connectNotifications(); // add observers
  
  if ( m_DataTreeFilter->GetSelectionMode() != mitk::DataTreeFilter::SINGLE_SELECT )
  {
    std::cerr << "---------------------------------------------------------------------------"
              << "QmitkDataTreeComboBoxes work only correctly with SINGLE_SELECT data tree filters!"
              << std::endl
              << "The filter you assigned in " << __FILE__ << ", l. " << __LINE__ << " has a differenct SelectionMode. Please change that."
              << std::endl
              << "If you don't know what this means, ask your programmer about it."
              << std::endl
              << "---------------------------------------------------------------------------"
              << std::endl;
  }
  
  determineDisplayedProperty();
  generateItems();
}

/**
  Get the currently active mitk::DataTreeFilter. May either be user provided or created privatly for some data tree.
  
  \return active mitk::DataTreeFilter
*/
mitk::DataTreeFilter* QmitkDataTreeComboBox::GetFilter()
{
  return m_DataTreeFilter;
}

void QmitkDataTreeComboBox::SetAutoUpdate(bool autoUpdatesEnabled)
{
  if ( autoUpdatesEnabled != m_AutoUpdate )
  {
    if (m_DataTreeFilter) m_DataTreeFilter->SetAutoUpdate(autoUpdatesEnabled);

    m_AutoUpdate = autoUpdatesEnabled;
  }
}

void QmitkDataTreeComboBox::Update()
{
  if (m_DataTreeFilter)
    m_DataTreeFilter->Update();
}


/**
  Called whenever the user selects a different item. The task here is to find the belonging item and change its selection status.
  The formerly selected item is correctly deselected by mitk::DataTreeFilter::Item::SetSelected(bool), when the DataTreeFilter's
  selection mode is SINGLE_SELECT.
*/
void QmitkDataTreeComboBox::onActivated(int index)
{
  // find item, generate signal
  try
  {
    // following cast is due to bad design... class Item needs needs rework 
    m_SelfCall = true;
    const_cast<mitk::DataTreeFilter::Item*>(m_Items.at(index))->SetSelected(true);
    const mitk::DataTreeFilter::Item* currentItem = m_Items.at(index);
    m_CurrentNode = currentItem->GetNode();
    emit activated( currentItem );
    m_SelfCall = false;
  }
  catch (std::out_of_range)
  {
  }
}

/**
  If the user has not determined (via SetDisplayedProperty), which property should be displayed, this method
  tries to find a mitk::StringProperty in the DataTreeFilter's list of available properties.

  Defaults to the "name" property.
*/
void QmitkDataTreeComboBox::determineDisplayedProperty()
{
  if ( !m_UserSetDisplayedProperty.empty() )
    if (std::find( m_DataTreeFilter->GetVisibleProperties().begin(), m_DataTreeFilter->GetVisibleProperties().end(), m_UserSetDisplayedProperty )
        != m_DataTreeFilter->GetVisibleProperties().end() )
    {
      m_DisplayedProperty = m_UserSetDisplayedProperty;
      return;
    }

  // if this line is reached, either the user has not chosen a property to be displayed, or 
  // that property is not available from the filter used.
  
  m_DisplayedProperty.clear();

  // This (often) determines the key of the left-most string property.
  // If such a property cannot be determined, "name" is used as a default.

  mitk::DataTreeFilter::ConstItemIterator itemiterend( m_DataTreeFilter->GetItems()->End() ); 
  --itemiterend; // last but one item (first items are often empty, just for structure)
  if (itemiterend != m_DataTreeFilter->GetItems()->End()) 
  { 
    // visible properties left to right
    for( mitk::DataTreeFilter::PropertyList::const_iterator nameiter = m_DataTreeFilter->GetVisibleProperties().begin();
        nameiter != m_DataTreeFilter->GetVisibleProperties().end();
        ++nameiter )
    {
      // check if this is a string item
      const mitk::BaseProperty* property( itemiterend->GetProperty(*nameiter) );
  
      if ( const mitk::StringProperty* prop = dynamic_cast<const mitk::StringProperty*>(property) )
      {
        // success, use this for display
        m_DisplayedProperty = prop->GetValue();
        break;
      }
    }
  }
  
  m_DisplayedProperty = "name";
}

/**
  Let the user set the property to be displayed. Then regenerate items.

  \param prop String key to the property to be displayed
*/
void QmitkDataTreeComboBox::SetDisplayedProperty(std::string prop)
{
  m_UserSetDisplayedProperty = prop;
  determineDisplayedProperty();
  generateItems();
}


/**
  This method creates combo box items for the item list that is provided by a mitk::DataTreeFilter.
  
  \param items The top level item list (items may have children, then this method calls itself recursively)
  \param visibleProps Properties for which display widgets should be created
  \param level Level of recursion. Needed to display the right amout of indentation for child items.
*/
void QmitkDataTreeComboBox::AddItemsToList(const mitk::DataTreeFilter::ItemList* items,
                                           const mitk::DataTreeFilter::PropertyList& visibleProps,
                                           int level)
{
  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 
  
  while ( itemiter != itemiterend ) // for all items
  {

    if (*itemiter == m_SkipItem)
    {
      // skip the item
      ++itemiter;
      continue;
    }
   
    // use first string property of visible properties list
    QString displayedText( ((const std::string)itemiter->GetProperty(m_DisplayedProperty)).c_str() );

    if ( m_DataTreeFilter->GetHierarchyHandling() != mitk::DataTreeFilter::FLATTEN_HIERARCHY )
    {
      displayedText = QString(" ") + displayedText;
      for (int i = 0; i < level; ++i)
        displayedText = QString("-") + displayedText;
    }
     
    m_Items.push_back(*itemiter);
    QComboBox::insertItem(displayedText);

    if (itemiter->GetNode() == m_CurrentNode)
    {
      QComboBox::setCurrentItem( QComboBox::count()-1 ); // select the recently added item
    }
 
    if ( itemiter->HasChildren() )
    {
      // add children, unless this item is the parent whose children were just deleted
      if (*itemiter != m_SkipItemParent)
        AddItemsToList(itemiter->GetChildren(), visibleProps, level+1);
    }
    
    ++itemiter; 
  }
}

/**
  Deletes all child widgets. 
*/
void QmitkDataTreeComboBox::clearItems()
{
  QComboBox::clear(); // seems not to work
  m_Items.clear();
} 

/**
  Regenerates all displayed items.
*/
void QmitkDataTreeComboBox::generateItems()
{
  if (!m_DataTreeFilter) return;

  clearItems();
  
  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );

  // fill rows with property views for the visible items 
  if (m_SkipItemParent != (mitk::DataTreeFilter::Item*) -1)
    AddItemsToList(m_DataTreeFilter->GetItems(), visibleProps, 0);

  try
  {
    if ( (QComboBox::currentItem() < 0) || (m_Items.size() == 0) )
    {
      // nothing selected (e.g. because there are no items)
      emit selectionCleared();
      return;
    }
   
    mitk::DataTreeFilter::Item* currentItem = const_cast<mitk::DataTreeFilter::Item*>(m_Items.at( QComboBox::currentItem() ));
    if ( currentItem->GetNode() != m_CurrentNode )
    {
      m_CurrentNode = currentItem->GetNode(); 
      m_SelfCall = true;
      currentItem->SetSelected(true);
      emit activated(currentItem);
      m_SelfCall = false;
    }
  }
  catch (std::out_of_range)
  {
  }
}

/**
  Handles TreeFilterRemoveItemEvents from the DataTreeFilter. For this purpose a variable m_SkipItem is set to contain the
  item, which will shortly be deleted. This is necessary because the notification is sent while the item still exists.
  m_SkipItem is considered in generateItem().
  Perhaps something more sophisticated could be implemented.
*/
void QmitkDataTreeComboBox::removeItemHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveItemEvent& event( static_cast<const mitk::TreeFilterRemoveItemEvent&>(e) );
  m_SkipItem = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
  m_SkipItem = NULL;
}

/**
  Handles TreeFilterRemoveChildrenEvents from the DataTreeFilter. For this purpose a variable m_SkipItemParent is set to contain the
  item, which will shortly be deleted. This is necessary because the notification is sent while the item still exists.
  m_SkipItemParent is considered in generateItem().
  Perhaps something more sophisticated could be implemented.
*/
void QmitkDataTreeComboBox::removeChildrenHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveChildrenEvent& event( static_cast<const mitk::TreeFilterRemoveChildrenEvent&>(e) );
  // TODO: do something more clever
  m_SkipItemParent = event.GetChangedItem();
  if ( m_SkipItemParent == NULL ) 
    m_SkipItemParent = (mitk::DataTreeFilter::Item*) -1; // in generateItems --> don't draw any content
  generateItems();
  m_SkipItemParent = NULL;
}

/**
  Handles TreeFilterRemoveAllEvents from the DataTreeFilter. The implementation is extremely easy, as clearing all items is
  needed in other contexts as well.
*/
void QmitkDataTreeComboBox::removeAllHandler( const itk::EventObject& )
{
  clearItems();
}

/**
  Handles TreeFilterSelectionChangedEvents from the DataTreeFilter. 
*/
void QmitkDataTreeComboBox::selectionChangedHandler( const itk::EventObject& e)
{
  if (m_SelfCall) return; // invoked by this object
 
  const mitk::TreeFilterSelectionChangedEvent& event( static_cast<const mitk::TreeFilterSelectionChangedEvent&>(e) );
  const mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  if (!event.IsSelected()) return; // only positive selections
  
  // find item, change selection, done
  int row(0);
  try
  {
    for (; row < QComboBox::count(); ++row)
    {
      if ( m_Items.at(row) == item )
      {
        QComboBox::setCurrentItem(row);
        m_CurrentNode = m_Items.at(row)->GetNode();
        emit activated(m_Items.at(row));
        break;
      }
    }
    emit selectionCleared();
  }
  catch (std::out_of_range)
  {
  }

}

/**
  Handles TreeFilterItemAddedEvents from the DataTreeFilter. Implemented by simply regenerating everything.
  Perhaps something more sophisticated could be implemented.
*/
void QmitkDataTreeComboBox::itemAddedHandler( const itk::EventObject& /*e*/ )
{
  //const mitk::TreeFilterItemAddedEvent& event( static_cast<const TreeFilterItemAddedEvent&>(e) );
  //mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
}

/**
  Handles TreeFilterUpdateAllEvents from the DataTreeFilter. 
*/
void QmitkDataTreeComboBox::updateAllHandler( const itk::EventObject& )
{
  determineDisplayedProperty();
  generateItems();
}

