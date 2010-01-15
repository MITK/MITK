@PLUGIN_COPYRIGHT@

#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>

#include "@VIEW_CLASS_H@"

@BEGIN_NAMESPACE@

const std::string @VIEW_CLASS@::VIEW_ID = "@VIEW_ID@";

void @VIEW_CLASS@::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);
  layout->setContentsMargins(0,0,0,0);
  m_ButtonStart = new QPushButton("start", parent);
  layout->addWidget(m_ButtonStart);
}

void @VIEW_CLASS@::SetFocus()
{
  m_ButtonStart->setFocus();
}

@END_NAMESPACE@
