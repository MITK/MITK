/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPointListViewWidget.h"

#include "QmitkEditPointDialog.h"
#include "QmitkPointListModel.h"
#include "QmitkStdMultiWidget.h"

#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"

#include <QKeyEvent>

QmitkPointListViewWidget::QmitkPointListViewWidget(QWidget *parent)
  : QListWidget(parent), m_TimeStep(0), m_SelfCall(false), m_MultiWidget(nullptr)
{
  QListWidget::setAlternatingRowColors(true);
  // logic

  QListWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
  QListWidget::setSelectionMode(QAbstractItemView::SingleSelection);

  connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnItemDoubleClicked(QListWidgetItem *)));

  connect(this, SIGNAL(currentRowChanged(int)), this, SLOT(OnCurrentRowChanged(int)));
}

QmitkPointListViewWidget::~QmitkPointListViewWidget()
{
  this->SetPointSet(nullptr); // remove listener
}

void QmitkPointListViewWidget::SetPointSet(mitk::PointSet *pointSet)
{
  if (!m_PointSet.IsExpired())
  {
    auto pointSet = m_PointSet.Lock();

    pointSet->RemoveObserver(m_PointSetModifiedTag);
    pointSet->RemoveObserver(m_PointSetDeletedTag);
  }

  m_PointSet = pointSet;

  if (!m_PointSet.IsExpired())
  {
    auto pointSet = m_PointSet.Lock();

    auto onPointSetDeleted = itk::SimpleMemberCommand<QmitkPointListViewWidget>::New();
    onPointSetDeleted->SetCallbackFunction(this, &QmitkPointListViewWidget::OnPointSetDeleted);
    m_PointSetDeletedTag = pointSet->AddObserver(itk::DeleteEvent(), onPointSetDeleted);

    auto onPointSetModified = itk::SimpleMemberCommand<QmitkPointListViewWidget>::New();
    onPointSetModified->SetCallbackFunction(this, &QmitkPointListViewWidget::OnPointSetChanged);
    m_PointSetModifiedTag = pointSet->AddObserver(itk::DeleteEvent(), onPointSetModified);
  }

  this->Update();
}

const mitk::PointSet *QmitkPointListViewWidget::GetPointSet() const
{
  return m_PointSet.Lock();
}

void QmitkPointListViewWidget::SetTimeStep(int t)
{
  m_TimeStep = t;
  this->Update();
}

int QmitkPointListViewWidget::GetTimeStep() const
{
  return m_TimeStep;
}

void QmitkPointListViewWidget::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
  m_MultiWidget = multiWidget;
}

QmitkStdMultiWidget *QmitkPointListViewWidget::GetMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkPointListViewWidget::OnPointSetChanged()
{
  if (!m_SelfCall)
    this->Update();
}

void QmitkPointListViewWidget::OnPointSetDeleted()
{
  this->SetPointSet(nullptr);
  this->Update();
}

void QmitkPointListViewWidget::OnItemDoubleClicked(QListWidgetItem *item)
{
  QmitkEditPointDialog _EditPointDialog(this);
  _EditPointDialog.SetPoint(m_PointSet.Lock(), this->row(item), m_TimeStep);
  _EditPointDialog.exec();
}

void QmitkPointListViewWidget::OnCurrentRowChanged(int)
{
  this->Update(true);
}

void QmitkPointListViewWidget::keyPressEvent(QKeyEvent *e)
{
  if (m_PointSet.IsExpired())
    return;

  int key = e->key();
  switch (key)
  {
    case Qt::Key_F2:
      this->MoveSelectedPointUp();
      break;
    case Qt::Key_F3:
      this->MoveSelectedPointDown();
      break;
    case Qt::Key_Delete:
      this->RemoveSelectedPoint();
      break;
    default:
      break;
  }
}

void QmitkPointListViewWidget::MoveSelectedPointUp()
{
  if (m_PointSet.IsExpired())
    return;

  auto pointSet = m_PointSet.Lock();

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation *doOp =
    new mitk::PointOperation(mitk::OpMOVEPOINTUP, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListViewWidget::MoveSelectedPointDown()
{
  if (m_PointSet.IsExpired())
    return;

  auto pointSet = m_PointSet.Lock();

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation *doOp =
    new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListViewWidget::RemoveSelectedPoint()
{
  if (m_PointSet.IsExpired())
    return;

  auto pointSet = m_PointSet.Lock();

  mitk::PointSet::PointIdentifier selectedID;
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointOperation *doOp =
    new mitk::PointOperation(mitk::OpREMOVE, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkPointListViewWidget::Update(bool currentRowChanged)
{
  if (m_SelfCall)
    return;

  if (m_PointSet.IsExpired())
  {
    this->clear();
    return;
  }

  auto pointSet = m_PointSet.Lock();

  m_SelfCall = true;
  QString text;
  int i = 0;

  mitk::PointSet::DataType::Pointer pointset = pointSet->GetPointSet(m_TimeStep);
  for (mitk::PointSet::PointsContainer::Iterator it = pointset->GetPoints()->Begin();
       it != pointset->GetPoints()->End();
       ++it)
  {
    text = QString("%0: (%1, %2, %3)")
             .arg(i, 3)
             .arg(it.Value().GetElement(0), 0, 'f', 3)
             .arg(it.Value().GetElement(1), 0, 'f', 3)
             .arg(it.Value().GetElement(2), 0, 'f', 3);

    if (i == this->count())
      this->addItem(text); // insert text
    else
      this->item(i)->setText(text); // update text

    if (currentRowChanged)
    {
      if (i == this->currentRow())
        pointSet->SetSelectInfo(this->currentRow(), true, m_TimeStep);
      else
        pointSet->SetSelectInfo(it->Index(), false, m_TimeStep); // select nothing now
    }
    ++i;
  }

  // remove unnecessary listwidgetitems
  while (pointSet->GetPointSet(m_TimeStep)->GetPoints()->Size() < (unsigned int)this->count())
  {
    QListWidgetItem *item = this->takeItem(this->count() - 1);
    delete item;
  }

  // update selection in pointset or in the list widget
  if (!currentRowChanged)
  {
    if (pointSet->GetNumberOfSelected(m_TimeStep) > 1)
    {
      /// @TODO use logging as soon as available
      std::cerr << "Point set has multiple selected points. This view is not designed for more than one selected point."
                << std::endl;
    }

    int selectedIndex = pointSet->SearchSelectedPoint(m_TimeStep);
    if (selectedIndex != -1) // no selected point is found
    {
      this->setCurrentRow(selectedIndex);
    }
  }
  m_SelfCall = false;
}
