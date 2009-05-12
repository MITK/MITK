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

#ifndef QMITK_DATATREEFILTERDEMO_H__INCLUDED
#define QMITK_DATATREEFILTERDEMO_H__INCLUDED

#include <QmitkFunctionality.h>

#include <mitkDataTreeFilter.h>

class QmitkStdMultiWidget;
class QmitkDataTreeFilterDemoControls;

/*!
\brief DataTreeFilterDemo demostrates the usage of mitk::DataTreeFilter, QmitkDataTreeListView and QmitkDataTreeComboBox.

This gives an impression of The usage and possibilities of QmitkDataTreeListView and QmitkDataTreeComboBox. 
This demo should be easy to modify (e.g. display different properties) and extend.

There are certainly lots of bugs left in these classes... Bug reports with suggested patches are welcome :-)

Ideas for enhancement:
 - popup menus for the listbox
 - a proper listbox instead of the custom-made one
 - tests tests tests
 - a third kind of widget like in QmitkDataManager showing something like a property inspector in Qt Designer, Delphi, Visual Basic

\ingroup Functionalities
*/
class QmitkDataTreeFilterDemo : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkDataTreeFilterDemo(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkDataTreeFilterDemo();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();
  
  virtual void Deactivated();
 
  void ListboxModifiedHandler( const itk::EventObject& e );
  
protected slots:  
  void TreeChanged();
  
  void onComboBoxItemSelected(const mitk::DataTreeFilter::Item*);
  void onListboxItemClicked(const mitk::DataTreeFilter::Item*, bool);
  void onListboxItemAdded(const mitk::DataTreeFilter::Item*);

  void onCmbFilterFunctionActivated(int);
  void onChkHierarchyToggled(bool);
  
protected:  
  
  void MoveCrossHairToItem(const mitk::DataTreeFilter::Item*);

  void ConnectListboxNotification();

  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkDataTreeFilterDemoControls * m_Controls;

  mitk::DataTreeFilter::Pointer m_DataTreeFilter;
  
  bool m_FilterInitialized;

  unsigned long m_ListboxModifiedConnection;
};


#endif 
