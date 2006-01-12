#include <QmitkDataTreeComboBox.h>
#include <QmitkPropertyViewFactory.h>

#include <itkCommand.h>
#include <mitkDataTreeFilterEvents.h>

#include <qobjectlist.h>

#include <stdexcept>

QmitkDataTreeComboBox::QmitkDataTreeComboBox(QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL)
{
  initialize();
}

QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeFilter* filter,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(filter)
{
  initialize();
  SetFilter(filter);
}

QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeBase* tree,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL)
{
  initialize();
  SetDataTree(tree);
}

QmitkDataTreeComboBox::QmitkDataTreeComboBox(mitk::DataTreeIteratorBase* iterator,QWidget* parent, const char* name)
: QComboBox(parent, name),
  m_DataTreeFilter(NULL)
{
  initialize();
  SetDataTree(iterator);
}

void QmitkDataTreeComboBox::initialize()
{
  // member initializations that are equal for all constructors
  m_SkipItem = NULL;
  m_SkipItemParent = NULL;
  m_SelfCall = false;
  m_DisplayedProperty.clear();
  m_UserSetDisplayedProperty.clear();

  connect(this, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}

QmitkDataTreeComboBox::~QmitkDataTreeComboBox()
{
  disconnectNotifications();
}

void QmitkDataTreeComboBox::connectNotifications()
{
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

void QmitkDataTreeComboBox::disconnectNotifications()
{
  m_DataTreeFilter->RemoveObserver( m_RemoveItemConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveChildrenConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveAllConnection );
  m_DataTreeFilter->RemoveObserver( m_SelectionChangedConnection );
  m_DataTreeFilter->RemoveObserver( m_ItemAddedConnection );
  m_DataTreeFilter->RemoveObserver( m_UpdateAllConnection );
} 

void QmitkDataTreeComboBox::SetDataTree(mitk::DataTreeBase* tree)
{
  if (tree)
  {
    // create default filter with visibility (editable) and name (non-editable)
    m_PrivateFilter = mitk::DataTreeFilter::New(tree);
    m_PrivateFilter->SetFilter(&mitk::IsGoodDataTreeNode);
    mitk::DataTreeFilter::PropertyList visible;
    visible.push_back("name");
    m_PrivateFilter->SetVisibleProperties(visible);
    
    m_DataTreeFilter = m_PrivateFilter;
  
    SetDisplayedProperty("name");
    generateItems();
  }
}

void QmitkDataTreeComboBox::SetDataTree(mitk::DataTreeIteratorBase* iterator)
{
  // create default filter if neccessary
  if (iterator && iterator->GetTree())
    SetDataTree(iterator->GetTree());
}

void QmitkDataTreeComboBox::SetFilter(mitk::DataTreeFilter* filter)
{
  if (m_DataTreeFilter) // remove observers
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

void QmitkDataTreeComboBox::onActivated(int index)
{
  // find item, generate signal
  try
  {
    // following cast is due to bad design... class Item needs needs rework 
    m_SelfCall = true;
    const_cast<mitk::DataTreeFilter::Item*>(m_Items.at(index))->SetSelected(true);
    emit activated(m_Items.at(index));
    m_SelfCall = false;
  }
  catch (std::out_of_range)
  {
    std::cerr << "in onActivated(" << index << "), " << __FILE__ << "l. " << __LINE__ << ": not a good index..." << std::endl;
  }
  
  
  // TODO on selectionChanged notifications from treefilter => change selection!
}

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

void QmitkDataTreeComboBox::SetDisplayedProperty(std::string prop)
{
  m_UserSetDisplayedProperty = prop;
  determineDisplayedProperty();
}

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
    QString displayedText( itemiter->GetProperty(m_DisplayedProperty) );

    if ( m_DataTreeFilter->GetHierarchyHandling() != mitk::DataTreeFilter::FLATTEN_HIERARCHY )
    {
      displayedText = QString(" ") + displayedText;
      for (int i = 0; i < level; ++i)
        displayedText = QString("-") + displayedText;
    }
     
    m_Items.push_back(*itemiter);
    QComboBox::insertItem(displayedText);
 
    if ( itemiter->HasChildren() )
    {
      // add children, unless this item is the parent whose children were just deleted
      if (*itemiter != m_SkipItemParent)
        AddItemsToList(itemiter->GetChildren(), visibleProps, level+1);
    }
    
    ++itemiter; 
  }
}

void QmitkDataTreeComboBox::clearItems()
{
  QComboBox::clear();
  m_Items.clear();
} 

void QmitkDataTreeComboBox::generateItems()
{
  if (!m_DataTreeFilter) return;

  clearItems();
  
  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );

  // fill rows with property views for the visible items 
  AddItemsToList(m_DataTreeFilter->GetItems(), visibleProps, 0);
}

void QmitkDataTreeComboBox::removeItemHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveItemEvent& event( static_cast<const mitk::TreeFilterRemoveItemEvent&>(e) );
  m_SkipItem = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
  m_SkipItem = NULL;
}

void QmitkDataTreeComboBox::removeChildrenHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveChildrenEvent& event( static_cast<const mitk::TreeFilterRemoveChildrenEvent&>(e) );
  // TODO: do something more clever
  m_SkipItemParent = event.GetChangedItem();
  generateItems();
  m_SkipItemParent = NULL;
}

void QmitkDataTreeComboBox::removeAllHandler( const itk::EventObject& )
{
  clearItems();
}

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
      if ( m_Items.at(row) == item )
      {
        QComboBox::setCurrentItem(row);
        emit activated(m_Items.at(row));
        break;
      }
  }
  catch (std::out_of_range)
  {
    std::cerr << "in selectionChangedHandler(" << row << "), " << __FILE__ << " l. " << __LINE__ << ": not a good index..." << std::endl;
  }

}

void QmitkDataTreeComboBox::itemAddedHandler( const itk::EventObject& /*e*/ )
{
  //const mitk::TreeFilterItemAddedEvent& event( static_cast<const TreeFilterItemAddedEvent&>(e) );
  //mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
}

void QmitkDataTreeComboBox::updateAllHandler( const itk::EventObject& )
{
  determineDisplayedProperty();
  generateItems();
}

