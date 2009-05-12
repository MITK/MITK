/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkSaveProjectWidget.h"
#include <QmitkDataTreeListView.h>

#include <mitkDataTreeFilterFunctions.h>
#include <mitkXMLWriter.h>

#include <qlayout.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtoolbutton.h>


QmitkSaveProjectWidget::QmitkSaveProjectWidget(mitk::DataTree::Pointer dataTree, QWidget* parent, const char* name)
:QWidget(parent,name), m_DataTree(dataTree)
{ 
  PrepareDataTreeFilter();
  SetLayout();  
  AddConnections();
}

void QmitkSaveProjectWidget::PrepareDataTreeFilter()
{
  visible_props.push_back("write");
  visible_props.push_back("name");

  editable_props.push_back("write");
  editable_props.push_back("name");
  
  tree_filter = mitk::DataTreeFilter::New(m_DataTree);
  tree_filter->SetFilter(mitk::IsDataTreeNode());

  const mitk::DataTreeFilter::ItemList* items;  
  items = tree_filter->GetItems();
  AddCheckBoxWrite(items);

  //tree_filter->SetFilter(&mitk::IsGoodDataTreeNode);  // filters all nodes without a name
  tree_filter->SetVisibleProperties(visible_props);
  tree_filter->SetEditableProperties(editable_props);          
}

void QmitkSaveProjectWidget::SetLayout()
{
  QVBoxLayout* vertical = new QVBoxLayout(this, QBoxLayout::TopToBottom);
  QHBoxLayout* horizontal = new QHBoxLayout();
  QHBoxLayout* pushButtonsHorizontal = new QHBoxLayout();

  sourceCheckBox = new QCheckBox("Save Source Files", this);
  sourceCheckBox->setChecked(true);
  vertical->addWidget(sourceCheckBox);

  folderChooseButton = new QToolButton(this);
  folderChooseButton->setMinimumWidth(20);
  folderChooseButton->setMinimumHeight(20);
  folderChooseButton->setText("...");
  folderChooseButton->setTextLabel("Choose a folder to store the source files");
  horizontal->addWidget(folderChooseButton);

  folderLineEdit = new QLineEdit(this);
  horizontal->addWidget(folderLineEdit);

  vertical->insertLayout(1, horizontal);

  treelistview = new QmitkDataTreeListView(tree_filter, this);
  vertical->addWidget( treelistview );

  saveButton = new QPushButton("&Save", this);
  saveButton->setDefault(true);
  pushButtonsHorizontal->addWidget(saveButton);

  cancelButton = new QPushButton("&Cancel", this);
  pushButtonsHorizontal->addWidget(cancelButton);

  vertical->insertLayout(-1, pushButtonsHorizontal);
}

void QmitkSaveProjectWidget::AddConnections()
{
  QObject::connect(saveButton, SIGNAL(clicked()), this, SLOT(Save()));
  QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(Cancel()));
  QObject::connect(folderChooseButton, SIGNAL(clicked()), this, SLOT(ChooseSourceFolder()));
  QObject::connect(sourceCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SetEditable(int)));
}

void QmitkSaveProjectWidget::Save()
{
  // save dialog
  QString filename = QFileDialog::getSaveFileName( QString::null, "XML Project description (*.xml)", this,
                                                   "Save Project File", "Choose a filename to save under");
  if ( !filename.isEmpty() ) {
      TryToExportTree(filename);
      //std::cout<<filename.ascii()<<std::endl;
      this->close();
  }
}

void QmitkSaveProjectWidget::Cancel()
{
  this->close();
}

void QmitkSaveProjectWidget::ChooseSourceFolder()
{
  // folder dialog
  QString filename = QFileDialog::getExistingDirectory(QString::null, this, "get existing directory",
                                                       "Choose the source directory", FALSE );
  if ( !filename.isEmpty() )
      folderLineEdit->setText(filename);
      //std::cout<<filename.ascii()<<std::endl;
}

void QmitkSaveProjectWidget::SetEditable(int state)
{
  if(state == QButton::On){
    folderChooseButton->setDisabled(false);
    folderLineEdit->setDisabled(false);
    editable_props.push_back("write");
    editable_props.push_back("name");
    tree_filter->SetEditableProperties(editable_props);
  }
  if(state == QButton::Off){
    //editable_props.clear();
    editable_props.pop_back();
    tree_filter->SetEditableProperties(editable_props);
    folderChooseButton->setDisabled(true);
    folderLineEdit->setDisabled(true);
  }
}

void QmitkSaveProjectWidget::WriteSelectedItems(const mitk::DataTreeFilter::ItemList* items,
                                          mitk::XMLWriter& xmlWriter)
{
  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 

  while ( itemiter != itemiterend ) // for all items
  {
    // check, whether item is selected    
    const mitk::DataTreeNode* node = itemiter->GetNode();
    if (node){
      mitk::BaseProperty::Pointer property = node->GetProperty("write");
      std::string m_bool = property->GetValueAsString();
      if (m_bool == "1")
      {
        xmlWriter.BeginNode(mitk::DataTree::XML_TAG_TREE_NODE);

        // writes node via XMLWriter
        // sets the source file name

        mitk::BaseProperty::Pointer m_Property = node->GetProperty("name");
        std::string sourceFileName;
        if (m_Property){
          sourceFileName = m_Property->GetValueAsString();
          if (sourceFileName != "") 
            sourceFileName = ReplaceWhiteSpaces(sourceFileName);
        }
        xmlWriter.SetSourceFileName(sourceFileName.c_str());      
        const_cast<mitk::DataTreeNode*>(node)->WriteXML( xmlWriter );
      }
    }
    // write children
    if ( itemiter->HasChildren() )
    {
      WriteSelectedItems( itemiter->GetChildren(), xmlWriter );
    }

    // close XML node
    if (node){
      mitk::BaseProperty::Pointer property = node->GetProperty("write");
      std::string m_bool = property->GetValueAsString();
      if (m_bool == "1")
        xmlWriter.EndNode();
    }
    ++itemiter; // next item
  }
}

void QmitkSaveProjectWidget::TryToExportTree(QString filename) 
{
  const mitk::DataTreeFilter::ItemList* items;
  
  items = tree_filter->GetItems();
  
  mitk::XMLWriter xmlWriter(filename.ascii());

  xmlWriter.SetSaveSourceFiles(sourceCheckBox->isChecked());
  if(folderLineEdit->isEnabled())
    xmlWriter.SetSubFolder(folderLineEdit->text());
  xmlWriter.BeginNode(mitk::DataTree::XML_NODE_NAME);
  WriteSelectedItems(items, xmlWriter);
  xmlWriter.EndNode();
}

QmitkSaveProjectWidget::~QmitkSaveProjectWidget() 
{
  delete sourceCheckBox;
  delete folderChooseButton;
  delete folderLineEdit;
  delete saveButton;
  delete treelistview;
  
  tree_filter = 0;
  m_DataTree = 0;
}

std::string QmitkSaveProjectWidget::ReplaceWhiteSpaces(std::string string) 
{
  std::string::size_type pos = string.find(" ");
  while (pos != string.npos){
    string.replace(pos, 1, "_");
    pos = string.find(" ");
  }  
  //int e0 = string.find(".");
  //return string.substr(0, e0).c_str();
  return string;
}

void QmitkSaveProjectWidget::AddCheckBoxWrite(const mitk::DataTreeFilter::ItemList* items)
{
  mitk::DataTreeFilter::ConstItemIterator itemiter( items->Begin() ); 
  mitk::DataTreeFilter::ConstItemIterator itemiterend( items->End() ); 

  while ( itemiter != itemiterend ) // for all items
  {
    mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>(itemiter->GetNode());
    if (node) // could be, that node is NULL
      node->SetProperty("write", mitk::BoolProperty::New(true));
     
    if ( itemiter->HasChildren() )
    {
      AddCheckBoxWrite( itemiter->GetChildren() );
    }
  ++itemiter; // next item
  }
}
