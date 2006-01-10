#include <QmitkDataTreeListView.h>
#include <QmitkPropertyViewFactory.h>

#include <itkCommand.h>
#include <mitkDataTreeFilterEvents.h>

#include <qlayout.h>
#include <qpainter.h>
#include <qobjectlist.h>

#include <stdexcept>

QmitkDataTreeListView::QmitkDataTreeListView(QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL)
{
  initialize();
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeFilter* filter,QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(filter)
{
  initialize();
  SetFilter(filter);
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeBase* tree,QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL)
{
  initialize();
  SetDataTree(tree);
}

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeIteratorBase* iterator,QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL)
{
  initialize();
  SetDataTree(iterator);
}

void QmitkDataTreeListView::initialize()
{
  // member initializations that are equal for all constructors
  m_SkipItem = NULL;
  m_SkipItemParent = NULL;
  m_StretchedColumn = 2;
  m_SelfCall = false;
  setBackgroundMode( Qt::PaletteBase );

}

QmitkDataTreeListView::~QmitkDataTreeListView()
{
  disconnectNotifications();
}

void QmitkDataTreeListView::connectNotifications()
{
  // connect to our filter's notifications
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command1 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command1->SetCallbackFunction(this, &QmitkDataTreeListView::removeItemHandler);
  m_RemoveItemConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveItemEvent(), command1);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command2= itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command2->SetCallbackFunction(this, &QmitkDataTreeListView::removeChildrenHandler);
  m_RemoveChildrenConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveChildrenEvent(), command2);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command3 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command3->SetCallbackFunction(this, &QmitkDataTreeListView::removeAllHandler);
  m_RemoveAllConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterRemoveAllEvent(), command3);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command4 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command4->SetCallbackFunction(this, &QmitkDataTreeListView::selectionChangedHandler);
  m_SelectionChangedConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterSelectionChangedEvent(), command4);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command5 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command5->SetCallbackFunction(this, &QmitkDataTreeListView::itemAddedHandler);
  m_ItemAddedConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterItemAddedEvent(), command5);
  }
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command6 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command6->SetCallbackFunction(this, &QmitkDataTreeListView::updateAllHandler);
  m_UpdateAllConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterUpdateAllEvent(), command6);
  }
}

void QmitkDataTreeListView::disconnectNotifications()
{
  m_DataTreeFilter->RemoveObserver( m_RemoveItemConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveChildrenConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveAllConnection );
  m_DataTreeFilter->RemoveObserver( m_SelectionChangedConnection );
  m_DataTreeFilter->RemoveObserver( m_ItemAddedConnection );
  m_DataTreeFilter->RemoveObserver( m_UpdateAllConnection );
} 

void QmitkDataTreeListView::SetDataTree(mitk::DataTreeBase* tree)
{
  if (tree)
  {
    // create default filter with visibility (editable) and name (non-editable)
    m_PrivateFilter = mitk::DataTreeFilter::New(tree);
    m_PrivateFilter->SetFilter(&mitk::IsGoodDataTreeNode);
    mitk::DataTreeFilter::PropertyList visible;
    visible.push_back("visible");
    visible.push_back("name");
    m_PrivateFilter->SetVisibleProperties(visible);
    mitk::DataTreeFilter::PropertyList editable;
    editable.push_back("visible");
    m_PrivateFilter->SetEditableProperties(editable);
    
    m_DataTreeFilter = m_PrivateFilter;
    generateItems();
  }
}

void QmitkDataTreeListView::SetDataTree(mitk::DataTreeIteratorBase* iterator)
{
  // create default filter if neccessary
  if (iterator && iterator->GetTree())
    SetDataTree(iterator->GetTree());
}

void QmitkDataTreeListView::SetFilter(mitk::DataTreeFilter* filter)
{
  if (m_DataTreeFilter) // remove observers
    disconnectNotifications();
  m_DataTreeFilter = filter;
  // in the case that somebody first sets a datatree and then a filter
  // destroy the default filter that was created in SetDataTree
  if (filter != m_PrivateFilter.GetPointer())
    m_PrivateFilter = NULL;
  connectNotifications(); // add observers
  generateItems();
}

int QmitkDataTreeListView::stretchedColumn()
{
  return m_StretchedColumn;
}

void QmitkDataTreeListView::setStretchedColumn(int col)
{
  m_StretchedColumn = col;
  generateItems();
}

QSize QmitkDataTreeListView::sizeHint() const
{
  return m_SizeHint;
}

void QmitkDataTreeListView::paintListBackground(QPainter& painter, QmitkListViewItemIndex* index)
{
  if (index && index->m_Grid)
  {
    for (int row = 0; row < index->m_Grid->numRows(); ++row)
    {
      if ( QmitkListViewItemIndex* temp = index->indexAt(row-1) ) 
        paintListBackground(painter, temp);
      else
      {
        QRect cell( index->m_Grid->cellGeometry(row, 1) );
      
        bool selected(false);
        mitk::DataTreeFilter::Item* item = index->itemAt(row);
        if (item)
        {
          if ( item->IsSelected() )
            selected = true;
        }

        if (selected)
          painter.fillRect(cell.left() - 4, cell.top()-2, width(), cell.height()+4 , colorGroup().brush(QColorGroup::Highlight) );
        else
          painter.fillRect(cell.left() - 4, cell.top()-2 ,width(),cell.height()+4, colorGroup().brush(QColorGroup::Base) );
      }
    }
  }
}

void QmitkDataTreeListView::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  paintListBackground(painter,this);
}

void QmitkDataTreeListView::mouseReleaseEvent ( QMouseEvent* e )
{
  // determine row
  // toggle selection status of item under cursor
  // tell row's widgets/expander to set their background according to their associated item's selection status
  // 
  // initiate paintEvent
  
  QmitkListViewItemIndex* index(this);
  int row(-1);
 
  row = index->rowAt( e->y() );
  if ( row == -1 ) return; // no row under cursor

  // find item
  QmitkListViewItemIndex* temp(0);

  while ( (temp = index->indexAt(row-1)) )
  {
    index = temp;
    row = index->rowAt( e->y() );
  }

  mitk::DataTreeFilter::Item* item = index->itemAt(row);

  bool selected(false);

  if ( e->button() == Qt::LeftButton )
  {
  
    if (item)
    {
      selected = !item->IsSelected();
      m_SelfCall = true;
      item->SetSelected( selected ); // toggle selection
      m_SelfCall = false;
    }
 
    temp = index;
    temp->lockBecauseOfSelection( selected );
    while ((temp = temp->parentIndex()))
    {
      temp->lockBecauseOfSelection( selected );
    }
  
    try
    {
      // set clicked row's widget backgrounds according to the selection status
      std::list<QWidget*>& widgets( index->widgetsAt(row) );
  
    std::list<QWidget*>::reverse_iterator iter;
    for ( iter = widgets.rbegin(), ++iter; iter != widgets.rend(); ++iter) // ignore last item (expander symbol)
    {
      if (selected)
      {
        (*iter)->setBackgroundMode(Qt::PaletteHighlight);
      
        // following lines seem too complicated, but I could not find an easier way of changing the foreground color
        QPalette palette( (*iter)->palette() );
        palette.setColor( QColorGroup::Foreground, palette.color( QPalette::Active, QColorGroup::HighlightedText ) );
        (*iter)->setPalette( palette );
      }
      else
      {
        (*iter)->setBackgroundMode( Qt::PaletteBase );
        (*iter)->unsetPalette();
      }
    }

    }
    catch (std::out_of_range)
    {
      // no recovery
    }
  }
  else // not left mouse button
  {
    // future enhancement. signal with item as parameter -> opportunity to generate a popup menu
  }

  update();
}

void QmitkDataTreeListView::AddItemsToList(QWidget* parent, QmitkListViewItemIndex* index,
                                           const mitk::DataTreeFilter::ItemList* items,
                                           const mitk::DataTreeFilter::PropertyList& visibleProps,
                                           const mitk::DataTreeFilter::PropertyList editableProps)
{
  index->m_Grid->setSpacing(4);
  
  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 
  
  int row(0);
  while ( itemiter != itemiterend ) // for all items
  {

    if (*itemiter == m_SkipItem)
    {
      // skip the item
      ++itemiter;
      continue;
    }
    
    // first column: reserved for expansion symbol
    int column(1);
    // following columns: all visible properties
    for( mitk::DataTreeFilter::PropertyList::const_iterator nameiter = visibleProps.begin();
         nameiter != visibleProps.end();
         ++nameiter )
    {
      QWidget* observerWidget(NULL);
        
      // editable?
      if ( std::find( editableProps.begin(), editableProps.end(), *nameiter ) != editableProps.end() )
      {
        // create editor
        try 
        {
          observerWidget = QmitkPropertyViewFactory::GetInstance()->CreateEditor( itemiter->GetProperty(*nameiter), 
                                                                                  QmitkPropertyViewFactory::etDEFAULT, 
                                                                                  parent);
        }
        catch ( mitk::DataTreeFilter::NoPermissionException& )
        {
          std::cerr << "Some error in mitk::DataTreeFilter: Filter object reports " << *nameiter << " to be editable, but "
                       "trying to access the non-const version yields exceptions..." << std::endl;
        }
      }
      else
      {
        // create (read-only) view
        observerWidget  = QmitkPropertyViewFactory::GetInstance()->CreateView( itemiter->GetProperty(*nameiter), 
                                                                               QmitkPropertyViewFactory::vtDEFAULT, 
                                                                               parent);
      }

      if (observerWidget)
      { // widget ready, now add it
        observerWidget->setBackgroundMode( Qt::PaletteBase );
        index->addWidget(observerWidget, row, column, Qt::AlignVCenter);
      }

      if (column == m_StretchedColumn)
        index->m_Grid->setColStretch(column, 10);
      
      ++column;
    }
 
    index->addItem(const_cast<mitk::DataTreeFilter::Item*>((*itemiter).GetPointer()), row);
    
    if ( itemiter->HasChildren() )
    {
      // add expansion symbol
      QGridLayout* childrenGridLayout = new QGridLayout( 0, 1, visibleProps.size()+1 ); // 1 extra for expansion symbol
      QmitkListViewExpanderIcon* childExpander = new QmitkListViewExpanderIcon(childrenGridLayout, index, parent);
      childExpander->setBackgroundMode( Qt::PaletteBase );
      
      index->addWidget(childExpander, row, 0, Qt::AlignVCenter); 
      //index->addMultiCellWidget(childExpander, row, row+1, 0, 0, Qt::AlignTop); 
                                        // fromRow, toRow, fromCol, toCol
      index->addIndex(childExpander, row); 
     
      index->m_Grid->addMultiCellLayout( childrenGridLayout, row+1, row+1, 1, visibleProps.size(), Qt::AlignVCenter );
      // add children, unless this item is the parent whose children were just deleted
      if (*itemiter != m_SkipItemParent)
        AddItemsToList(parent, childExpander, itemiter->GetChildren(), visibleProps, editableProps);
      
      ++row;
    }
    else
    {
      // to get some indent for child elements
      QLabel* label = new QLabel(" ", parent);
      label->setBackgroundMode( Qt::PaletteBase );
      index->addWidget(label, row, 0, Qt::AlignVCenter); 
    }
    
    ++row;

    ++itemiter; 
  }

}

void QmitkDataTreeListView::clearItems()
{
  // delete all existing children
  while ( !queryList()->isEmpty() )
    delete queryList()->first();
 
  // delete references to now deleted children
  clearIndex();
} 

void QmitkDataTreeListView::generateItems()
{
  if (!m_DataTreeFilter) return;

  clearItems();
  
  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );
  const mitk::DataTreeFilter::PropertyList& editableProps( m_DataTreeFilter->GetEditableProperties() );

  // create a new layout grid
  m_Grid = new QGridLayout( this, 1, visibleProps.size()+1 ); // 1 extra for expansion symbol
  //m_Grid->setMargin(2);
  
  if (m_StretchedColumn == -1)
    m_StretchedColumn = visibleProps.size();
 
  // fill rows with property views for the visible items 
  AddItemsToList(this, this, m_DataTreeFilter->GetItems(), visibleProps, editableProps);
  
  m_SizeHint = m_Grid->sizeHint();

  m_Grid->addItem( new QSpacerItem(1, 5, QSizePolicy::Minimum, QSizePolicy::Ignored) , m_Grid->numRows(),0);
  m_Grid->setRowStretch( m_Grid->numRows()-1, 1 );

}

void QmitkDataTreeListView::removeItemHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveItemEvent& event( static_cast<const mitk::TreeFilterRemoveItemEvent&>(e) );
  m_SkipItem = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
  m_SkipItem = NULL;
}

void QmitkDataTreeListView::removeChildrenHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveChildrenEvent& event( static_cast<const mitk::TreeFilterRemoveChildrenEvent&>(e) );
  // TODO: do something more clever
  m_SkipItemParent = event.GetChangedItem();
  generateItems();
  m_SkipItemParent = NULL;
}

void QmitkDataTreeListView::removeAllHandler( const itk::EventObject& )
{
  clearItems();
}

void QmitkDataTreeListView::selectionChangedHandler( const itk::EventObject& )
{
  if (m_SelfCall) return; // invoked by this object
  /*
  const mitk::TreeFilterSelectionChangedEvent& event( static_cast<const mitk::TreeFilterSelectionChangedEvent&>(e) );
  const mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  bool selected = event.IsSelected();
  */
  // find item, change selection, done
}

void QmitkDataTreeListView::itemAddedHandler( const itk::EventObject& /*e*/ )
{
  //const mitk::TreeFilterItemAddedEvent& event( static_cast<const TreeFilterItemAddedEvent&>(e) );
  //mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
}

void QmitkDataTreeListView::updateAllHandler( const itk::EventObject& )
{
  generateItems();
}





