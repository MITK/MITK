#ifndef QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS
#define QMITK_DATATREELISTVIEW_H_INCLUDED_SNewS

/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include <qlistview.h>

#include <mitkDataTree.h>

namespace mitk
{
  class DataTreeFilter;
}

/// @brief Displays items of a mitk::DataTree
class QmitkDataTreeListView : public QListView
{
  Q_OBJECT;

  public:

    QmitkDataTreeListView(QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeFilter* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTree* filter,QWidget* parent = 0, const char* name = 0);
    QmitkDataTreeListView(mitk::DataTreeIteratorBase* filter,QWidget* parent = 0, const char* name = 0);
    ~QmitkDataTreeListView();

    void SetDataTree(mitk::DataTree*);
    void SetDataTree(mitk::DataTreeIteratorBase*);

    void SetFilter(mitk::DataTreeFilter*);
    
  protected:

    void GenerateItems();
    
  private:

    mitk::DataTreeFilter* m_DataTreeFilter;

};

#endif

