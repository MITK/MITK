/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkNodeSelectionButton.h>

// mitk qt widgets module
#include <QmitkNodeDescriptorManager.h>

#include <QApplication>
#include <QEvent>
#include <QPainter>
#include <QTextDocument>

QmitkNodeSelectionButton::QmitkNodeSelectionButton(QWidget *parent)
  : QPushButton(parent)
  , m_OutDatedThumbNail(true)
  , m_DataMTime(0)
  , m_IsOptional(true)
  , m_NodeModifiedObserverTag(0)
  , m_NodeObserved(false)
{
}

QmitkNodeSelectionButton::~QmitkNodeSelectionButton()
{
  this->RemoveNodeObserver();
  this->m_SelectedNode = nullptr;
}

void QmitkNodeSelectionButton::AddNodeObserver()
{
  if (this->m_SelectedNode.IsNotNull())
  {
    if (m_NodeObserved)
    {
      MITK_DEBUG << "Invalid observer state in QmitkNodeSelectionButton. There is already a registered observer. Internal logic is not correct. May be an old observer was not removed.";
    }

    auto modifiedCommand = itk::MemberCommand<QmitkNodeSelectionButton>::New();
    modifiedCommand->SetCallbackFunction(this, &QmitkNodeSelectionButton::OnNodeModified);

    // const cast because we need non const nodes and it seems to be the lesser of two evil.
    // the changes to the node are only on the observer level. The other option would be to
    // make the public interface require non const nodes, this we don't want to introduce.
    auto nonconst_node = const_cast<mitk::DataNode*>(this->m_SelectedNode.GetPointer());
    m_NodeModifiedObserverTag = nonconst_node->AddObserver(itk::ModifiedEvent(), modifiedCommand);
    m_NodeObserved = true;
  }
}

void QmitkNodeSelectionButton::RemoveNodeObserver()
{
  if (this->m_SelectedNode.IsNotNull())
  {
    // const cast because we need non const nodes and it seems to be the lesser of two evil.
    // the changes to the node are only on the observer level. The other option would be to
    // make the public interface require non const nodes, this we don't want to introduce.
    auto nonconst_node = const_cast<mitk::DataNode*>(this->m_SelectedNode.GetPointer());
    nonconst_node->RemoveObserver(m_NodeModifiedObserverTag);
  }
  m_NodeObserved = false;
}

void QmitkNodeSelectionButton::OnNodeModified(const itk::Object * /*caller*/, const itk::EventObject & event)
{
  if (itk::ModifiedEvent().CheckEvent(&event))
  {
    this->update();
  }
}

const mitk::DataNode* QmitkNodeSelectionButton::GetSelectedNode() const
{
  return m_SelectedNode;
}

void QmitkNodeSelectionButton::SetSelectedNode(const mitk::DataNode* node)
{
  if (m_SelectedNode != node)
  {
    this->RemoveNodeObserver();
    this->m_SelectedNode = node;
    this->m_OutDatedThumbNail = true;
    this->AddNodeObserver();
  }

  this->update();
}

void QmitkNodeSelectionButton::SetNodeInfo(QString info)
{
  this->m_Info = info;
  this->update();
}

void QmitkNodeSelectionButton::paintEvent(QPaintEvent *p)
{
  QPushButton::paintEvent(p);

  auto styleSheet = qApp->styleSheet();

  QPainter painter(this);
  QTextDocument td(this);
  td.setDefaultStyleSheet(styleSheet);

  auto widgetSize = this->size();
  QPoint origin = QPoint(5, 5);

  if (this->m_SelectedNode)
  {
    auto iconLength = widgetSize.height() - 10;
    auto node = this->m_SelectedNode;

    itk::ModifiedTimeType dataMTime = 0;
    if (m_SelectedNode->GetData())
    {
      dataMTime = m_SelectedNode->GetData()->GetMTime();
    }
    if (dataMTime>m_DataMTime || this->m_OutDatedThumbNail)
    {
      auto descriptor = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(node);
      this->m_ThumbNail = descriptor->GenerateThumbnail(node, iconLength);
      this->m_OutDatedThumbNail = false;
      m_DataMTime = dataMTime;
    }

    auto thumbNailOrigin = origin;
    thumbNailOrigin.setY(thumbNailOrigin.y() + ((iconLength - m_ThumbNail.height()) / 2));
    painter.drawPixmap(thumbNailOrigin, m_ThumbNail);
    origin.setX(origin.x() + iconLength + 5);

    if (this->isEnabled())
    {
      td.setHtml(QString::fromStdString("<font class=\"normal\">" + node->GetName() + "</font>"));
    }
    else
    {
      td.setHtml(QString::fromStdString("<font class=\"disabled\">" + node->GetName() + "</font>"));
    }
  }
  else
  {
    if (this->isEnabled())
    {
      if (this->m_IsOptional)
      {
        td.setHtml(QString("<font class=\"normal\">") + m_Info + QString("</font>"));
      }
      else
      {
        td.setHtml(QString("<font class=\"warning\">") + m_Info + QString("</font>"));
      }
    }
    else
    {
      td.setHtml(QString("<font class=\"disabled\">") + m_Info + QString("</font>"));
    }
  }

  auto textSize = td.size();

  origin.setY( (widgetSize.height() - textSize.height()) / 2.);

  painter.translate(origin);
  td.drawContents(&painter);
}

void QmitkNodeSelectionButton::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::EnabledChange)
  {
    this->update();
  }
}

bool QmitkNodeSelectionButton::GetSelectionIsOptional() const
{
  return m_IsOptional;
}

void QmitkNodeSelectionButton::SetSelectionIsOptional(bool isOptional)
{
  m_IsOptional = isOptional;
  this->update();
}
