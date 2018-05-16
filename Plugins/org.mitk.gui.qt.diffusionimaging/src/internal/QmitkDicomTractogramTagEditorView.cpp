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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIStructuredSelection.h>

// Qmitk
#include "QmitkDicomTractogramTagEditorView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

// MITK
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateProperty.h>

const std::string QmitkDicomTractogramTagEditorView::VIEW_ID = "org.mitk.views.dicomtractogramtageditor";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

QmitkDicomTractogramTagEditorView::QmitkDicomTractogramTagEditorView()
  : m_Controls(nullptr)
{
  m_TagList = {"DICOM.patient_id",
               "DICOM.patient_name",
               "DICOM.study_instance_uid",
               "DICOM.series_instance_uid",
               "DICOM.sop_instance_uid",
               "DICOM.frame_of_reference_uid",
              "DICOM.algo_code.value",
              "DICOM.algo_code.meaning",
              "DICOM.model_code.value",
              "DICOM.model_code.meaning",
              "DICOM.anatomy.value",
              "DICOM.anatomy.meaning"};
  m_ImageTagList = {"DICOM.patient_id",
               "DICOM.patient_name",
               "DICOM.study_instance_uid",
               "DICOM.series_instance_uid",
               "DICOM.sop_instance_uid",
               "DICOM.frame_of_reference_uid"};
}

// Destructor
QmitkDicomTractogramTagEditorView::~QmitkDicomTractogramTagEditorView()
{
}

void QmitkDicomTractogramTagEditorView::CreateQtPartControl( QWidget *parent )
{
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDicomTractogramTagEditorViewControls;
    m_Controls->setupUi( parent );

    m_Controls->m_TractogramBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFibPredicate = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();

    m_Controls->m_TractogramBox->SetPredicate( isFibPredicate );
    m_Controls->m_TractogramBox->SetZeroEntryText("--");

    m_Controls->m_TagTable->setColumnCount(2);
    m_Controls->m_TagTable->setRowCount(m_TagList.size());
    m_Controls->m_TagTable->verticalHeader()->setVisible(false);
    m_Controls->m_TagTable->setSelectionMode(QAbstractItemView::SingleSelection);
    QStringList tableHeader;
    tableHeader <<"Tag"<<"Value";
    m_Controls->m_TagTable->setHorizontalHeaderLabels(tableHeader);
    m_Controls->m_TagTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_Controls->m_CopyPropsButton->setEnabled(false);

    connect( (QObject*)(m_Controls->m_TractogramBox), SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnTractSelectionChanged()) );
    connect( (QObject*)(m_Controls->m_TagTable), SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(OnItemChanged(QTableWidgetItem*)) );
    connect( m_Controls->m_CopyPropsButton, SIGNAL(clicked()), this, SLOT(CopyProperties()));

    UpdateGui();
  }
}

void QmitkDicomTractogramTagEditorView::CopyProperties()
{
  if (m_Controls->m_TractogramBox->GetSelectedNode()==nullptr)
    return;
  mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractogramBox->GetSelectedNode()->GetData());
  if (fib.IsNull())
    return;
  if (m_Image.IsNull())
    return;

  mitk::PropertyList* source_p_list = m_Image->GetPropertyList();
  mitk::PropertyList* target_p_list = fib->GetPropertyList();

  for (std::string tag : m_ImageTagList)
  {
    std::string val = "-";
    source_p_list->GetStringProperty(tag.c_str(), val);
    target_p_list->SetStringProperty(tag.c_str(), val.c_str());
  }
  UpdateGui();
}

void QmitkDicomTractogramTagEditorView::OnItemChanged(QTableWidgetItem* item)
{
  if (m_Controls->m_TractogramBox->GetSelectedNode()==nullptr)
  {
    UpdateGui();
    return;
  }

  int col = item->column();
  if (col==1)
  {
    std::string tag = m_TagList.at(item->row());

    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractogramBox->GetSelectedNode()->GetData());
    mitk::PropertyList* p_list = fib->GetPropertyList();

    p_list->SetStringProperty(tag.c_str(), item->text().toStdString().c_str());
  }
  UpdateGui();
}

void QmitkDicomTractogramTagEditorView::OnTractSelectionChanged()
{
  UpdateGui();
}

void QmitkDicomTractogramTagEditorView::UpdateGui()
{
  m_Controls->m_CopyPropsButton->setEnabled(false);
  m_Controls->m_TagTable->blockSignals(true);
  if (m_Controls->m_TractogramBox->GetSelectedNode()==nullptr)
  {
    int row = 0;
    for (std::string tag : m_TagList)
    {
      m_Controls->m_TagTable->setItem(row, 0, new QTableWidgetItem(tag.c_str()));
      m_Controls->m_TagTable->setItem(row, 1, new QTableWidgetItem("-"));
      ++row;
    }
    m_Controls->m_TagTable->blockSignals(false);
    return;
  }

  mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_TractogramBox->GetSelectedNode()->GetData());
  mitk::PropertyList* p_list = fib->GetPropertyList();

  int row = 0;
  for (std::string tag : m_TagList)
  {
    std::string val = "-";
    p_list->GetStringProperty(tag.c_str(), val);
    m_Controls->m_TagTable->setItem(row, 0, new QTableWidgetItem(tag.c_str()));
    m_Controls->m_TagTable->setItem(row, 1, new QTableWidgetItem(val.c_str()));
    ++row;
  }

  if (m_Image.IsNotNull())
    m_Controls->m_CopyPropsButton->setEnabled(true);
  m_Controls->m_TagTable->blockSignals(false);
}

void QmitkDicomTractogramTagEditorView::OnSelectionChanged( berry::IWorkbenchPart::Pointer , const QList<mitk::DataNode::Pointer>& nodes)
{
  m_Image = nullptr;
  for (auto node : nodes)
  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(node->GetData());
    if (img.IsNotNull())
    {
      m_Image = img;
    }
  }
  UpdateGui();
}

void QmitkDicomTractogramTagEditorView::SetFocus()
{

}
