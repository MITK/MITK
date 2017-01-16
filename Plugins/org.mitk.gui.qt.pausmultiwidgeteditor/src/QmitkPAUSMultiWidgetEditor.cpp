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

#include "QmitkPAUSMultiWidgetEditor.h"

#include <QWidget>


const QString QmitkPAUSMultiWidgetEditor::EDITOR_ID = "org.mitk.editors.pausmultiwidget";

QmitkPAUSMultiWidgetEditor::QmitkPAUSMultiWidgetEditor()
{
}

QmitkPAUSMultiWidgetEditor::~QmitkPAUSMultiWidgetEditor()
{
}

void QmitkPAUSMultiWidgetEditor::SetFocus()
{
}

bool QmitkPAUSMultiWidgetEditor::IsDecorationEnabled(const QString &decoration) const
{
  return false;
}

void QmitkPAUSMultiWidgetEditor::EnableDecorations(bool enable, const QStringList &decorations)
{
}

QStringList QmitkPAUSMultiWidgetEditor::GetDecorations() const
{
  QStringList decorations;
  decorations << DECORATION_BORDER;
  return decorations;
}

mitk::Point3D QmitkPAUSMultiWidgetEditor::GetSelectedPosition(const QString & /*id*/) const
{
  mitk::Point3D zeroPosition;
  return zeroPosition;
}

void QmitkPAUSMultiWidgetEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &/*id*/)
{
}

QHash<QString, QmitkRenderWindow *> QmitkPAUSMultiWidgetEditor::GetQmitkRenderWindows() const
{
  return m_RenderWindows;
}

QmitkRenderWindow *QmitkPAUSMultiWidgetEditor::GetActiveQmitkRenderWindow() const
{
  return 0;
}

QmitkRenderWindow *QmitkPAUSMultiWidgetEditor::GetQmitkRenderWindow(const QString &id) const
{
  return 0;
}

void QmitkPAUSMultiWidgetEditor::CreateQtPartControl(QWidget* parent)
{
  // if (d->m_StdMultiWidget == 0)
  // {
    // QHBoxLayout* layout = new QHBoxLayout(parent);
    // layout->setContentsMargins(0,0,0,0);

    // if (d->m_MouseModeToolbar == NULL)
    // {
      // d->m_MouseModeToolbar = new QmitkMouseModeSwitcher(parent); // delete by Qt via parent
      // layout->addWidget(d->m_MouseModeToolbar);
    // }

    // berry::IPreferences::Pointer prefs = this->GetPreferences();

    // mitk::BaseRenderer::RenderingMode::Type renderingMode = static_cast<mitk::BaseRenderer::RenderingMode::Type>(prefs->GetInt( "Rendering Mode" , 0 ));

    // d->m_StdMultiWidget = new QmitkStdMultiWidget(parent,0,0,renderingMode);
    // d->m_RenderWindows.insert("axial", d->m_StdMultiWidget->GetRenderWindow1());
    // d->m_RenderWindows.insert("sagittal", d->m_StdMultiWidget->GetRenderWindow2());
    // d->m_RenderWindows.insert("coronal", d->m_StdMultiWidget->GetRenderWindow3());
    // d->m_RenderWindows.insert("3d", d->m_StdMultiWidget->GetRenderWindow4());

    // d->m_MouseModeToolbar->setMouseModeSwitcher( d->m_StdMultiWidget->GetMouseModeSwitcher() );

    // layout->addWidget(d->m_StdMultiWidget);

    // mitk::DataStorage::Pointer ds = this->GetDataStorage();

    // // Tell the multiWidget which (part of) the tree to render
    // d->m_StdMultiWidget->SetDataStorage(ds);

    // // Initialize views as axial, sagittal, coronar to all data objects in DataStorage
    // // (from top-left to bottom)
    // mitk::TimeGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
    // mitk::RenderingManager::GetInstance()->InitializeViews(geo);

    // // Initialize bottom-right view as 3D view
    // d->m_StdMultiWidget->GetRenderWindow4()->GetRenderer()->SetMapperID(
      // mitk::BaseRenderer::Standard3D );

    // // Enable standard handler for levelwindow-slider
    // d->m_StdMultiWidget->EnableStandardLevelWindow();

    // // Add the displayed views to the tree to see their positions
    // // in 2D and 3D
    // d->m_StdMultiWidget->AddDisplayPlaneSubTree();

    // //d->m_StdMultiWidget->EnableNavigationControllerEventListening();

    // // Store the initial visibility status of the menu widget.
    // d->m_MenuWidgetsEnabled = d->m_StdMultiWidget->IsMenuWidgetEnabled();

    // this->GetSite()->GetPage()->AddPartListener(d->m_PartListener.data());

    // berry::IBerryPreferences* berryprefs = dynamic_cast<berry::IBerryPreferences*>(prefs.GetPointer());
    // InitializePreferences(berryprefs);
    // this->OnPreferencesChanged(berryprefs);

    // this->RequestUpdate();
  // }
}

