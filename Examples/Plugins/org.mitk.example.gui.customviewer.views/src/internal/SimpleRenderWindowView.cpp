/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SimpleRenderWindowView.h"

#include <QmitkRenderWindow.h>

#include "org_mitk_example_gui_customviewer_views_Activator.h"
#include <mitkIRenderingManager.h>
#include <mitkInteractionConst.h>
#include <mitkSliceNavigationController.h>

#include <QVBoxLayout>
#include <mitkPlaneGeometry.h>

/**
 * \brief Helper class adapted from QmitkAbstractRenderEditor by defining the correct plugin context.
 *
 *  This helper class adapted from QmitkAbstractRenderEditor provides the rendering manager interface.
 */
// //! [SimpleRenderWindowViewHelper]
class AbstractRenderWindowViewPrivate
{
public:
  AbstractRenderWindowViewPrivate()
    : m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
  // //! [SimpleRenderWindowViewHelper]
  {
  }

  ~AbstractRenderWindowViewPrivate() { delete m_RenderingManagerInterface; }
  mitk::IRenderingManager *m_RenderingManagerInterface;
};

const std::string SimpleRenderWindowView::VIEW_ID = "org.mitk.customviewer.views.simplerenderwindowview";

SimpleRenderWindowView::SimpleRenderWindowView() : m_RenderWindow(nullptr), d(new AbstractRenderWindowViewPrivate)
{
}

SimpleRenderWindowView::~SimpleRenderWindowView()
{
}

QmitkRenderWindow *SimpleRenderWindowView::GetActiveQmitkRenderWindow() const
{
  return m_RenderWindow;
}

QHash<QString, QmitkRenderWindow *> SimpleRenderWindowView::GetRenderWindows() const
{
  QHash<QString, QmitkRenderWindow *> wnds;
  wnds.insert("axial", m_RenderWindow);
  return wnds;
}

QHash<QString, QmitkRenderWindow *> SimpleRenderWindowView::GetQmitkRenderWindows() const
{
  QHash<QString, QmitkRenderWindow *> wnds;
  wnds.insert("axial", m_RenderWindow);
  return wnds;
}

QmitkRenderWindow *SimpleRenderWindowView::GetRenderWindow(const QString &id) const
{
  if (id == "axial")
  {
    return m_RenderWindow;
  }
  return nullptr;
}

QmitkRenderWindow *SimpleRenderWindowView::GetQmitkRenderWindow(const QString &id) const
{
  if (id == "axial")
  {
    return m_RenderWindow;
  }
  return nullptr;
}

QmitkRenderWindow *SimpleRenderWindowView::GetQmitkRenderWindow(const mitk::AnatomicalPlane& orientation) const
{
  if (orientation == mitk::AnatomicalPlane::Axial)
  {
    return m_RenderWindow;
  }
  return 0;
}

void SimpleRenderWindowView::SetFocus()
{
  m_RenderWindow->setFocus();
}

// //! [SimpleRenderWindowViewCreatePartControl]
void SimpleRenderWindowView::CreateQtPartControl(QWidget *parent)
{
  QVBoxLayout *layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0, 0, 0, 0);

  m_RenderWindow = new QmitkRenderWindow(parent);
  layout->addWidget(m_RenderWindow);

  mitk::DataStorage::Pointer ds = this->GetDataStorage();
  m_RenderWindow->GetRenderer()->SetDataStorage(ds);

  this->RequestUpdate();
}
// //! [SimpleRenderWindowViewCreatePartControl]

mitk::IRenderingManager *SimpleRenderWindowView::GetRenderingManager() const
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

void SimpleRenderWindowView::InitializeViews(const mitk::TimeGeometry* /*geometry*/, bool /*resetCamera*/)
{
  // not implemented
}

void SimpleRenderWindowView::SetInteractionReferenceGeometry(const mitk::TimeGeometry* /*referenceGeometry*/)
{
  // not implemented
}

bool SimpleRenderWindowView::HasCoupledRenderWindows() const
{
  return false;
}

mitk::SliceNavigationController *SimpleRenderWindowView::GetTimeNavigationController() const
{
  if (GetRenderingManager())
    return GetRenderingManager()->GetTimeNavigationController();
  return nullptr;
}

mitk::Point3D SimpleRenderWindowView::GetSelectedPosition(const QString& /*id*/) const
{
  const mitk::PlaneGeometry* pg = m_RenderWindow->GetSliceNavigationController()->GetCurrentPlaneGeometry();
  if (pg)
  {
    return pg->GetCenter();
  }
  else
  {
    return mitk::Point3D();
  }
}

void SimpleRenderWindowView::SetSelectedPosition(const mitk::Point3D&, const QString&)
{
}

mitk::TimePointType SimpleRenderWindowView::GetSelectedTimePoint(const QString& /*id*/) const
{
  auto timeNavigator = this->GetTimeNavigationController();
  if (nullptr != timeNavigator)
  {
    return timeNavigator->GetSelectedTimePoint();
  }
  return 0;
}

void SimpleRenderWindowView::EnableDecorations(bool enable, const QStringList& decorations)
{
  if (decorations.isEmpty() || decorations.contains(DECORATION_MENU))
  {
    m_RenderWindow->ActivateMenuWidget(enable);
  }
}

bool SimpleRenderWindowView::IsDecorationEnabled(const QString& decoration) const
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
