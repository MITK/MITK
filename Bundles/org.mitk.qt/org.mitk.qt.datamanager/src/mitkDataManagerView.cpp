#include "mitkDataManagerView.h"

#include <QPushButton>
#include <QDockWidget>

#include "QmitkStandardViews.h"

void* mitkDataManagerView::CreatePartControl(void* parent)
{
  QDockWidget* dock = static_cast<QDockWidget*>(parent);
  /*
  QPushButton* button = new QPushButton("Hi", static_cast<QWidget*>(parent));
  dock->setWidget(button);
  return button;
  */
  QmitkStandardViews* stdViews = new QmitkStandardViews(dock);
  //stdViews->setCameraController();
  dock->setWidget( stdViews );
  return stdViews;
}
  
void mitkDataManagerView::SetFocus()
{
  
}
