/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <QmitkDataTreeListView.h>
#include <QmitkPropertyViewFactory.h>

#include <itkCommand.h>
#include <mitkDataTreeFilterEvents.h>
#include <mitkDataTreeFilterFunctions.h>
#include <mitkRenderingManager.h>

#include <qlayout.h>
#include <qpainter.h>
#include <qobjectlist.h>

#include <stdexcept>

/**
  Initializes QmitkDataTreeListView from nothing. Results in an empty widget. Call SetDataTree or GetFilter later to fill widget with items.

  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeListView::QmitkDataTreeListView(QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL),
  m_PrivateFilter(NULL)
{
  initialize();
}

/**
  Initializes DataTreeListView from a mitk::DataTreeFilter. 

  \param filter pointer to the mitk::DataTreeFilter to be displayed
  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeFilter* filter, QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(filter),
  m_PrivateFilter(NULL)
{
  initialize();
  SetFilter(filter);
}

/**
  Initializes DataTreeListView from a mitk::DataTreeBase. Will create a private mitk::DataTreeFilter, containing only the "name" and "visible" properties of anything

  \param tree pointer to the mitk::DataTreeBase to be displayed (after creating a private filter)
  \param parent Qt widget that is parent
  \param name Qt name
*/
QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeBase* tree, QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL),
  m_PrivateFilter(NULL)
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
QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTreeIteratorBase* iterator, QWidget* parent, const char* name)
: QWidget(parent, name),
  QmitkListViewItemIndex(),
  m_DataTreeFilter(NULL),
  m_PrivateFilter(NULL)
{
  initialize();
  SetDataTree(iterator);
}

/**
  This is called by all constructors. Initializes some members.
*/
void QmitkDataTreeListView::initialize()
{
  // member initializations that are equal for all constructors
  m_SkipItem = NULL;
  m_SkipItemParent = NULL;
  m_StretchedColumn = -1; 
  m_SelfCall = false;
  m_AutoUpdate = true;
  
  m_RemoveItemConnection       = 63535;
  m_RemoveChildrenConnection   = 63535;
  m_RemoveAllConnection        = 63535;
  m_SelectionChangedConnection = 63535;
  m_ItemAddedConnection        = 63535;
  m_UpdateAllConnection        = 63535;
  
  setBackgroundMode( Qt::PaletteBase );
  setFocusPolicy( QWidget::StrongFocus ); // to receive keyboard events
}

/**
    Minor cleanup. Disconnect all itk-observers. If a private filter exists, it is destructed due to the smart pointer.
*/
QmitkDataTreeListView::~QmitkDataTreeListView()
{
  disconnectNotifications();
}

/**
  Creates new itk-observers for the notifications of mitk::DataTreeFilter. Connects them to ...Handler(itk::EventObject) functions.
*/
void QmitkDataTreeListView::connectNotifications()
{
  if (!m_DataTreeFilter) return;
  
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
  {
  itk::ReceptorMemberCommand<QmitkDataTreeListView>::Pointer command7 = itk::ReceptorMemberCommand<QmitkDataTreeListView>::New();
  command7->SetCallbackFunction(this, &QmitkDataTreeListView::newItemHandler);
  m_NewItemConnection = m_DataTreeFilter->AddObserver(mitk::TreeFilterNewItemEvent(), command7);
  }
}

/**
  removes all itk-Observers
*/
void QmitkDataTreeListView::disconnectNotifications()
{
  if (!m_DataTreeFilter) return;
  
  // remove observers
  m_DataTreeFilter->RemoveObserver( m_RemoveItemConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveChildrenConnection );
  m_DataTreeFilter->RemoveObserver( m_RemoveAllConnection );
  m_DataTreeFilter->RemoveObserver( m_SelectionChangedConnection );
  m_DataTreeFilter->RemoveObserver( m_ItemAddedConnection );
  m_DataTreeFilter->RemoveObserver( m_UpdateAllConnection );
  m_DataTreeFilter->RemoveObserver( m_NewItemConnection );
} 

/**
  User provides only a data tree and does not tell, what should be displayed. So this method creates a private
  mitk::DataTreeFilter, determines "visible" to be editable and "name" to be visible only.
  Then the pointer m_DataTreeFilter is set to this private filter.

  \param tree Tree to display
*/
void QmitkDataTreeListView::SetDataTree(mitk::DataTreeBase* tree)
{
  if (tree)
  {
    // create default filter with visibility (editable) and name (non-editable)
    m_PrivateFilter = mitk::DataTreeFilter::New(tree);
    m_PrivateFilter->SetAutoUpdate( m_AutoUpdate ); 
    m_PrivateFilter->SetFilter(mitk::IsGoodDataTreeNode());
    mitk::DataTreeFilter::PropertyList visible;
    visible.push_back("visible");
    visible.push_back("name");
    m_PrivateFilter->SetVisibleProperties(visible);
    mitk::DataTreeFilter::PropertyList editable;
    editable.push_back("visible");
    m_PrivateFilter->SetEditableProperties(editable);
    
    disconnectNotifications(); 
    m_DataTreeFilter = m_PrivateFilter;
    connectNotifications(); // add observers
    generateItems();
  }
  else
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
void QmitkDataTreeListView::SetDataTree(mitk::DataTreeIteratorBase* iterator)
{
  // create default filter if neccessary
  if (iterator && iterator->GetTree())
    SetDataTree(iterator->GetTree());
  else
  {
    disconnectNotifications(); 
    m_DataTreeFilter = NULL;
    m_PrivateFilter = NULL;
    generateItems();
  }
}

/**
  Display items from the mitk::DataTreeFilter provided in \a filter
  
  \param filter pointer to the mitk::DataTreeFilter to be displayed
*/
void QmitkDataTreeListView::SetFilter(mitk::DataTreeFilter* filter)
{
  disconnectNotifications();
  m_DataTreeFilter = filter;
  // in the case that somebody first sets a datatree and then a filter
  // destroy the default filter that was created in SetDataTree
  if (filter != m_PrivateFilter.GetPointer())
    m_PrivateFilter = NULL;
  connectNotifications(); // add observers
  generateItems();
}

/**
  Get the currently active mitk::DataTreeFilter. May either be user provided or created privatly for some data tree.
  
  \return active mitk::DataTreeFilter
*/
mitk::DataTreeFilter* QmitkDataTreeListView::GetFilter()
{
  return m_DataTreeFilter;
}

/**
  Define what kind of widget should be created to display or edit the given property.

  E.g. call SetViewType("name", QmitkPropertyViewFactory::etON_DEMAND_EDIT) to display the "name" property
  in QLabels which switch to input fields on demand (will only work, when "name" is included in the DataTreeFilter's 
  list of editable properties - see SetEditableProperties()).
*/
void QmitkDataTreeListView::SetViewType(const std::string& property, unsigned int type)
{
  if ( property.length() > 0 )
    m_ViewTypes[property] = type;
}

/**
  Get the column number that is currently stretched, when free display space has to be filled.
*/
int QmitkDataTreeListView::stretchedColumn()
{
  return m_StretchedColumn;
}

/**
  This widget uses a Qt GridLayout to arrange its widgets. Since chances are that there is more space on the screen 
  than all widgets would need, the user can determine which column to stretched, when Qt asks this widget to fill some more
  space. Selecting a column with QLabels in it is usually a good idea.

  \param col index of the column to be streched. Counts from 0.
*/
void QmitkDataTreeListView::setStretchedColumn(int col)
{
  m_StretchedColumn = col;
  generateItems();
}

/**
  Qt uses this method to determine how much display space is reasonable for this widget.

  \return QSize as determined in GenerateItems.
*/
QSize QmitkDataTreeListView::sizeHint() const
{
  return m_SizeHint;
}

/**
  Called from paintEvent() to draw the highlights behind selected rows.
  This method iterates over the rows of all GridLayouts that are used internally and
  each time asks the associated items, whether they are selected or not. The background color is determined
  from the items' answers.

  Method is invoked recursively for nested items.

  \param painter QPainter objects, which is used to issue paint requests
  \param index indexing object which holds information about the GridLayouts and associated items
*/
void QmitkDataTreeListView::paintListBackground(QPainter& painter, QmitkListViewItemIndex* index)
{
  if (index && index->m_Grid)
  {
    for (int row = 1; row < index->m_Grid->numRows(); ++row)
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
          painter.fillRect(cell.left() - 4, cell.top()-2, width(), cell.height()+4, colorGroup().brush(QColorGroup::Highlight) );
        else
          painter.fillRect(cell.left() - 4, cell.top()-2, width(), cell.height()+4, colorGroup().brush(QColorGroup::Base) );
      }
    }
  }
}

/**
  Creates a QPainter and initiates painting of the highlights for selected rows.
*/
void QmitkDataTreeListView::paintEvent(QPaintEvent*)
{
  if (m_Grid) 
  {
    m_Grid->activate(); // after clearItems there is no m_Grid
    m_SizeHint = m_Grid->sizeHint();
  }
  QPainter painter(this);
  paintListBackground(painter,this);
}

/**
  This handles item selection by the user. Currently selections can only be made using the mouse.
  Using the keyboard would be a nice feature (although not too urgent, because the mouse is used in most applications).

  Asks the contained index objects about the row under the mouse cursor, determines the associated item,
  then toggles selection.
  
  \param e Qt event information. Contains mouse position and mouse button states.
*/
void QmitkDataTreeListView::mouseReleaseEvent ( QMouseEvent* e )
{
  // determine row
  // toggle selection status of item under cursor
  // tell row's widgets/expander to set their background according to their associated item's selection status
  // 
  // initiate paintEvent

  if (m_Grid) 
  {
    m_Grid->activate(); // don't know why this has to be here, but it fixes a selection problem. It SHOULD do calling this in generateItems(), but it doesn't...
    m_SizeHint = m_Grid->sizeHint();
  }

  QmitkListViewItemIndex* index(this);
  int row(-1);
  
  row = index->rowAt( e->y() );
  if ( row <= 0 ) return; // no row under cursor

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
      item->SetSelected( selected ); // toggle selection
      emit clicked(item, selected);
    }
  }
  else // not left mouse button
  {
    // future enhancement. signal with item as parameter -> opportunity to generate a popup menu
  }

  setFocus(); // this focus things is somehow strange
}

/**
  When the DELETE button is pressed, the currently selected items are deleted (after confirmation).
*/
void QmitkDataTreeListView::keyReleaseEvent(QKeyEvent* e)
{
  if ( e->key() == Qt::Key_Delete )
  {
    m_DataTreeFilter->DeleteSelectedItemsAndSubItems();
    e->accept();
  }
  else
  {
    e->ignore();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(true);
}

/**
  Most complicated method of this widget. Purpose is to create widgets and layouts from
  the item list that is provided by a mitk::DataTreeFilter.

  \param parent Qt parent of all newly created widgets
  \param items The top level item list (items may have children, then this method calls itself recursively)
  \param visibleProps Properties for which display widgets should be created
  \param editableProps Properties for which editable widgets should be created
*/
void QmitkDataTreeListView::AddItemsToList(QWidget* parent, QmitkListViewItemIndex* index,
                                           const mitk::DataTreeFilter::ItemList* items,
                                           const mitk::DataTreeFilter::PropertyList& visibleProps,
                                           const mitk::DataTreeFilter::PropertyList& editableProps)
{
  index->m_Grid->setSpacing(4);
  index->m_Grid->setColStretch(m_StretchedColumn, 10);
  
  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 
  
  int row(1); // headings are row 0
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
        
      unsigned int viewType = m_ViewTypes[*nameiter];
        
      // editable?
      if ( std::find( editableProps.begin(), editableProps.end(), *nameiter ) != editableProps.end() )
      {
        
        // create editor
        try 
        {
          observerWidget = QmitkPropertyViewFactory::GetInstance()->CreateEditor( itemiter->GetProperty(*nameiter), 
                                                                                  viewType,
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
                                                                               viewType,
                                                                               parent);
      }

      if (observerWidget)
      { // widget ready, now add it
        observerWidget->show();
        if (itemiter->IsSelected())
        {
          observerWidget->setBackgroundMode(Qt::PaletteHighlight);
       
          // following lines seem too complicated, but I could not find an easier way of changing the foreground color
          QPalette palette( observerWidget->palette() );
          palette.setColor( QColorGroup::Foreground, palette.color( QPalette::Active, QColorGroup::HighlightedText ) );
          observerWidget->setPalette( palette );
        }
        else
        {
          observerWidget->setBackgroundMode( Qt::PaletteBase );
        }
        index->addWidget(observerWidget, row, column, Qt::AlignVCenter);
      }

      ++column;
    }
 
    index->addItem(const_cast<mitk::DataTreeFilter::Item*>((*itemiter).GetPointer()), row);
    
    if ( itemiter->HasChildren() )
    {
      // add expansion symbol
      QGridLayout* childrenGridLayout = new QGridLayout( 0, 1, visibleProps.size()+1 ); // 1 extra for expansion symbol
      QmitkListViewExpanderIcon* childExpander = new QmitkListViewExpanderIcon(childrenGridLayout, index, parent);
      childExpander->show();
      childExpander->setBackgroundMode( Qt::PaletteBase );
      
      index->m_Grid->addMultiCellLayout( childrenGridLayout, row+1, row+1, 1, visibleProps.size(), Qt::AlignVCenter );
      
      index->addWidget(childExpander, row, 0, Qt::AlignVCenter); 
      index->addIndex(childExpander, row); 
     
      // add children, unless this item is the parent whose children were just deleted
      if (*itemiter != m_SkipItemParent)
        AddItemsToList(parent, childExpander, itemiter->GetChildren(), visibleProps, editableProps);
     
      ++row;
    }
    else
    {
      // to get some indent for child elements
      QLabel* label = new QLabel(" ", parent);
      label->show();
      label->setBackgroundMode( Qt::PaletteBase );
      index->addWidget(label, row, 0, Qt::AlignVCenter); 
    }
    
    ++row;
    
    ++itemiter; 
  }

}

/**
  Deletes all child widgets. Then deletes all index information.
*/
void QmitkDataTreeListView::clearItems()
{
  // delete all existing children
  while ( !queryList()->isEmpty() )
  {
    delete queryList()->first();
  }
    
  m_Grid = NULL;

  // delete references to now deleted children
  clearIndex();
} 

/**
  Regenerates all widgets.

  <ol>
   <li> Delete all items and index information
   <li> Get lists of visible and editable properties from the mitk::DataTreeFilter
   <li> Create a new top level GridLayout and
   <li> Call AddItemsToList() to populate the GridLayout with observer widgets
   <li> Finally, add an empty row to fill space at the bottom of this widget
  </ol>
*/
void QmitkDataTreeListView::generateItems()
{
  if (!m_DataTreeFilter) return;
  QWidget::setUpdatesEnabled(false);

  clearItems();
  
  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );
  const mitk::DataTreeFilter::PropertyList& editableProps( m_DataTreeFilter->GetEditableProperties() );

  // create a new layout grid
  m_Grid = new QGridLayout( this, 2, visibleProps.size()+1 ); // 1 extra for expansion symbol

  // fill column headings
  QLabel* label = new QLabel("", this);
  label->show();
  label->setFrameStyle( QFrame::Panel | QFrame::Raised );
  label->setBackgroundMode( Qt::PaletteButton );
  QmitkListViewItemIndex::addWidget(label, 0, 0, Qt::AlignVCenter); 
  int column(1); // column zero is reserved for expander icons and empty labels (see above)
  const mitk::DataTreeFilter::PropertyList* labels( &(m_DataTreeFilter->GetPropertiesLabels()) );
  // if there are no lables, then use the property keys
  if ( labels->empty() )
    labels = &(m_DataTreeFilter->GetVisibleProperties());

  for( mitk::DataTreeFilter::PropertyList::const_iterator labeliter = labels->begin();
       labeliter != labels->end();
       ++labeliter, ++column )
  {
    QLabel* label = new QLabel( labeliter->c_str(), this);
    label->show();
    label->setFrameStyle( QFrame::Panel | QFrame::Raised );
    label->setTextFormat( Qt::RichText );
    label->setBackgroundMode( Qt::PaletteButton );
    QmitkListViewItemIndex::addWidget(label, 0, column, Qt::AlignVCenter); 
  }
  //m_Grid->setMargin(2);
  
  if (m_StretchedColumn == -1)
    m_StretchedColumn = visibleProps.size();
 
  // fill rows with property views for the visible items 
  if (m_SkipItemParent != (mitk::DataTreeFilter::Item*) -1)
    AddItemsToList(this, this, m_DataTreeFilter->GetItems(), visibleProps, editableProps);
  else
  {
    m_Grid->setSpacing(4);
    m_Grid->setColStretch(m_StretchedColumn, 10);
  }
   
  m_SizeHint = m_Grid->sizeHint();
  m_Grid->addItem( new QSpacerItem(1, 5, QSizePolicy::Minimum, QSizePolicy::Ignored) , m_Grid->numRows(),0);
  m_Grid->setRowStretch( m_Grid->numRows()-1, 1 );
  
  QWidget::setUpdatesEnabled(true);
  repaint();
}

/**
  Handles TreeFilterRemoveItemEvents from the DataTreeFilter. For this purpose a variable m_SkipItem is set to contain the
  item, which will shortly be deleted. This is necessary because the notification is sent while the item still exists.
  m_SkipItem is considered in generateItem().
  Perhaps something more sophisticated could be implemented.
*/
void QmitkDataTreeListView::removeItemHandler( const itk::EventObject& e )
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
void QmitkDataTreeListView::removeChildrenHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterRemoveChildrenEvent& event( static_cast<const mitk::TreeFilterRemoveChildrenEvent&>(e) );
  // TODO: do something more clever (still TODO?)
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
void QmitkDataTreeListView::removeAllHandler( const itk::EventObject& )
{
  clearItems();
}

/**
  Recursively looks for an item in an index. When found, the displayed selection status of that item is changed to selected.
  This method is called from the selectionChangedHandler on notifications from the data tree filter.

  \param item The item to (un)highlight
  \param selected Selection status that should be displayed
  \param index The index information, where to browse for \a item
*/
bool QmitkDataTreeListView::selectItemInGrid(const mitk::DataTreeFilter::Item* item, bool selected, QmitkListViewItemIndex* index)
{
  // find item, change selection, done
  //std::vector<QmitkListViewItemIndex*>::iterator iterindices = index->m_Indices.begin();
  for (int row = 1; row < index->m_Grid->numRows(); ++row)
  {
    // if this is the item, (un)select it
    // then break, return true
    if ( index->itemAt(row) == item )
    {
      // locking issues
      QmitkListViewItemIndex* temp(index);
      temp->lockBecauseOfSelection( selected );
      while ((temp = temp->parentIndex()))
      {
        temp->lockBecauseOfSelection( selected );
      }
      
      QRect cell( index->m_Grid->cellGeometry(row, 1) );
     
      QPainter painter(this);
      if (selected)
        painter.fillRect(cell.left() - 4, cell.top()-2, width(), cell.height()+4, colorGroup().brush(QColorGroup::Highlight) );
      else
        painter.fillRect(cell.left() - 4, cell.top()-2, width(), cell.height()+4, colorGroup().brush(QColorGroup::Base) );
      // do the appropriate magic on all widgets
      std::list<QWidget*>::reverse_iterator iter;
      for ( iter = index->widgetsAt(row).rbegin(), ++iter; iter != index->widgetsAt(row).rend(); ++iter) // ignore last item (expander symbol)
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
      
      return true;
    }
    
    // if the current item has children, recurse, break on positive return value
    try
    {
      //if (iterindices != index->m_Indices.end() && *iterindices != NULL)
      QmitkListViewItemIndex* ind(index->indexAt(row));
      if (ind)
      {
        if ( selectItemInGrid(item, selected, ind) )
        {
          return true;
        }
      }
    }
    catch (std::out_of_range)
    {
    }

  }

  // by default return false 
  return false;
}

/**
  Handles TreeFilterSelectionChangedEvents from the DataTreeFilter. The actual work is done in selectItemInGrid, which recursively
  looks for the item to (de)select and then does as requested.
*/
void QmitkDataTreeListView::selectionChangedHandler( const itk::EventObject& e)
{
  const mitk::TreeFilterSelectionChangedEvent& event( static_cast<const mitk::TreeFilterSelectionChangedEvent&>(e) );
  const mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  bool selected = event.IsSelected();

  emit activated(item, selected);
 
  selectItemInGrid(item, selected, this);
  update();
}

/**
  Handles TreeFilterItemAddedEvents from the DataTreeFilter. Implemented by simply regenerating everything.
  Perhaps something more sophisticated could be implemented.
*/
void QmitkDataTreeListView::itemAddedHandler( const itk::EventObject& /*e*/ )
{
  //const mitk::TreeFilterItemAddedEvent& event( static_cast<const TreeFilterItemAddedEvent&>(e) );
  //mitk::DataTreeFilter::Item* item = event.GetChangedItem();
  // TODO: do something more clever
  generateItems();
}


/**
  Handles TreeFilterNewItemEvents from the DataTreeFilter. Will just forward a notification to any Qt observers.
*/
void QmitkDataTreeListView::newItemHandler( const itk::EventObject& e )
{
  const mitk::TreeFilterItemAddedEvent& event( static_cast<const mitk::TreeFilterItemAddedEvent&>(e) );
  const mitk::DataTreeFilter::Item* item = event.GetChangedItem();

  if (item)
    emit newItem( item );
}

/**
  Handles TreeFilterUpdateIncludingVtkActorsEvents from the DataTreeFilter. 
*/
void QmitkDataTreeListView::updateAllHandler( const itk::EventObject& )
{
  generateItems();
}

void QmitkDataTreeListView::Update()
{
  if (m_DataTreeFilter)
    m_DataTreeFilter->Update();
}

void QmitkDataTreeListView::SetAutoUpdate(bool autoUpdatesEnabled)
{
  if ( autoUpdatesEnabled != m_AutoUpdate )
  {
    if (m_DataTreeFilter) m_DataTreeFilter->SetAutoUpdate(autoUpdatesEnabled);

    m_AutoUpdate = autoUpdatesEnabled;
  }
}



