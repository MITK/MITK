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
#include <qwidget.h>

#include <mitkDataTreeFilter.h>

namespace mitk {
  class XMLWriter;
}

class QmitkDataTreeListView;
class QPushButton;
class QCheckBox;
class QLineEdit;
class QToolButton;

//##Documentation
//## @brief QmitkSaveProjectWidget class for storing a project. 
//##
//## Class QmitkSaveProjectWidget represents a dialog for storing a project.
//## Items (nodes of the datatree) can be selected to store.
//## @ingroup Widgets
class QmitkSaveProjectWidget : public QWidget
{
  Q_OBJECT

  public:

    QmitkSaveProjectWidget(mitk::DataTree::Pointer dataTree, QWidget* parent, const char* name = 0);

    virtual ~QmitkSaveProjectWidget();
 
  public slots:
    
    /// opens the save dialog and calls the function to write the XML file
    void Save();

    /// closes the save dialog without saving the datatree
    void Cancel();

    /// opens a dialog to set the directory of the source files
    void ChooseSourceFolder();

    /// sets elements (folderChooseButton, folderLineEdit, treelistview) of the GUI editable
    void SetEditable(int state);
    
  protected:
    void PrepareDataTreeFilter();

    /// sets the layout of the QmitkSaveProjectWidget
    void SetLayout();
    
    /// connects signals and slots
    void AddConnections();

    /// starts the xmlWriter
    void TryToExportTree(QString);
    
    /// writes the selected nodes of the datatree to the XML file
    void WriteSelectedItems(const mitk::DataTreeFilter::ItemList*, mitk::XMLWriter&);

  private:
    /// replaces white spaces with "_"
    std::string ReplaceWhiteSpaces(std::string);

    void AddCheckBoxWrite(const mitk::DataTreeFilter::ItemList*);

    mitk::DataTree::Pointer m_DataTree;

    mitk::DataTreeFilter::Pointer tree_filter;
    
    mitk::DataTreeFilter::PropertyList visible_props;
    mitk::DataTreeFilter::PropertyList editable_props;

    QmitkDataTreeListView * treelistview;
    QToolButton * folderChooseButton;
    QLineEdit * folderLineEdit;
    QPushButton * saveButton;
    QPushButton * cancelButton;
    QCheckBox * sourceCheckBox;


};

