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

#include "SimpleRenderWindowView.h"

#include <QmitkRenderWindow.h>

#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>
#include <ctkServiceTracker.h>
#include <berryIBerryPreferences.h>
#include <mitkIRenderingManager.h>
#include "org_mitk_example_gui_testapplication_Activator.h"

#include <QVBoxLayout>

class AbstractRenderWindowViewPrivate
{
public:

  AbstractRenderWindowViewPrivate()
    : m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
    , m_PrefServiceTracker(org_mitk_example_gui_testapplication_Activator::GetPluginContext())
  {
    m_PrefServiceTracker.open();
  }

  ~AbstractRenderWindowViewPrivate()
  {
    delete m_RenderingManagerInterface;
  }

  mitk::IRenderingManager* m_RenderingManagerInterface;
  ctkServiceTracker<berry::IPreferencesService*> m_PrefServiceTracker;
  berry::IBerryPreferences::Pointer m_Prefs;
};

//const std::string SimpleRenderWindowView::VIEW_ID = "org.mitk.editors.stdmultiwidget";
const std::string SimpleRenderWindowView::VIEW_ID = "org.mitk.views.stdmultiwidget";


SimpleRenderWindowView::SimpleRenderWindowView()
  : m_RenderWindow(0), d(new AbstractRenderWindowViewPrivate)
{
}

QmitkRenderWindow *SimpleRenderWindowView::GetActiveRenderWindow() const
{
  return m_RenderWindow;
}

QHash<QString, QmitkRenderWindow *> SimpleRenderWindowView::GetRenderWindows() const
{
  QHash<QString, QmitkRenderWindow*> wnds;
  wnds.insert("transversal", m_RenderWindow);
  return wnds;
}

QmitkRenderWindow *SimpleRenderWindowView::GetRenderWindow(const QString &id) const
{
  if (id == "transversal")
  {
    return m_RenderWindow;
  }
  return 0;
}

mitk::Point3D SimpleRenderWindowView::GetSelectedPosition(const QString & /*id*/) const
{
  const mitk::PlaneGeometry* pg =
      m_RenderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  if (pg)
  {
    return pg->GetCenter();
  }
  else
  {
    return mitk::Point3D();
  }
}

void SimpleRenderWindowView::SetSelectedPosition(const mitk::Point3D &pos, const QString &/*id*/)
{
  // create a PositionEvent with the given position and
  // tell the slice navigation controller to move there

  mitk::Point2D p2d;
  mitk::PositionEvent event( m_RenderWindow->GetRenderer(), 0, 0, 0,
                             mitk::Key_unknown, p2d, pos );
  mitk::StateEvent stateEvent(mitk::EIDLEFTMOUSEBTN, &event);
  mitk::StateEvent stateEvent2(mitk::EIDLEFTMOUSERELEASE, &event);

  m_RenderWindow->GetSliceNavigationController()->HandleEvent( &stateEvent );

  // just in case SNCs will develop something that depends on the mouse
  // button being released again
  m_RenderWindow->GetSliceNavigationController()->HandleEvent( &stateEvent2 );

  // update displays
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void SimpleRenderWindowView::EnableDecorations(bool enable, const QStringList &decorations)
{
  if (decorations.isEmpty() || decorations.contains(DECORATION_MENU))
  {
    m_RenderWindow->ActivateMenuWidget(enable);
  }
}

bool SimpleRenderWindowView::IsDecorationEnabled(const QString &decoration) const
{
  if (decoration == DECORATION_MENU)
  {
    return m_RenderWindow->GetActivateMenuWidgetFlag();
  }
  return false;
}

QStringList SimpleRenderWindowView::GetDecorations() const
{
  QStringList decorations;
  decorations << DECORATION_MENU;
  return decorations;
}

void SimpleRenderWindowView::SetFocus()
{
  m_RenderWindow->setFocus();
}

void SimpleRenderWindowView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);

  m_RenderWindow = new QmitkRenderWindow(parent);
  layout->addWidget(m_RenderWindow);

  mitk::DataStorage::Pointer ds = this->GetDataStorage();
  m_RenderWindow->GetRenderer()->SetDataStorage(ds);

  // Initialize the QmitkRenderWindow as transversal to all data objects in DataStorage
//  mitk::TimeSlicedGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
//  m_RenderWindow->
//  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  this->RequestUpdate();
}

mitk::IRenderingManager* SimpleRenderWindowView::GetRenderingManager() const
{
  // we use the global rendering manager here. This should maybe replaced
  // by a local one, managing only the render windows specific for the view
  return d->m_RenderingManagerInterface;
}

void SimpleRenderWindowView::RequestUpdate(mitk::RenderingManager::RequestType requestType)
{
  if (GetRenderingManager())
    GetRenderingManager()->RequestUpdateAll(requestType);
}

void SimpleRenderWindowView::ForceImmediateUpdate(mitk::RenderingManager::RequestType requestType)
{
  if (GetRenderingManager())
    GetRenderingManager()->ForceImmediateUpdateAll(requestType);
}

mitk::SliceNavigationController* SimpleRenderWindowView::GetTimeNavigationController() const
{
  if (GetRenderingManager())
    return GetRenderingManager()->GetTimeNavigationController();
  return 0;
}
