#include <QmitkDataTreeListView.h>
#include <QmitkPropertyViewFactory.h>

#include <qlayout.h>
#include <qpainter.h>

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

QmitkDataTreeListView::QmitkDataTreeListView(mitk::DataTree* tree,QWidget* parent, const char* name)
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
  m_StretchedColumn = -1;
  setBackgroundMode( Qt::PaletteBase );
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

int QmitkDataTreeListView::stretchedColumn()
{
  return m_StretchedColumn;
}

void QmitkDataTreeListView::setStretchedColumn(int col)
{
  m_StretchedColumn = col;
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

void QmitkDataTreeListView::paintEvent(QPaintEvent* e)
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
  
  if (item)
  {
    selected = !item->IsSelected();
    item->SetSelected( selected ); // toggle selection
  }

  try
  {
    // set backgrounds of widgets accordingly
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
        catch ( mitk::DataTreeFilter::NoPermissionException& e )
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
      QmitkListViewExpanderIcon* childExpander = new QmitkListViewExpanderIcon(childrenGridLayout, parent);
      childExpander->setBackgroundMode( Qt::PaletteBase );
      
      index->addMultiCellWidget(childExpander, row, row+1, 0, 0, Qt::AlignTop); 
                                        // fromRow, toRow, fromCol, toCol
      index->addIndex(childExpander, row); 
     
      index->m_Grid->addMultiCellLayout( childrenGridLayout, row+1, row+1, 1, visibleProps.size(), Qt::AlignVCenter );
      // add children
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

void QmitkDataTreeListView::GenerateItems()
{
  resize(300,400);
  if (!m_DataTreeFilter) return;

  // query DataTreeFilter about items and properties, 
  // then ask factory to create PropertyObservers, 
  // add them to the visible Qt items
  const mitk::DataTreeFilter::PropertyList&  visibleProps( m_DataTreeFilter->GetVisibleProperties() );
  const mitk::DataTreeFilter::PropertyList& editableProps( m_DataTreeFilter->GetEditableProperties() );

  // create a new layout grid
  if (m_Grid) delete m_Grid;
  m_Grid = new QGridLayout( this, 1, visibleProps.size()+1 ); // 1 extra for expansion symbol
  
  if (m_StretchedColumn == -1)
    m_StretchedColumn = visibleProps.size();

 
  // fill rows with property views for the visible items 
  AddItemsToList(this, this, m_DataTreeFilter->GetItems(), visibleProps, editableProps);
}


