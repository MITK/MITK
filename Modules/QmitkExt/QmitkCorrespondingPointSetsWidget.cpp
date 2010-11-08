#include "QmitkCorrespondingPointSetsWidget.h"
#include <mitkGlobalInteraction.h>
#include <mitkPointSetReader.h>
#include <mitkPointSetWriter.h>

#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include "btnLoad.xpm"
#include "btnSave.xpm"
#include "btnClear.xpm"
#include "btnSetPoints.xpm"
#include "btnUp.xpm"
#include "btnDown.xpm"



QmitkCorrespondingPointSetsWidget::QmitkCorrespondingPointSetsWidget(QWidget *parent):
  QWidget(parent), 
  m_CorrespondingPointSetsView(NULL)
{
  m_CorrespondingPointSetsView = new QmitkCorrespondingPointSetsView();

  SetupUi();
}

QmitkCorrespondingPointSetsWidget::~QmitkCorrespondingPointSetsWidget()
{
  delete m_CorrespondingPointSetsView;
}


void QmitkCorrespondingPointSetsWidget::SetupUi()
{
    QBoxLayout* lay1 = new QVBoxLayout(this);

    lay1->insertWidget(0,m_CorrespondingPointSetsView);
    this->setLayout(lay1);
}
void QmitkCorrespondingPointSetsWidget::SetPointSetNodes(std::vector<mitk::DataNode*> nodes)
{
  this->m_CorrespondingPointSetsView->SetPointSetNodes(nodes);
}
std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsWidget::GetPointSetNodes()
{
  return this->m_CorrespondingPointSetsView->GetPointSetNodes();
}
void QmitkCorrespondingPointSetsWidget::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
    m_CorrespondingPointSetsView->SetMultiWidget(multiWidget);
}
void QmitkCorrespondingPointSetsWidget::UpdateSelection(mitk::DataNode* selectedNode)
{
  m_CorrespondingPointSetsView->UpdateSelection(selectedNode);
}