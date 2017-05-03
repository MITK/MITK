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

#include "QmitkPointListView.h"

#include "QmitkEditPointDialog.h"
#include "QmitkPointListModel.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderingManager.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QTimer>

QmitkPointListView::QmitkPointListView(QWidget *parent)
  : QListView(parent),
    m_PointListModel(new QmitkPointListModel()),
    m_SelfCall(false),
    m_showFading(false),
    m_MultiWidget(nullptr)
{
  QListView::setAlternatingRowColors(true);

  QListView::setSelectionBehavior(QAbstractItemView::SelectRows);
  QListView::setSelectionMode(QAbstractItemView::SingleSelection);
  QListView::setModel(m_PointListModel);
  QString tooltip = QString("Use the F2/F3 keys to move a point up/down, the Del key to remove a point\nand the mouse "
                            "wheel to change the timestep.\n\nTimeStep:\t%1")
                      .arg(0);
  QListView::setToolTip(tooltip);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  m_TimeStepFaderLabel = new QLabel(this);
  QFont font("Arial", 17);
  m_TimeStepFaderLabel->setFont(font);

  this->setMinimumHeight(40);

  this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  connect(m_PointListModel, SIGNAL(SignalUpdateSelection()), this, SLOT(OnPointSetSelectionChanged()));

  connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnPointDoubleClicked(const QModelIndex &)));

  connect(QListView::selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,
          SLOT(OnListViewSelectionChanged(const QItemSelection &, const QItemSelection &)));

  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));
}

QmitkPointListView::~QmitkPointListView()
{
  delete m_PointListModel;
}

void QmitkPointListView::SetPointSetNode(mitk::DataNode *pointSetNode)
{
  m_PointListModel->SetPointSetNode(pointSetNode);
}

const mitk::PointSet *QmitkPointListView::GetPointSet() const
{
  return m_PointListModel->GetPointSet();
}

void QmitkPointListView::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
  m_MultiWidget = multiWidget;
  this->AddSliceNavigationController(multiWidget->mitkWidget1->GetSliceNavigationController());
  this->AddSliceNavigationController(multiWidget->mitkWidget2->GetSliceNavigationController());
  this->AddSliceNavigationController(multiWidget->mitkWidget3->GetSliceNavigationController());
}

QmitkStdMultiWidget *QmitkPointListView::GetMultiWidget() const
{
  return m_MultiWidget;
}

void QmitkPointListView::OnPointDoubleClicked(const QModelIndex &index)
{
  mitk::PointSet::PointType p;
  mitk::PointSet::PointIdentifier id;
  m_PointListModel->GetPointForModelIndex(index, p, id);
  QmitkEditPointDialog _EditPointDialog(this);
  _EditPointDialog.SetPoint(m_PointListModel->GetPointSet(), id, m_PointListModel->GetTimeStep());
  _EditPointDialog.exec();
}

void QmitkPointListView::OnPointSetSelectionChanged()
{
  const mitk::PointSet *pointSet = m_PointListModel->GetPointSet();
  if (pointSet == nullptr)
    return;

  // update this view's selection status as a result to changes in the point set data structure
  m_SelfCall = true;
  int timeStep = m_PointListModel->GetTimeStep();

  if (pointSet->GetNumberOfSelected(timeStep) > 1)
  {
    MITK_ERROR << "Point set has multiple selected points. This view is not designed for more than one selected point.";
  }

  int selectedIndex = pointSet->SearchSelectedPoint(timeStep);

  if (selectedIndex == -1) // no selected point is found
  {
    m_SelfCall = false;
    return;
  }

  QModelIndex index;

  bool modelIndexOkay = m_PointListModel->GetModelIndexForPointID(selectedIndex, index);

  if (modelIndexOkay == true)
    QListView::selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

  emit SignalPointSelectionChanged();

  m_SelfCall = false;
}

void QmitkPointListView::OnListViewSelectionChanged(const QItemSelection &selected,
                                                    const QItemSelection & /*deselected*/)
{
  if (m_SelfCall)
    return;

  mitk::PointSet *pointSet = const_cast<mitk::PointSet *>(m_PointListModel->GetPointSet());

  if (pointSet == nullptr)
    return;

  // (take care that this widget doesn't react to self-induced changes by setting m_SelfCall)
  m_SelfCall = true;

  // update selection of all points in pointset: select the one(s) that are selected in the view, deselect all others
  QModelIndexList selectedIndexes = selected.indexes();

  for (mitk::PointSet::PointsContainer::Iterator it =
         pointSet->GetPointSet(m_PointListModel->GetTimeStep())->GetPoints()->Begin();
       it != pointSet->GetPointSet(m_PointListModel->GetTimeStep())->GetPoints()->End();
       ++it)
  {
    QModelIndex index;
    if (m_PointListModel->GetModelIndexForPointID(it->Index(), index))
    {
      if (selectedIndexes.indexOf(index) != -1) // index is found in the selected indices list
      {
        pointSet->SetSelectInfo(it->Index(), true, m_PointListModel->GetTimeStep());

        mitk::Point3D p = pointSet->GetPoint(it->Index(), m_PointListModel->GetTimeStep());

        for (auto snc : m_Sncs)
          snc->SelectSliceByPoint(p);
      }
      else
      {
        pointSet->SetSelectInfo(it->Index(), false, m_PointListModel->GetTimeStep());
      }
    }
  }

  m_SelfCall = false;

  emit SignalPointSelectionChanged();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPointListView::keyPressEvent(QKeyEvent *e)
{
  if (m_PointListModel == nullptr)
    return;

  int key = e->key();

  switch (key)
  {
    case Qt::Key_F2:
      m_PointListModel->MoveSelectedPointUp();
      break;
    case Qt::Key_F3:
      m_PointListModel->MoveSelectedPointDown();
      break;
    case Qt::Key_Delete:
      m_PointListModel->RemoveSelectedPoint();
      break;
    default:
      break;
  }
}

void QmitkPointListView::wheelEvent(QWheelEvent *event)
{
  if (!m_PointListModel || !m_PointListModel->GetPointSet() ||
      (int)(m_PointListModel->GetPointSet()->GetTimeSteps()) == 1)
    return;

  int whe = event->delta();
  mitk::PointSet::Pointer ps = dynamic_cast<mitk::PointSet *>(m_PointListModel->GetPointSet());
  unsigned int numberOfTS = ps->GetTimeSteps();

  if (numberOfTS == 1)
    return;
  int currentTS = this->m_PointListModel->GetTimeStep();
  if (whe > 0)
  {
    if ((currentTS + 1 >= (int)numberOfTS))
      return;

    this->m_PointListModel->SetTimeStep(++currentTS);
  }
  else
  {
    if ((currentTS <= 0))
      return;
    this->m_PointListModel->SetTimeStep(--currentTS);
  }

  QString tooltip = QString("Use the F2/F3 keys to move a point up/down, the Del key to remove a point\nand the mouse "
                            "wheel to change the timestep.\n\nTimeStep:\t%1")
                      .arg(currentTS);
  this->setToolTip(tooltip);

  fadeTimeStepIn();
}

void QmitkPointListView::fadeTimeStepIn()
{
  // Setup Widget
  QWidget *m_TimeStepFader = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(m_TimeStepFader);

  int x = (int)(this->geometry().x() + this->width() * 0.6);
  int y = (int)(this->geometry().y() + this->height() * 0.8);
  m_TimeStepFader->move(x, y);
  m_TimeStepFader->resize(60, 55);
  m_TimeStepFader->setLayout(layout);
  m_TimeStepFader->setAttribute(Qt::WA_DeleteOnClose);

  layout->addWidget(m_TimeStepFaderLabel);
  m_TimeStepFaderLabel->setAlignment(Qt::AlignCenter);
  m_TimeStepFaderLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
  m_TimeStepFaderLabel->setLineWidth(2);
  m_TimeStepFaderLabel->setText(QString("%1").arg(this->m_PointListModel->GetTimeStep()));

  // give the widget opacity and some colour
  QPalette pal = m_TimeStepFaderLabel->palette();
  QColor semiTransparentColor(139, 192, 223, 50);
  QColor labelTransparentColor(0, 0, 0, 200);
  pal.setColor(m_TimeStepFaderLabel->backgroundRole(), semiTransparentColor);
  pal.setColor(m_TimeStepFaderLabel->foregroundRole(), labelTransparentColor);
  m_TimeStepFaderLabel->setAutoFillBackground(true);
  m_TimeStepFaderLabel->setPalette(pal);

  // show the widget
  m_TimeStepFader->show();

  // and start the timer
  m_TimeStepFaderLabel->setVisible(true);
  QTimer::singleShot(2000, this, SLOT(fadeTimeStepOut()));
}

void QmitkPointListView::fadeTimeStepOut()
{
  m_TimeStepFaderLabel->hide();
}

void QmitkPointListView::ctxMenu(const QPoint &pos)
{
  QMenu *menu = new QMenu;

  // add Fading check
  QAction *showFading = new QAction(this);
  showFading->setCheckable(false); // TODO: reset when fading is working
  showFading->setEnabled(false);   // TODO: reset when fading is working
  showFading->setText("Fade TimeStep");
  connect(showFading, SIGNAL(triggered(bool)), this, SLOT(SetFading(bool)));
  menu->addAction(showFading);

  // add Clear action
  QAction *clearList = new QAction(this);
  clearList->setText("Clear List");
  connect(clearList, SIGNAL(triggered()), this, SLOT(ClearPointList()));
  menu->addAction(clearList);

  // add Clear TimeStep action
  QAction *clearTS = new QAction(this);
  clearTS->setText("Clear current time step");
  connect(clearTS, SIGNAL(triggered()), this, SLOT(ClearPointListTS()));
  menu->addAction(clearTS);

  menu->exec(this->mapToGlobal(pos));
}

void QmitkPointListView::SetFading(bool onOff)
{
  m_showFading = onOff;
}

void QmitkPointListView::ClearPointList()
{
  if (!m_PointListModel->GetPointSet())
    return;
  mitk::PointSet::Pointer curPS = m_PointListModel->GetPointSet();
  if (curPS->GetSize() == 0)
    return;

  switch (QMessageBox::question(this,
                                tr("Clear Points"),
                                tr("Remove all points from the displayed list?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No))
  {
    case QMessageBox::Yes:
    {
      mitk::PointSet::PointsIterator it;
      mitk::PointSet::PointsContainer *curPsPoints;
      while (!curPS->IsEmptyTimeStep(0))
      {
        curPsPoints = curPS->GetPointSet()->GetPoints();
        it = curPsPoints->Begin();
        curPS->SetSelectInfo(it->Index(), true);
        m_PointListModel->RemoveSelectedPoint();
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
    case QMessageBox::No:
    default:
      break;
  }
}

void QmitkPointListView::ClearPointListTS()
{
}

void QmitkPointListView::AddSliceNavigationController(mitk::SliceNavigationController *snc)
{
  if (snc == nullptr)
    return;
  m_Sncs.insert(snc);
}

void QmitkPointListView::RemoveSliceNavigationController(mitk::SliceNavigationController *snc)
{
  if (snc == nullptr)
    return;
  m_Sncs.erase(snc);
}
