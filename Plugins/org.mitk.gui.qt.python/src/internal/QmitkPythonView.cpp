/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPythonView.h"

#include "mitkNodePredicateDataType.h"
#include <QList>

const std::string QmitkPythonView::VIEW_ID = "org.mitk.views.python";

QmitkPythonView::QmitkPythonView() : m_Controls(nullptr)
{
  m_ReferencePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
  m_PythonContext = mitk::PythonContext::New();
  m_PythonContext->Activate();
}

void QmitkPythonView::CreateQtPartControl(QWidget *parent)
{
  m_Controls = new Ui::QmitkPythonViewControls;
  m_Controls->setupUi(parent);
  m_Controls->referenceNodeSelector->SetDataStorage(GetDataStorage());
  m_Controls->referenceNodeSelector->SetNodePredicate(m_ReferencePredicate);
  m_Controls->referenceNodeSelector->SetInvalidInfo("Select an image");
  m_Controls->referenceNodeSelector->SetPopUpTitel("Select an image");
  m_Controls->referenceNodeSelector->SetPopUpHint(
    "Select an image that should be used to define the geometry and bounds of the segmentation.");

  QFont monospaceFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  m_Controls->pythonConsole->setFont(monospaceFont);
  m_Controls->pythonConsole->setStyleSheet("background-color: black; color: white;");
  QFontMetrics metrics(monospaceFont); // Get font metrics of the current font
  int spaceWidth = metrics.horizontalAdvance(' ');
  m_Controls->pythonConsole->setTabStopDistance(spaceWidth * 4);
  connect(m_Controls->executeButton, SIGNAL(clicked()), this, SLOT(OnExecuteBtnClicked()));
  connect(m_Controls->referenceNodeSelector,
          &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged,
          this,
          &QmitkPythonView::OnCurrentSelectionChanged);
  connect(m_Controls->venvSelectionButton,
          SIGNAL(directoryChanged(const QString &)),
          this,
          SLOT(OnSitePackageSelected(const QString &)));
  connect(m_Controls->venvDeleteButton, SIGNAL(clicked()), this, SLOT(OnExecuteBtnClicked()));
  
  QString pythonCommand = "pyMITK.SayHi()\n";
  m_Controls->pythonConsole->setPlainText(pythonCommand);
  this->OnExecuteBtnClicked();
}

void QmitkPythonView::SetFocus()
{
  m_Controls->referenceNodeSelector->setFocus();
}

void QmitkPythonView::OnExecuteBtnClicked()
{
  QString text = m_Controls->pythonConsole->toPlainText();
  std::string pyCommand = text.toStdString();
  const char *result = m_PythonContext->ExecuteString(pyCommand);
  m_Controls->pythonOutput->clear();
  m_Controls->pythonOutput->setText(QString::fromUtf8(result));
}

void QmitkPythonView::OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes)
{
  if (nodes.size() == 0)
  {
    return;
  }
  mitk::DataNode::Pointer node = nodes.first();
  auto *image = dynamic_cast<mitk::Image *>(node->GetData());
  if (nullptr == image)
  {
    MITK_ERROR << "image was null";
    return;
  }
  m_PythonContext->TransferBaseDataToPython(image);
  const char *result = m_PythonContext->GetStdOut();
  m_Controls->pythonOutput->clear();
  m_Controls->pythonOutput->setText(QString::fromUtf8(result));
}

void QmitkPythonView::OnSitePackageSelected(const QString & /*sitePackagesFolder*/)
{
  const QString sitePackageFolder = m_Controls->venvSelectionButton->directory();
  m_Controls->venvSelectionButton->setText(sitePackageFolder);
  m_PythonContext->SetVirtualEnvironmentPath(sitePackageFolder.toStdString());
}

void QmitkPythonView::OnSitePackageDeleted()
{
  m_PythonContext->ClearVirtualEnvironmentPath();
}
