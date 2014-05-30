/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
#include <QWidgetAction>

QmitkPropertyListPopup::QmitkPropertyListPopup( mitk::PropertyList* list, QObject* parent, bool disableBoolProperties, bool fillMenuImmediatelty, const char* name )
:QObject(parent, name),
m_PopupMenu( new QMenu( name, dynamic_cast<QWidget*>(parent) ) ),
m_PropertyList(list),
m_MaterialEditor(NULL),
m_NameMenuAction(0),
m_VisibleMenuAction(0),
m_ColorMenuAction(0),
m_MaterialMenuAction(0),
m_OpacityMenuAction(0),
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
  connect( m_PopupMenu, SIGNAL(hovered(int)), this, SLOT(popupMenuItemHovered(int)) );
}

QIcon QmitkPropertyListPopup::createColorIcon(QColor color)
{
  QPixmap pixmap(20, 20);
  QPainter painter(&pixmap);
  painter.setPen(Qt::NoPen);
  painter.fillRect(QRect(0, 0, 20, 20), color);

  return QIcon(pixmap);
}

void QmitkPropertyListPopup::fillPopup()
{
  if (m_PropertyList.IsNotNull())
  {
    // color
    mitk::ColorProperty* colorProperty = dynamic_cast<mitk::ColorProperty*>( m_PropertyList->GetProperty("color"));
    if (colorProperty)
    {
      mitk::Color col = colorProperty->GetColor();
      QColor currentColor((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255), (int)(col.GetBlue() * 255));

      QFont normalFont;
      normalFont.setBold(true);
      m_ColorMenuAction = new QAction(this->createColorIcon(currentColor), QString("Color..."), this);
      m_ColorMenuAction->setFont(normalFont);
      m_PopupMenu->addAction(m_ColorMenuAction);
      m_ColorMenuAction->setEnabled(true);
      connect( m_ColorMenuAction, SIGNAL(triggered()), this, SLOT(onColorClicked()) );
    }
    else
    {
      m_ColorMenuAction = new QAction(QString("Color..."), this);
      m_PopupMenu->addAction(m_ColorMenuAction);
      m_ColorMenuAction->setEnabled(true);
    }

    if ( !AddMaterialPopup() )
    {
      m_MaterialMenuAction = new QAction(QString("Material"), this);
      m_PopupMenu->addAction(m_MaterialMenuAction);
      m_MaterialMenuAction->setEnabled(false);
    }


    // opacity
    if ( mitk::FloatProperty* opacityProperty = dynamic_cast<mitk::FloatProperty*>( m_PropertyList->GetProperty("opacity")))
    {
      m_OriginalOpacity = mitk::FloatProperty::New( opacityProperty->GetValue() );
      QMenu* opacityPopup = m_PopupMenu->addMenu("Opacity");

      QmitkNumberPropertySlider* npe = new QmitkNumberPropertySlider( opacityProperty, opacityPopup );
      npe->setShowPercent(true);
      npe->setMinValue(0);
      npe->setMaxValue(1);

      QWidgetAction* opacityMenuAction = new QWidgetAction(opacityPopup);
      opacityMenuAction->setDefaultWidget(npe);
      m_OpacityMenuAction = opacityMenuAction;
      opacityPopup->addAction(m_OpacityMenuAction);
      m_OpacityMenuAction->setEnabled(true);
    }
    else
    {
      m_OpacityMenuAction = new QAction(QString("Opacity"), this);
      m_PopupMenu->addAction(m_OpacityMenuAction);
      m_OpacityMenuAction->setEnabled(true);
    }

    m_NameMenuAction = new QAction(QString("Name..."), this);
    m_PopupMenu->addAction(m_NameMenuAction);
    mitk::StringProperty* nameProperty = dynamic_cast<mitk::StringProperty*>( m_PropertyList->GetProperty("name"));
    m_NameMenuAction->setEnabled(nameProperty != NULL);
    if (nameProperty)
    {
      connect( m_NameMenuAction, SIGNAL(triggered()), this, SLOT(onNameClicked()) );
    }

    m_VisibleMenuAction = new QAction(QString("Visibility"), this);
    m_VisibleMenuAction->setCheckable(true);
    m_PopupMenu->addAction(m_VisibleMenuAction);
    mitk::BoolProperty* visibleProperty = dynamic_cast<mitk::BoolProperty*>( m_PropertyList->GetProperty("visible"));
    m_VisibleMenuAction->setEnabled( visibleProperty != NULL );
    if (visibleProperty)
    {
      m_VisibleMenuAction->setChecked( visibleProperty->GetValue() );
      connect( m_VisibleMenuAction, SIGNAL(triggered()), this, SLOT(onVisibleClicked()) );
    }

    // other properties, "information"
    const mitk::PropertyList::PropertyMap* map = m_PropertyList->GetMap();
    if (map)
    {
      m_InfoPopup = m_PopupMenu->addMenu("Information");

      m_PopupMenu->addSeparator();

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
  if ( mitk::MaterialProperty* materialProperty = dynamic_cast<mitk::MaterialProperty*>( m_PropertyList->GetProperty("material"))) // normal "material"
  {
    m_OriginalMaterial = mitk::MaterialProperty::New( *materialProperty );
    QMenu* materialPopup = new QMenu( m_PopupMenu );

    m_MaterialEditor = new QmitkMaterialEditor( m_PopupMenu );
    m_MaterialEditor->setInline(true); // important to call this first :(
    m_MaterialEditor->Initialize( materialProperty );
    // setting QDialog as menu item with Qt4 QWidgetAction
    QWidgetAction* materialEditorMenuItem = new QWidgetAction(materialPopup);
    materialEditorMenuItem->setDefaultWidget(m_MaterialEditor);
    materialPopup->addAction(materialEditorMenuItem);
    connect( m_MaterialEditor, SIGNAL(ChangesAccepted(QmitkMaterialEditor*)), this, SLOT(MaterialEditorChangesAccepted(QmitkMaterialEditor*)) );

    m_MaterialMenuAction = new QAction(QString("Material"), materialPopup);
    materialPopup->addAction(m_OpacityMenuAction);
    m_OpacityMenuAction->setEnabled(true);

    return true;
  }

  return false;
}

QmitkPropertyListPopup::~QmitkPropertyListPopup()
{
  delete m_MaterialEditor;
}

void QmitkPropertyListPopup::popup( const QPoint& pos, QAction* action)
{
  m_PopupMenu->exec(pos, action);
}

void QmitkPropertyListPopup::onNameClicked()
{
  mitk::StringProperty* nameProperty = dynamic_cast<mitk::StringProperty*>( m_PropertyList->GetProperty("name"));
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
      emit propertyListChangesDone();
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
  m_VisibleMenuAction->setChecked(m_VisibleMenuAction->isChecked());
  mitk::BoolProperty* visibleProperty = dynamic_cast<mitk::BoolProperty*>( m_PropertyList->GetProperty("visible"));
  if (visibleProperty)
  {
    visibleProperty->SetValue( m_VisibleMenuAction->isChecked() );
    visibleProperty->Modified(); // quite stupid that this is not done in SetValue() to inform observers
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    emit propertyListChangesDone();
  }
}

void QmitkPropertyListPopup::onColorClicked()
{
  mitk::ColorProperty* colorProperty = dynamic_cast<mitk::ColorProperty*>( m_PropertyList->GetProperty("color"));
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

void QmitkPropertyListPopup::MaterialEditorChangesAccepted(QmitkMaterialEditor*)
{
}

void QmitkPropertyListPopup::popupAboutToHide()
{
  if (!m_AcceptOnHide) return;

  bool changes(false);

  UpdateNodeMaterialOnPopupHiding( changes );

  mitk::FloatProperty* opacity = dynamic_cast<mitk::FloatProperty*>( m_PropertyList->GetProperty("opacity"));
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

void QmitkPropertyListPopup::popupMenuItemHovered( QAction* action )
{
  if ( action == m_OpacityMenuAction || action == m_MaterialMenuAction )
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
  mitk::MaterialProperty* material = dynamic_cast<mitk::MaterialProperty*>( m_PropertyList->GetProperty("material"));
  if (material)
  {
    mitk::DataNode* node = material->GetDataNode();
    material->SetDataNode(NULL);
    m_OriginalMaterial->SetDataNode(NULL);
    if ( !(*material == *m_OriginalMaterial) )
    {
      changes = true;
    }
    material->SetDataNode(node);
  }
}

