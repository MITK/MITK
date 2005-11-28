#include <QmitkDataTreeListView.h>
#include <QmitkPropertyViewFactory.h>

#include <mitkDataTreeFilter.h>

QmitkDataTreeListView::QmitkDataTreeListView(QWidget* parent, const char* name)
: QListView(parent, name),
  m_DataTreeFilter(NULL)
{
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeFilter* filter,QWidget* parent, const char* name)
: QListView(parent, name),
  m_DataTreeFilter(filter)
{
  SetFilter(filter);
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTree* tree,QWidget* parent, const char* name)
: QListView(parent, name),
  m_DataTreeFilter(NULL)
{
  SetDataTree(tree);
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeIteratorBase* iterator,QWidget* parent, const char* name)
: QListView(parent, name),
  m_DataTreeFilter(NULL)
{
  SetDataTree(iterator);
}

QmitkDataTreeListView::~QmitkDataTreeListView()
{
}

void QmitkDataTreeListView::SetDataTree(mitk::DataTree* tree)
{
  // create default filter if neccessary
}

void QmitkDataTreeListView::SetDataTree(mitk::DataTreeIteratorBase*)
{
  // create default filter if neccessary
}

void QmitkDataTreeListView::SetFilter(mitk::DataTreeFilter* filter)
{
  m_DataTreeFilter = filter;
  GenerateItems();
}
    
void QmitkDataTreeListView::GenerateItems()
{
  if (!m_DataTreeFilter) return;

std::cout << "GenerateItems()" << std::endl;
  while ( columns() ) removeColumn(0); // remove everything from the list. QListView will call delete!
  
  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );
  const mitk::DataTreeFilter::PropertyList& editableProps( m_DataTreeFilter->GetEditableProperties() );

  // create columns
  for( mitk::DataTreeFilter::PropertyList::const_iterator nameiter = visibleProps.begin();
       nameiter != visibleProps.end();
       ++nameiter )
  {
    addColumn( nameiter->c_str() );
  }
std::cout << "Added Columns()" << std::endl;
 
  // fill rows with property views for the visible items 
  mitk::DataTreeFilter::ConstItemIterator itemiter( m_DataTreeFilter->GetItems()->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( m_DataTreeFilter->GetItems()->End() ); 
      
std::cout << "Create Items()" << std::endl;
  while ( itemiter != itemiterend ) // for all items
  {
    const mitk::BaseProperty* prop = itemiter->GetProperty("name");

    std::cout << "One Item" << std::endl;
  
    // show all visible properties
    for( mitk::DataTreeFilter::PropertyList::const_iterator nameiter = visibleProps.begin();
         nameiter != visibleProps.end();
         ++nameiter )
    {
      QWidget* observerWidget(NULL);
        
      // editable?
      if ( std::find( editableProps.begin(), editableProps.end(), *nameiter ) != editableProps.end() )
      {
        // create editor
std::cout << "create editor" << std::endl;
        try 
        {
          QmitkPropertyViewFactory::GetInstance()->CreateEditor( itemiter->GetProperty(*nameiter), QmitkPropertyViewFactory::vtDEFAULT, this);
        }
        catch ( mitk::DataTreeFilter::NoPermissionException& e )
        {
          std::cerr << "Some error in mitk::DataTreeFilter: Filter object reports " << *nameiter << " to be editable, but "
                       "trying to access the non-const version yields exceptions..." << std::endl;
        }
std::cout << "created editor" << std::endl;
      }
      else
      {
        // create (read-only) view
std::cout << "create view" << std::endl;
        QmitkPropertyViewFactory::GetInstance()->CreateView( itemiter->GetProperty(*nameiter), QmitkPropertyViewFactory::etDEFAULT, this);
std::cout << "created view" << std::endl;
      }
      
    }
    
    ++itemiter; // next item
  }
}


