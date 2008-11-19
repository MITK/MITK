/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-10-30 12:33:31 +0100 (Do, 30 Okt 2008) $
Version:   $Revision: 15606 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolWorkingDataSelectionBox.h"
//#include "QmitkPropertyListPopup.h"

#include "mitkDataStorage.h"
#include "mitkNodePredicateProperty.h"
#include "mitkRenderingManager.h"

#include <qpixmap.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qevent.h>
#include <qapplication.h>
#include <Q3Header>

#include <algorithm>
#include <sstream>

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

#include "mitkNodePredicateOR.h"
#include "mitkNodePredicateAND.h"

class QmitkToolWorkingDataSelectionBoxUpdateDataEvent : public QCustomEvent
{
  public:
    QmitkToolWorkingDataSelectionBoxUpdateDataEvent()
    : QCustomEvent( QEvent::MaxUser - 256 )
    {
    }
};


QmitkToolWorkingDataSelectionBox::QmitkToolWorkingDataSelectionBox(QWidget* parent, const char* name)
:Q3ListView(parent, name),
 m_ShowOnlySelected(true),
 m_SelfCall(false),
 m_LastKeyFilterObject(NULL),
 m_LastSelectedReferenceData(NULL),
 m_DisplayMode( ListDataIfAnyToolMatches ),
 m_ToolGroupsForFiltering(std::string()),
 m_DisplayOnlyDerivedNodes(true)
{
  m_ToolManager = mitk::ToolManager::New(); // this widget should be placeable from designer so it can't take other than the defaul parameters

  Q3ListView::addColumn( "" ); // color
  Q3ListView::addColumn( "Segmentation" );
  Q3ListView::setResizeMode( Q3ListView::LastColumn ); // stretch to full width
  Q3ListView::setSortColumn( -1 ); // no sorting
  Q3ListView::setSelectionMode( Q3ListView::Extended );
  Q3ListView::setAllColumnsShowFocus( true );

  /*connect( this, SIGNAL(rightButtonClicked( QListViewItem*, const QPoint&, int )),
               this, SLOT(itemRightClicked( QListViewItem*, const QPoint&, int )) );*/

  connect( this, SIGNAL(selectionChanged()),
           this, SLOT(OnWorkingDataSelectionChanged()) );

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );
}

QmitkToolWorkingDataSelectionBox::~QmitkToolWorkingDataSelectionBox()
{
}

mitk::ToolManager* QmitkToolWorkingDataSelectionBox::GetToolManager()
{
  return m_ToolManager;
}

void QmitkToolWorkingDataSelectionBox::SetFirstColumnTitle(const QString& title)
{
  Q3ListView::setColumnText(1, title);
}

void QmitkToolWorkingDataSelectionBox::SetToolManager(mitk::ToolManager& newManager) // no NULL pointer allowed here, a manager is required
{
  m_ToolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );

  m_ToolManager = &newManager;

  m_ToolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified );
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkToolWorkingDataSelectionBox>( this, &QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified );

  UpdateDataDisplay();
}

void QmitkToolWorkingDataSelectionBox::OnWorkingDataSelectionChanged()
{
  static mitk::ToolManager::DataVectorType previouslySelectedNodes;

  mitk::ToolManager::DataVectorType selection = this->GetSelectedNodes();
  ///if ( selection == previouslySelectedNodes ) return;
  previouslySelectedNodes = selection;

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

void QmitkToolWorkingDataSelectionBox::OnToolManagerWorkingDataModified()
{
  UpdateNodeVisibility();

  if (m_SelfCall) return;


  const mitk::DataTreeNode* node = m_ToolManager->GetWorkingData(0);
  emit WorkingNodeSelected(node);

  UpdateDataDisplay();
}

void QmitkToolWorkingDataSelectionBox::OnToolManagerReferenceDataModified()
{
  if ( m_ToolManager->GetReferenceData(0) != m_LastSelectedReferenceData )
  {
    m_ToolManager->SetWorkingData(NULL);
    UpdateDataDisplay();

    m_LastSelectedReferenceData = m_ToolManager->GetReferenceData(0);
  }
}

void QmitkToolWorkingDataSelectionBox::UpdateDataDisplayLater()
{
  qApp->postEvent( this, new QmitkToolWorkingDataSelectionBoxUpdateDataEvent() ); // one round through the event loop
}

void QmitkToolWorkingDataSelectionBox::customEvent(QCustomEvent* e)
{
  if ( e->type() == QEvent::MaxUser - 256 )
  {
    UpdateDataDisplay();
  }
}

void QmitkToolWorkingDataSelectionBox::UpdateDataDisplay()
{

  // get old/correct selection
  mitk::ToolManager::DataVectorType oldSelectedNodes = m_ToolManager->GetWorkingData(); // maybe empty

  // clear all
  Q3ListView::clear();
  m_Node.clear();

  bool columnAdjustingNeccessary( false );

  // fix column number and labels
  if (m_AdditionalColumns.size() + 2 > (unsigned)Q3ListView::columns())
  {
    for (unsigned int col = Q3ListView::columns(); col < m_AdditionalColumns.size() + 2; ++col) // we have to add some columns
    {
      Q3ListView::addColumn("");
    }

    columnAdjustingNeccessary = true;
  }
  else if (m_AdditionalColumns.size() + 2 < (unsigned) Q3ListView::columns())
  {
    for (unsigned int col = Q3ListView::columns()-1; col >= m_AdditionalColumns.size() + 2; --col) // we have to remove some columns
    {
      Q3ListView::removeColumn(col);
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
      Q3ListView::setColumnText(col, colIter->second.c_str());
    }
  }

  // rebuild contents
  mitk::ToolManager::DataVectorType allObjects = GetAllNodes( m_DisplayOnlyDerivedNodes );
  unsigned int laufendeNummer(1);
  Q3ListViewItem* lastItem(NULL);
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
      //qtcolor.getHsv(ha,es,vau);
      // Qt4 mmueller
      ha = qtcolor.hue();
      es = qtcolor.saturation();
      vau = qtcolor.value();
      // Qt4
      qtcolor.setHsv(ha,es,vau);
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
      Q3ListViewItem* newItem = lastItem = new Q3ListViewItem( this, lastItem ); // always insert after the last item to insert front to back
      newItem->setPixmap(0,pixmap);
      newItem->setText(1,name.c_str());

      // set optional descriptive columns
      unsigned int col = 2;
      for (ColumnsList::iterator colIter = m_AdditionalColumns.begin();
           colIter != m_AdditionalColumns.end();
           ++colIter, ++col)
      {
        mitk::FloatProperty::Pointer floatProperty = dynamic_cast<mitk::FloatProperty*>( node->GetProperty( colIter->first.c_str() ));
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

    for (unsigned int col = 1; col < (unsigned) Q3ListView::columns(); ++col)
    {
      if (col>1) Q3ListView::setColumnWidthMode(col, Q3ListView::Maximum);
      Q3ListView::header()->setStretchEnabled(true, col);
    }

    for (unsigned int col = 1; col < (unsigned) Q3ListView::columns(); ++col)
      Q3ListView::adjustColumn(col);

    Q3ListView::setResizeMode( Q3ListView::LastColumn ); // stretch to fill whole width of box
    Q3ListView::header()->adjustHeaderSize();
  }
}

mitk::ToolManager::DataVectorType QmitkToolWorkingDataSelectionBox::GetSelectedNodes()
{
  mitk::ToolManager::DataVectorType result;

  Q3ListViewItemIterator itemIter( this );
  while ( itemIter.current() )
  {
    Q3ListViewItem* item = itemIter.current();

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

mitk::DataTreeNode* QmitkToolWorkingDataSelectionBox::GetSelectedNode()
{
  Q3ListViewItem* item = Q3ListView::selectedItem();
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

mitk::ToolManager::DataVectorType QmitkToolWorkingDataSelectionBox::GetAllNodes( bool onlyDerivedFromOriginal )
{
  mitk::DataStorage* dataStorage = mitk::DataStorage::GetInstance();

  /**
   * Build up predicate:
   *  - ask each tool that is displayed for a predicate (indicating the type of data that this tool will work with)
   *  - connect all predicates using AND or OR, depending on the parameter m_DisplayMode (ListDataIfAllToolsMatch or ListDataIfAnyToolMatches)
   *    \sa SetDisplayMode
   */

  static std::vector< const mitk::NodePredicateBase* > m_Predicates;
  static const mitk::NodePredicateBase* completePredicate = NULL;
  bool rebuildNeeded = true;
  if (rebuildNeeded)
  {
    for ( std::vector< const mitk::NodePredicateBase* >::iterator iter = m_Predicates.begin();
          iter != m_Predicates.end();
          ++iter )
    {
      delete *iter;
    }

    m_Predicates.clear();
    completePredicate = NULL;

    const mitk::ToolManager::ToolVectorTypeConst allTools = m_ToolManager->GetTools();

    for ( mitk::ToolManager::ToolVectorTypeConst::const_iterator iter = allTools.begin();
          iter != allTools.end();
          ++iter )
    {
      const mitk::Tool* tool = *iter;

      if ( (m_ToolGroupsForFiltering.empty()) || ( m_ToolGroupsForFiltering.find( tool->GetGroup() ) != std::string::npos ) ||
                                                 ( m_ToolGroupsForFiltering.find( tool->GetName() )  != std::string::npos )
         )
      {
        if (completePredicate)
        {
          if ( m_DisplayMode == ListDataIfAnyToolMatches )
          {
              m_Predicates.push_back( new mitk::NodePredicateOR( *completePredicate, tool->GetWorkingDataPreference() ) );
          }
          else
          {
              m_Predicates.push_back( new mitk::NodePredicateAND( *completePredicate, tool->GetWorkingDataPreference() ) );
          }
          completePredicate = m_Predicates.back();
        }
        else
        {
          completePredicate = &tool->GetWorkingDataPreference();
        }
      }
    }
  }

  // TODO delete all m_Predicates
  mitk::DataStorage::SetOfObjects::ConstPointer allObjects;

  /**
   * Two modes here:
   *  - display only nodes below reference data from ToolManager (onlyDerivedFromOriginal == true)
   *  - display everything matching the predicate (else)
   */
  if ( onlyDerivedFromOriginal )
  {
    mitk::DataTreeNode* sourceNode( m_ToolManager->GetReferenceData(0) );
    if (sourceNode)
    {
      allObjects = dataStorage->GetDerivations( sourceNode, completePredicate, false );
    }
    else
    {
      allObjects = mitk::DataStorage::SetOfObjects::New();
    }
  }
  else
  {
    if (completePredicate)
    {
      allObjects = dataStorage->GetSubset( *completePredicate );
    }
    else
    {
      allObjects = dataStorage->GetAll();
    }
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

/*
void QmitkToolWorkingDataSelectionBox::itemRightClicked( QListViewItem* item, const QPoint& p, int )
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
*/

void QmitkToolWorkingDataSelectionBox::SetShowOnlySelected(bool on)
{
  m_ShowOnlySelected = on;

  UpdateNodeVisibility();
}

void QmitkToolWorkingDataSelectionBox::UpdateNodeVisibility()
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

void QmitkToolWorkingDataSelectionBox::SetAdditionalColumns(const std::string& columns)
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
QmitkToolWorkingDataSelectionBox::StringListType QmitkToolWorkingDataSelectionBox::Split( const std::string& wholeString, const char separator, const char escape)
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

void QmitkToolWorkingDataSelectionBox::keyReleaseEvent( QKeyEvent* k )
{
//handle DELETE key, reach through all others
  if ( k->key() == Qt::Key_Delete )
  {
    emit DeleteKeyPressed();
  }

  if ( k->key() >= Qt::Key_0 && k->key() <= Qt::Key_9 ) return;
  if ( k->key() == Qt::Key_Space ) return;

  Q3ListView::keyPressEvent(k);
}

void QmitkToolWorkingDataSelectionBox::keyPressEvent( QKeyEvent* k )
{
  if ( k->key() >= Qt::Key_0 && k->key() <= Qt::Key_9 ) return;
  if ( k->key() == Qt::Key_Space ) return;

  Q3ListView::keyPressEvent(k);
}

bool QmitkToolWorkingDataSelectionBox::eventFilter( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent *k = (QKeyEvent *)e;
    if ( k->key() >= Qt::Key_0 && k->key() <= Qt::Key_9 ) // are we interested in this key?
    {
      unsigned int figuredOutIndex(9);
      if ( k->key() > Qt::Key_0 )
        figuredOutIndex = k->key() - Qt::Key_1;

      mitk::ToolManager::DataVectorType allSegmentations = GetAllNodes( m_DisplayOnlyDerivedNodes ); // find the index in our list and activate it
      {
        try
        {
          mitk::DataTreeNode* node = allSegmentations.at(figuredOutIndex);
          m_ToolManager->SetWorkingData( node );
          return false; // let anybody know about this event, don't swallow it
        }
        catch(std::out_of_range)
        {
        }
      }
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

  return Q3ListView::eventFilter( o, e ); // default: reach through to parent
}

void QmitkToolWorkingDataSelectionBox::InstallKeyFilterOn( QObject* object )
{
  //TODO crashes under linux in eventFilter(o,e), Q3ListView::eventFilter(o,e)
//  if (m_LastKeyFilterObject)
//    m_LastKeyFilterObject->removeEventFilter(this);
//
//  if (!object) return;
//  m_LastKeyFilterObject = object;
//
//  object->installEventFilter( this );
}

void QmitkToolWorkingDataSelectionBox::SetToolGroupsForFiltering(const std::string& groups)
{
  m_ToolGroupsForFiltering = groups;
  UpdateDataDisplay();
}

void QmitkToolWorkingDataSelectionBox::SetDisplayOnlyDerivedNodes(bool on)
{
  if ( m_DisplayOnlyDerivedNodes != on )
  {
    m_DisplayOnlyDerivedNodes = on;
    UpdateDataDisplay();
  }
}

void QmitkToolWorkingDataSelectionBox::SetDisplayMode( QmitkToolWorkingDataSelectionBox::DisplayMode mode )
{
  if (m_DisplayMode != mode)
  {
    m_DisplayMode = mode;
    UpdateDataDisplay();
  }
}

