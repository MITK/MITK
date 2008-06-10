#include "mitkDataManagerView.h"

#include <QPushButton>
#include <QDockWidget>

void* mitkDataManagerView::CreatePartControl(void* parent)
{
  QDockWidget* dock = static_cast<QDockWidget*>(parent);
  QPushButton* button = new QPushButton("Hi", static_cast<QWidget*>(parent));
  dock->setWidget(button);
  return button;
}
  
void mitkDataManagerView::SetFocus()
{
  
}
