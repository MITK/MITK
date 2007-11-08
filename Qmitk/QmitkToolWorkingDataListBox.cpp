#include "QmitkToolWorkingDataListBox.h"
#include "QmitkPropertyListPopup.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkRenderingManager.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qevent.h>
#include <qapplication.h>
#include <qheader.h>

#include <itkCommand.h>

#include <algorithm>
#include <sstream>

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

#include "mitkNodePredicateBase.h"
 
class QmitkToolWorkingDataListBoxUpdateDataEvent : public QCustomEvent
{
  public:
    QmitkToolWorkingDataListBoxUpdateDataEvent()
    : QCustomEvent( QEvent::MaxUser - 256 )
    {
    }
};
   

QmitkToolWorkingDataListBox::QmitkToolWorkingDataListBox(QWidget* parent, const char* name)
:QListView(parent, name),
 m_ShowOnlySelected(true),
 m_SelfCall(false),
 m_LastKeyFilterObject(NULL),
 m_LastSelectedReferenceData(NULL)
{
  m_ToolManager = mitk::ToolManager::New(); // this widget should be placeable from designer so it can't take other than the defaul parameters

  QListView::addColumn( "" ); // color
  QListView::addColumn( "Segmentation" );
  QListView::setResizeMode( QListView::LastColumn ); // stretch to full width
  QListView::setSortColumn( -1 ); // no sorting
  QListView::setSelectionMode( QListView::Extended );
  QListView::setAllColumnsShowFocus( true );

  connect( this, SIGNAL(rightButtonClicked( QListViewItem*, const QPoint&, int )),
               this, SLOT(itemRightClicked( QListViewItem*, const QPoint&, int )) );
  
  connect( this, SIGNAL(selectionChanged()),
           this, SLOT(OnWorkingDataSelectionChanged()) );

  {
  itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::New();
  command->SetCallbackFunction( this, &QmitkToolWorkingDataListBox::OnToolManagerWorkingDataModified );
  m_ToolWorkingDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolWorkingDataChangedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::New();
  command->SetCallbackFunction( this, &QmitkToolWorkingDataListBox::OnToolManagerReferenceDataModified );
  m_ToolReferenceDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }


}

QmitkToolWorkingDataListBox::~QmitkToolWorkingDataListBox()
{
}

mitk::ToolManager* QmitkToolWorkingDataListBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolWorkingDataListBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->RemoveObserver( m_ToolWorkingDataChangedObserverTag );
  m_ToolManager->RemoveObserver( m_ToolReferenceDataChangedObserverTag );

  m_ToolManager = &newManager;

  {
  itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::New();
  command->SetCallbackFunction( this, &QmitkToolWorkingDataListBox::OnToolManagerWorkingDataModified );
  m_ToolWorkingDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolWorkingDataChangedEvent(), command );
  }

  {
  itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::Pointer command = itk::ReceptorMemberCommand<QmitkToolWorkingDataListBox>::New();
  command->SetCallbackFunction( this, &QmitkToolWorkingDataListBox::OnToolManagerReferenceDataModified );
  m_ToolReferenceDataChangedObserverTag = m_ToolManager->AddObserver( mitk::ToolReferenceDataChangedEvent(), command );
  }


  UpdateDataDisplay();
}
     
void QmitkToolWorkingDataListBox::OnWorkingDataSelectionChanged()
{
  mitk::ToolManager::DataVectorType selection = this->GetSelectedNodes();
  
  if (selection.size() >0)
  {
    const mitk::DataTreeNode* node = selection[0];
    emit WorkingNodeSelected(node);
  }
  else
  {
    emit WorkingNodeSelected(NULL);
  }

  m_SelfCall = true;
  m_ToolManager->SetWorkingData( selection ); // maybe empty
  m_SelfCall = false;
}

void QmitkToolWorkingDataListBox::OnToolManagerWorkingDataModified(const itk::EventObject&)
{
  UpdateNodeVisibility();

  if (m_SelfCall) return;

  const mitk::DataTreeNode* node = m_ToolManager->GetWorkingData(0);
  emit WorkingNodeSelected(node);

  UpdateDataDisplay();
}

void QmitkToolWorkingDataListBox::OnToolManagerReferenceDataModified(const itk::EventObject&)
{
  if ( m_ToolManager->GetReferenceData(0) != m_LastSelectedReferenceData )
  {
    m_ToolManager->SetWorkingData(NULL);
    UpdateDataDisplay();

    m_LastSelectedReferenceData = m_ToolManager->GetReferenceData(0);
  }
}
  
void QmitkToolWorkingDataListBox::UpdateDataDisplayLater()
{
  qApp->postEvent( this, new QmitkToolWorkingDataListBoxUpdateDataEvent() ); // one round through the event loop
}

void QmitkToolWorkingDataListBox::customEvent(QCustomEvent* e)
{
  if ( e->type() == QEvent::MaxUser - 256 )
  {
    UpdateDataDisplay();
  }
}
 
void QmitkToolWorkingDataListBox::UpdateDataDisplay()
{
  // get old/correct selection
  mitk::ToolManager::DataVectorType oldSelectedNodes = m_ToolManager->GetWorkingData(); // maybe empty

  // clear all
  QListView::clear();
  m_Node.clear();

  bool columnAdjustingNeccessary( false );

  // fix column number and labels
  if (m_AdditionalColumns.size() + 2 > QListView::columns())
  {
    for (unsigned int col = QListView::columns(); col < m_AdditionalColumns.size() + 2; ++col) // we have to add some columns
    {
      QListView::addColumn("");
    }

    columnAdjustingNeccessary = true;
  }
  else if (m_AdditionalColumns.size() + 2 < QListView::columns())
  {
    for (unsigned int col = QListView::columns()-1; col >= m_AdditionalColumns.size() + 2; --col) // we have to remove some columns
    {
      QListView::removeColumn(col);
    }
    
    columnAdjustingNeccessary = true;
  }

  if (columnAdjustingNeccessary)
  {
    unsigned int col(2);
    for (ColumnsList::iterator colIter = m_AdditionalColumns.begin();
         colIter != m_AdditionalColumns.end();
         ++colIter, ++col)
    {
      QListView::setColumnText(col, colIter->second.c_str());
    }
  }
    
  // rebuild contents
  mitk::ToolManager::DataVectorType allObjects = GetAllNodes();
  unsigned int laufendeNummer(1);
  QListViewItem* lastItem(NULL);
  for ( mitk::ToolManager::DataVectorType::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter, ++laufendeNummer )
  {
    mitk::DataTreeNode* node = *objectIter;
      
    if (node) // delete this check when datastorage is really used
    {
      // get name
      std::string name("???"); // should be white, red, blue
      node->GetName(name);

      // Do some pretty pixmap filling (background in the node's color, foreground readable, indicating the index of the item and the hotkey 
      // get color, convert it to Qt's color system
      float rgb[3]; rgb[0] = 1.0; rgb[1] = 0.0; rgb[2] = 0.0;
      node->GetColor(rgb);
      QRgb qrgb = qRgb( ROUND(rgb[0]*255.0), ROUND(rgb[1]*255.0), ROUND(rgb[2]*255.0) );
      QColor qtcolor(qrgb);

      QString text = QString("%1").arg( laufendeNummer );
      QPixmap pixmap(25,18);
      pixmap.fill(qrgb);
      QPainter painter( &pixmap );
      QPen pen = painter.pen();
      int ha,es,vau;
      qtcolor.getHsv(ha,es,vau);
      if ( vau < 160  ) 
        pen.setColor ( Qt::white );
      else
        pen.setColor ( Qt::black );
      painter.setPen( pen );
      QFontMetrics fm = painter.fontMetrics();
      QRect bb = fm.boundingRect( text );
      painter.drawText( (25 - bb.width()) / 2,
                        (18 + bb.height()) / 2,
                        text, 0, -1 );

      // create a list item
      QListViewItem* newItem = lastItem = new QListViewItem( this, lastItem ); // always insert after the last item to insert front to back
      newItem->setPixmap(0,pixmap);
      newItem->setText(1,name.c_str());

      // set optional descriptive columns
      unsigned int col = 2;
      for (ColumnsList::iterator colIter = m_AdditionalColumns.begin();
           colIter != m_AdditionalColumns.end();
           ++colIter, ++col)
      {
        mitk::FloatProperty::Pointer floatProperty = dynamic_cast<mitk::FloatProperty*>( node->GetProperty( colIter->first.c_str() ).GetPointer() );
        if (floatProperty.IsNotNull())
        {
          float value( floatProperty->GetValue() );
          std::stringstream ss;
          ss << std::fixed << std::setprecision(1) << value;
          std::string textValue = ss.str();
          newItem->setText(col, textValue.c_str());
        }
        else
        {
          mitk::BaseProperty::Pointer property = node->GetProperty( colIter->first.c_str() );
          if (property.IsNotNull())
          {
            std::string textValue( property->GetValueAsString() );
            newItem->setText(col, textValue.c_str());
          }
        }
      }

      if ( std::find(oldSelectedNodes.begin(), oldSelectedNodes.end(), node) != oldSelectedNodes.end() )
      {
        newItem->setSelected(true);
      }

      m_Node.insert( std::make_pair( newItem, node ) );
    }
  }

  if (columnAdjustingNeccessary)
  {

    for (unsigned int col = 1; col < QListView::columns(); ++col)
    {
      if (col>1) QListView::setColumnWidthMode(col, QListView::Maximum);
      QListView::header()->setStretchEnabled(true, col); 
    }
    
    for (unsigned int col = 1; col < QListView::columns(); ++col)
      QListView::adjustColumn(col);
    
    QListView::setResizeMode( QListView::LastColumn ); // stretch to fill whole width of box
    QListView::header()->adjustHeaderSize(); 
  }

}
    
mitk::ToolManager::DataVectorType QmitkToolWorkingDataListBox::GetSelectedNodes()
{
  mitk::ToolManager::DataVectorType result;

  QListViewItemIterator itemIter( this );
  while ( itemIter.current() )
  {
    QListViewItem* item = itemIter.current();

    if ( item->isSelected() )
    {
      ItemNodeMapType::iterator iter = m_Node.find(item);
      if ( iter != m_Node.end() )
      {
        result.push_back( iter->second );
      }
      else
      {
        std::cout << "kaputt" << std::endl;
      }
    }

    ++itemIter;
  }

  return result;
}

mitk::DataTreeNode* QmitkToolWorkingDataListBox::GetSelectedNode()
{
  QListViewItem* item = QListView::selectedItem();
  if (item)
  {
    ItemNodeMapType::iterator iter = m_Node.find(item);
    if ( iter != m_Node.end() )
    {
      return iter->second;
    }
  }

  return NULL;
}

mitk::ToolManager::DataVectorType QmitkToolWorkingDataListBox::GetAllNodes( bool onlyDerivedFromOriginal )
{
  mitk::DataStorage* dataStorage = mitk::DataStorage::GetInstance();

  mitk::NodePredicateProperty isSegmentation("segmentation", new mitk::BoolProperty(true));
  mitk::DataStorage::SetOfObjects::ConstPointer allObjects;
 
  if ( onlyDerivedFromOriginal )
  {
    mitk::DataTreeNode* sourceNode( m_ToolManager->GetReferenceData(0) );
    if (sourceNode)
    {
      allObjects = dataStorage->GetDerivations( sourceNode, &isSegmentation, false );
    }
    else
    {
      allObjects = mitk::DataStorage::SetOfObjects::New();
    }
  }
  else
  {
    allObjects = dataStorage->GetSubset( isSegmentation );
  }

  mitk::ToolManager::DataVectorType resultVector;

  for ( mitk::DataStorage::SetOfObjects::const_iterator objectIter = allObjects->begin();
        objectIter != allObjects->end();
        ++objectIter )
  {
    mitk::DataTreeNode* node = (*objectIter).GetPointer();
    resultVector.push_back( node );
  }

  return resultVector; 
}

void QmitkToolWorkingDataListBox::itemRightClicked( QListViewItem* item, const QPoint& p, int )
{
  if (item)
  {
    // get node and display popup
    ItemNodeMapType::iterator iter = m_Node.find(item);
    if ( iter != m_Node.end() )
    {
      mitk::DataTreeNode* node( iter->second );

      if (node)
      {
        QmitkPropertyListPopup* popup = new QmitkPropertyListPopup( node->GetPropertyList(), this, true );
        popup->popup(p);
        delete popup;

        UpdateDataDisplayLater();
      }
    }
  }
}

void QmitkToolWorkingDataListBox::SetShowOnlySelected(bool on)
{
  m_ShowOnlySelected = on;

  UpdateNodeVisibility();
}

void QmitkToolWorkingDataListBox::UpdateNodeVisibility()
{
  // hide or show all nodes of our list
  mitk::ToolManager::DataVectorType allObjects = GetAllNodes(false); // really get all possible "segmentation"s and hide/show them
  for ( mitk::ToolManager::DataVectorType::const_iterator objectIter = allObjects.begin();
        objectIter != allObjects.end();
        ++objectIter)
  {
    (*objectIter)->SetVisibility(!m_ShowOnlySelected);
  }
 
  if (m_ShowOnlySelected)
  {
    // show all selected nodes
    mitk::ToolManager::DataVectorType segmentations = m_ToolManager->GetWorkingData();
    for ( mitk::ToolManager::DataVectorType::iterator iter = segmentations.begin();
          iter != segmentations.end();
          ++iter )
    {
      (*iter)->SetVisibility(true);
    }
  }
      
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
    
void QmitkToolWorkingDataListBox::SetAdditionalColumns(const std::string& columns)
{
  m_AdditionalColumns.clear();

  StringListType chunks = Split( columns, ',', '\\' );
  for (StringListType::iterator chunkIter = chunks.begin();
       chunkIter != chunks.end();
       ++chunkIter)
  {
    StringListType keyValuePair = Split( *chunkIter, ':', '\\' );
    if (keyValuePair.size() == 2)
    {
      m_AdditionalColumns.push_back( std::make_pair(keyValuePair[0], keyValuePair[1]) );
    }
    else
    {
      std::cout << "Sorry, don't understand the following line. Was expecting something like 'property key:label for property' (two strings separated by a colon)" << std::endl;
      for (StringListType::iterator chunkIter2 = keyValuePair.begin();
           chunkIter2 != keyValuePair.end();
           ++chunkIter2)
      {
        std::cout << "'" << *chunkIter << "' " << std::endl;
      }
    }
  }

  /*
  unsigned int col(1);
  for (ColumnsList::iterator colIter = m_AdditionalColumns.begin();
       colIter != m_AdditionalColumns.end();
       ++colIter, ++col)
  {
    std::cout << col << ".: '" << colIter->first << "' labeled as '" << colIter->second << "'" << std::endl;
  }
  */
}

/**
 Splits a string into a list of substrings, that were separated by the character \a separator.
 Within the chunks the separator character can be used as a normal character if it's escaped with \a escape
*/
QmitkToolWorkingDataListBox::StringListType QmitkToolWorkingDataListBox::Split( const std::string& wholeString, const char separator, const char escape)
{
  StringListType resultVector;
  std::string::size_type currentPos( 0 );
  std::string::size_type nextPos ( 0 );
  std::string chunk;

  if (wholeString[ currentPos ] == separator)
  {
    resultVector.push_back( chunk );
    ++currentPos;
  }

  while (currentPos != std::string::npos)
  {
    nextPos = wholeString.find( separator, currentPos);
    chunk += wholeString.substr( currentPos, nextPos - currentPos);
    if (    nextPos == std::string::npos        // no separator anymore
        || wholeString[nextPos-1] != escape ) // or this was a real (unescaped) separator
    {
      resultVector.push_back( chunk );
      chunk.clear();
    }

    if (nextPos != std::string::npos)
    {
      if (wholeString[nextPos-1] == escape) // so, the last separator was just escaped. this means we have to rescue it into the result
      {
        chunk += separator;
      }
      currentPos = nextPos + 1;
    }
    else
      currentPos = nextPos;
  }

  return resultVector;
}

void QmitkToolWorkingDataListBox::keyReleaseEvent( QKeyEvent * e )
{
//handle DELETE key, reach through all others
  if ( e->key() == Qt::Key_Delete )
  {
    emit DeleteKeyPressed();
  }
}

bool QmitkToolWorkingDataListBox::eventFilter( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent *k = (QKeyEvent *)e;
    if ( k->key() >= Qt::Key_0 && k->key() <= Qt::Key_9 ) // are we interested in this key?
    {
      unsigned int figuredOutIndex(9);
      if ( k->key() > Qt::Key_0 )
        figuredOutIndex = k->key() - Qt::Key_1;

      mitk::ToolManager::DataVectorType allSegmentations = GetAllNodes(); // find the index in our list and activate it
      {
        try
        {
          mitk::DataTreeNode* node = allSegmentations.at(figuredOutIndex);
          m_ToolManager->SetWorkingData( node );
        }
        catch(std::out_of_range)
        {
          return QListView::eventFilter( o, e );
        }
      }

      return false; // let anybody know about this event, don't swallow it
    }
    else if ( k->key() == Qt::Key_Space )
    {
      mitk::ToolManager::DataVectorType segmentations = m_ToolManager->GetWorkingData();
      for ( mitk::ToolManager::DataVectorType::iterator iter = segmentations.begin();
            iter != segmentations.end();
            ++iter )
      {
        (*iter)->SetVisibility(false);
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return false; // let anybody know about this event, don't swallow it
    }
  }

  else if ( e->type() == QEvent::KeyRelease )
  {
    QKeyEvent *k = (QKeyEvent *)e;
    if ( k->key() == Qt::Key_Space )
    {
      mitk::ToolManager::DataVectorType segmentations = m_ToolManager->GetWorkingData();
      for ( mitk::ToolManager::DataVectorType::iterator iter = segmentations.begin();
            iter != segmentations.end();
            ++iter )
      {
        (*iter)->SetVisibility(true);
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      return false; // let anybody know about this event, don't swallow it
    }
  }

  return QListView::eventFilter( o, e ); // default: reach through to parent
}

void QmitkToolWorkingDataListBox::InstallKeyFilterOn( QObject* object )
{
  if (m_LastKeyFilterObject)
    m_LastKeyFilterObject->removeEventFilter(this);

  if (!object) return;
  m_LastKeyFilterObject = object;

  object->installEventFilter( this );
}

