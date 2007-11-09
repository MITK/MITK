/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "QmitkPropertyListPopup.h"
#include "QmitkMaterialEditor.h"
#include "QmitkNumberPropertySlider.h"

#include "mitkRenderingManager.h"
#include "mitkStringProperty.h"
#include "mitkColorProperty.h"

#include <qinputdialog.h>
#include <qcolordialog.h>
#include <qmessagebox.h>

#include <qmenudata.h>
#include <qpainter.h>

/*!
   Auxiliary class to provide fancy menu items with different fonts.

   Copied from the Qt example program menu/menu.cpp:
   "This example program may be used, distributed and modified without limitation."
 */
class QFontMenuItem : public QCustomMenuItem
{
  public:
    QFontMenuItem( const QString& s, const QFont& f )
      : string( s ), font( f ), color ( Qt::black ) {};
    QFontMenuItem( const QString& s, const QFont& f, const QColor& c )
      : string( s ), font( f ), color( c ) {};
    ~QFontMenuItem(){}

    void paint( QPainter* p, const QColorGroup& /*cg*/, bool /*act*/, bool /*enabled*/, int x, int y, int w, int h )
    {
      p->setBackgroundMode ( Qt::OpaqueMode );
      int ha,es,vau;
      color.getHsv(ha,es,vau);
      if ( es < 60 && vau > 200 ) 
        p->setBackgroundColor ( Qt::black );
      else
        p->setBackgroundColor ( Qt::white );
      p->setPen ( color );
      p->setFont ( font );
      p->drawText( x, y, w, h, AlignLeft | AlignVCenter | DontClip | ShowPrefix, string );
    }

    QSize sizeHint()
    {
      return QFontMetrics( font ).size( AlignLeft | AlignVCenter | ShowPrefix | DontClip,  string );
    }
  private:
    QString string;
    QFont font;
    QColor color;
};

QmitkPropertyListPopup::QmitkPropertyListPopup( mitk::PropertyList* list, QObject* parent, bool disableBoolProperties, bool fillMenuImmediatelty, const char* name )
:QObject(parent, name),
 m_PopupMenu( new QPopupMenu( dynamic_cast<QWidget*>(parent), name ) ),
 m_PropertyList(list),
 m_MaterialEditor(NULL),
 m_AcceptOnHide(false),
 m_DisableBoolProperties(disableBoolProperties)
{
  if (!parent)
  {
    std::cerr << "In " __FILE__ ", l." << __LINE__ << ": popup menu without parent. This tends to crash (click 'Choose color' in the material editor)." << std::endl;
  }

  if (fillMenuImmediatelty)
  {
    fillPopup();
  }

  connect( m_PopupMenu, SIGNAL(aboutToHide()), this, SLOT(popupAboutToHide()) );
  connect( m_PopupMenu, SIGNAL(highlighted(int)), this, SLOT(popupMenuItemHighlighted(int)) );
}

void QmitkPropertyListPopup::fillPopup()
{
  if (m_PropertyList.IsNotNull())
  {
    m_PopupMenu->setCheckable(true); // in general, there could are some checkable items in this menu

    // color
    mitk::ColorProperty* colorProperty = dynamic_cast<mitk::ColorProperty*>( m_PropertyList->GetProperty("color").GetPointer() );
    if (colorProperty)
    {
      mitk::Color col = colorProperty->GetColor();
      QColor currentColor((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255), (int)(col.GetBlue() * 255));

      QFont normalFont;
      normalFont.setBold(true);
      m_ColorMenuID = m_PopupMenu->insertItem( new QFontMenuItem("Color...", normalFont, currentColor ) );
      m_PopupMenu->connectItem( m_ColorMenuID, this, SLOT(onColorClicked()) );
      m_PopupMenu->setItemEnabled( m_ColorMenuID, true );
    }
    else
    {
      m_ColorMenuID = m_PopupMenu->insertItem( "Color..." );
      m_PopupMenu->setItemEnabled( m_ColorMenuID, false );
    }

    if ( !AddMaterialPopup() )
    {
      m_MaterialMenuID = m_PopupMenu->insertItem("Material");
      m_PopupMenu->setItemEnabled( m_MaterialMenuID, false );
    }


    // opacity
    if ( mitk::FloatProperty* opacityProperty = dynamic_cast<mitk::FloatProperty*>( m_PropertyList->GetProperty("opacity").GetPointer() ))
    {
      m_OriginalOpacity = new mitk::FloatProperty( opacityProperty->GetValue() );
      QPopupMenu* opacityPopup = new QPopupMenu( m_PopupMenu );
   
      QmitkNumberPropertySlider* npe = new QmitkNumberPropertySlider( opacityProperty, opacityPopup );
      npe->setShowPercent(true);
      npe->setMinValue(0);
      npe->setMaxValue(1);
      opacityPopup->insertItem( npe );

      m_OpacityMenuID = m_PopupMenu->insertItem("Opacity", opacityPopup);
      m_PopupMenu->setItemEnabled( m_OpacityMenuID, true );
    }
    else
    {
      m_OpacityMenuID = m_PopupMenu->insertItem("Opacity");
      m_PopupMenu->setItemEnabled( m_OpacityMenuID, false );
    }

    // Build up a "name" entry. On click, call onNameChangeClicked.
    m_NameMenuID = m_PopupMenu->insertItem("Name...");
    mitk::StringProperty* nameProperty = dynamic_cast<mitk::StringProperty*>( m_PropertyList->GetProperty("name").GetPointer() );
    m_PopupMenu->setItemEnabled( m_NameMenuID, nameProperty != NULL );
    if (nameProperty)
    {
      m_PopupMenu->connectItem( m_NameMenuID, this, SLOT(onNameClicked()) );
    }


    // Build up a checkable "visible" entry. On click, call onVisibleChanged.
    m_VisibleMenuID = m_PopupMenu->insertItem("Visibility");
    mitk::BoolProperty* visibleProperty = dynamic_cast<mitk::BoolProperty*>( m_PropertyList->GetProperty("visible").GetPointer() );
    m_PopupMenu->setItemEnabled( m_VisibleMenuID, visibleProperty != NULL );
    if (visibleProperty)
    {
      m_PopupMenu->setItemChecked( m_VisibleMenuID, visibleProperty->GetValue() );
      m_PopupMenu->connectItem( m_VisibleMenuID, this, SLOT(onVisibleClicked()) );
    }

    // other properties, "information"
    const mitk::PropertyList::PropertyMap* map = m_PropertyList->GetMap();
    if (map)
    {
      // build a sub-menu with all properties shown
      m_InfoPopup = new QPopupMenu( m_PopupMenu );
      m_InfoPopup->setCheckable(true); // bool properties are checked
      m_PopupMenu->insertItem("Information", m_InfoPopup);

      m_PopupMenu->insertSeparator();

      QFont boldFont = m_PopupMenu->font();
      boldFont.setBold( true );
     
      // first all bool properties
      for ( mitk::PropertyList::PropertyMap::const_iterator propertyIter = map->begin();
            propertyIter != map->end();
            ++propertyIter )
      {
        std::string name = propertyIter->first;
        if (name == "visible") continue; // we already display that above

        if ( mitk::BoolProperty* boolProperty = dynamic_cast<mitk::BoolProperty*>(propertyIter->second.first.GetPointer()) )
        {
          // fill a vector (int -> smartpointer(boolprop)) for reacting to clicks on checked items
          m_BoolProperties.push_back( boolProperty );

          int newID = m_PopupMenu->insertItem( QString("%1").arg( name.c_str()) );
          m_PopupMenu->setItemChecked( newID,  boolProperty->GetValue() );
          m_PopupMenu->setItemParameter( newID, m_BoolProperties.size() );
          m_PopupMenu->connectItem( newID, this, SLOT(onBoolPropertyClicked(int)) );
          if (m_DisableBoolProperties )
          {
            m_PopupMenu->setItemEnabled( newID, false );
          }

        }
      }

      boldFont = m_InfoPopup->font();
    
      // then all non-bool properties
      for ( mitk::PropertyList::PropertyMap::const_iterator propertyIter = map->begin();
            propertyIter != map->end();
            ++propertyIter )
      {
        std::string name = propertyIter->first;
        if ( !dynamic_cast<mitk::BoolProperty*>(propertyIter->second.first.GetPointer()) )
        {
          std::string value("no value");
          mitk::BaseProperty::Pointer bp = propertyIter->second.first;
          if ( bp.IsNotNull() )
          {
            value = bp->GetValueAsString();
          }
          m_InfoPopup->insertItem( QString("%1: %2").arg( name.c_str()).arg(value.c_str()) );
        }
      }
    }
  }
}

bool QmitkPropertyListPopup::AddMaterialPopup()
{
  // normal material
  if ( mitk::MaterialProperty* materialProperty = dynamic_cast<mitk::MaterialProperty*>( m_PropertyList->GetProperty("material").GetPointer() )) // normal "material"
  {
    m_OriginalMaterial = new mitk::MaterialProperty( *materialProperty );
    QPopupMenu* materialPopup = new QPopupMenu( m_PopupMenu );
  
    m_MaterialEditor = new QmitkMaterialEditor( m_PopupMenu );
    m_MaterialEditor->setInline(true); // important to call this first :(
    m_MaterialEditor->Initialize( materialProperty );
    materialPopup->insertItem( m_MaterialEditor );
    connect( m_MaterialEditor, SIGNAL(ChangesAccepted(QmitkMaterialEditor*)), this, SLOT(MaterialEditorChangesAccepted(QmitkMaterialEditor*)) );

    m_MaterialMenuID = m_PopupMenu->insertItem("Material", materialPopup);
    m_PopupMenu->setItemEnabled( m_MaterialMenuID, true );

    return true;
  }

  return false;
}
    
QmitkPropertyListPopup::~QmitkPropertyListPopup()
{
  delete m_MaterialEditor;
}
    
void QmitkPropertyListPopup::popup( const QPoint& pos, int indexAtPoint )
{
  m_PopupMenu->exec(pos, indexAtPoint);
}

void QmitkPropertyListPopup::onNameClicked()
{
  mitk::StringProperty* nameProperty = dynamic_cast<mitk::StringProperty*>( m_PropertyList->GetProperty("name").GetPointer() );
  if (nameProperty)
  {
    bool ok;
    QString newName = QInputDialog::getText( tr("Change object name"), 
                                             QString(tr("Enter a new name for \"%1\"")).arg(nameProperty->GetValue()), 
                                             QLineEdit::Normal, 
                                             QString(nameProperty->GetValue()), 
                                             &ok,
                                             m_PopupMenu );
    if ( ok && !newName.isEmpty() ) 
    {
      // user entered something and pressed OK
      nameProperty->SetValue( newName.ascii() );
      nameProperty->Modified();
    } 
    else if (ok)
    { 
      // user entered nothing or pressed Cancel
       if ( QMessageBox::question( m_PopupMenu, 
                                   tr("Change object name"), 
                                   tr("Do you really want to assign an empty name to '%1'?").arg( nameProperty->GetValue() ), 
                                   QMessageBox::Yes,
                                   QMessageBox::No )
            == QMessageBox::Yes )
       {
         // ok, this user is sure, we assign "" as a name
         nameProperty->SetValue( newName.ascii() );
         nameProperty->Modified();
         emit propertyListChangesDone();
       }
    }
  }
}

void QmitkPropertyListPopup::onVisibleClicked()
{
  m_PopupMenu->setItemChecked( m_VisibleMenuID, !m_PopupMenu->isItemChecked(m_VisibleMenuID) );
  mitk::BoolProperty* visibleProperty = dynamic_cast<mitk::BoolProperty*>( m_PropertyList->GetProperty("visible").GetPointer() );
  if (visibleProperty)
  {
    visibleProperty->SetValue( m_PopupMenu->isItemChecked( m_VisibleMenuID ) );
    visibleProperty->Modified(); // quite stupid that this is not done in SetValue() to inform observers
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    emit propertyListChangesDone();
  }
}

void QmitkPropertyListPopup::onColorClicked()
{
  mitk::ColorProperty* colorProperty = dynamic_cast<mitk::ColorProperty*>( m_PropertyList->GetProperty("color").GetPointer() );
  if (colorProperty)
  {
    mitk::Color col = colorProperty->GetColor();
    QColor result = QColorDialog::getColor(QColor((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255), (int)(col.GetBlue() * 255)));
    if (result.isValid())
    {
      col.SetRed(result.red() / 255.0);
      col.SetGreen(result.green() / 255.0);
      col.SetBlue(result.blue() / 255.0);
      colorProperty->SetColor(col);
      colorProperty->Modified(); // quite stupid that this is not done in SetColor() to inform observers
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    emit propertyListChangesDone();
  }
}

void QmitkPropertyListPopup::onBoolPropertyClicked(int param)
{
  int item = m_PopupMenu->idAt(param+6); // plus number of items before all the boolean properties (excluding separator)
  bool on( !m_PopupMenu->isItemChecked(item) );
  
  m_PopupMenu->setItemChecked( item, on ); // toggle
  
  try
  {
    mitk::BoolProperty* boolProperty = m_BoolProperties.at( param-1 );
    if (boolProperty)
    {
      boolProperty->SetValue( on );
      boolProperty->Modified();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      emit propertyListChangesDone();
    }
  }
  catch(...)
  {
    // strange
  }
}

void QmitkPropertyListPopup::MaterialEditorChangesAccepted(QmitkMaterialEditor* /*ed*/)
{
}

void QmitkPropertyListPopup::popupAboutToHide()
{
  if (!m_AcceptOnHide) return;

  bool changes(false);

  UpdateNodeMaterialOnPopupHiding( changes );

  mitk::FloatProperty* opacity = dynamic_cast<mitk::FloatProperty*>( m_PropertyList->GetProperty("opacity").GetPointer() );
  if (opacity)
  {
    if ( !(*opacity == *m_OriginalOpacity) )
    {
      changes = true;
    }
  }

  if (changes)
  {
    emit propertyListChangesDone();
  }
}

void QmitkPropertyListPopup::popupMenuItemHighlighted(int id)
{
  if ( id == m_OpacityMenuID || id == m_MaterialMenuID )
  {
    m_AcceptOnHide = true;
  }
  else
  {
    m_AcceptOnHide = false;
  }
}

void QmitkPropertyListPopup::UpdateNodeMaterialOnPopupHiding( bool& changes )
{
  mitk::MaterialProperty* material = dynamic_cast<mitk::MaterialProperty*>( m_PropertyList->GetProperty("material").GetPointer() );
  if (material)
  {
    mitk::DataTreeNode* node = material->GetDataTreeNode();
    material->SetDataTreeNode(NULL);
    m_OriginalMaterial->SetDataTreeNode(NULL);
    if ( !(*material == *m_OriginalMaterial) )
    {
      changes = true;
    }
    material->SetDataTreeNode(node);
  }
}

