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

void QmitkDataTreeListView::paintEvent(QPaintEvent* e)
{
  QPainter painter(this);
  //painter.setBrush(QColor(QWidget::palettePaletteHighlight));
  painter.setBrush( colorGroup().brush(QColorGroup::Highlight) );

  //QRect rect( m_ChildContainer->cellGeometry(1,1) );
  //painter.fillRect(rect, painter.brush() );
  
  painter.fillRect(0,0,width(),height(), painter.brush() );
}

void QmitkDataTreeListView::AddItemsToList(QWidget* parent, QmitkListViewExpanderIcon* expander, QGridLayout* layout,
                                           const mitk::DataTreeFilter::ItemList* items,
                                           const mitk::DataTreeFilter::PropertyList& visibleProps,
                                           const mitk::DataTreeFilter::PropertyList editableProps)
{
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
        layout->addWidget(observerWidget, row, column, Qt::AlignVCenter);
        if (expander != 0)
          expander->addWidget(observerWidget);

        observerWidget->setBackgroundMode(Qt::PaletteHighlight);
      }

      if (column == m_StretchedColumn)
        layout->setColStretch(column, 10);
      
      ++column;
    }
    
    if ( itemiter->HasChildren() )
    {
      // add expansion symbol
      QGridLayout* childrenGridLayout = new QGridLayout( 0, 1, visibleProps.size()+1 ); // 1 extra for expansion symbol
      QmitkListViewExpanderIcon* childExpander = new QmitkListViewExpanderIcon(childrenGridLayout, parent);
      
      childExpander->setBackgroundMode(Qt::PaletteHighlight);

      layout->addMultiCellWidget(childExpander, row, row+1, 0, 0, Qt::AlignTop); 
                                                                                     // fromRow, toRow, fromCol, toCol
     
      layout->addMultiCellLayout( childrenGridLayout, row+1, row+1, 1, visibleProps.size(), Qt::AlignVCenter );
      // add children
      AddItemsToList(parent, childExpander, childrenGridLayout, itemiter->GetChildren(), visibleProps, editableProps);
      
      ++row;
    }
    else
    {
      // to get some indent for child elements
      QLabel* label = new QLabel(" ", parent);
      label->setBackgroundMode(Qt::PaletteHighlight);
      layout->addWidget(label, row, 0, Qt::AlignVCenter); 
      if (expander != 0)
        expander->addWidget(label);
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
  m_Grid->setSpacing(4);
  
  if (m_StretchedColumn == -1)
    m_StretchedColumn = visibleProps.size();

 
  // fill rows with property views for the visible items 
  AddItemsToList(this, 0, m_Grid, m_DataTreeFilter->GetItems(), visibleProps, editableProps);
}


