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

#if !defined(QmitkLoadSaveToChiliExample_H__INCLUDED)
#define QmitkLoadSaveToChiliExample_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkPACSPlugin.h"

#include "QmitkPluginListViewItem.h"

class QmitkStdMultiWidget;
class QmitkLoadSaveToChiliExampleControls;

/*!
\brief QmitkLoadSaveToChiliExample 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkLoadSaveToChiliExample : public QmitkFunctionality
{
  Q_OBJECT

  public:
  /*!
  \brief default constructor
  */
  QmitkLoadSaveToChiliExample(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!
  \brief default destructor
  */
  virtual ~QmitkLoadSaveToChiliExample();

  /*!
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.
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

  virtual void TreeChanged();

protected slots:

  void SaveDataTree();
  void SaveSingleNodes();
  void LoadFromStudyListView();
  void LoadFromPSListView();
  void ChangeReaderType();
  void CheckPluginCapabilties();

protected:

  mitk::PACSPlugin* m_Plugin;

  unsigned long m_ObserverTag;

  struct MimeTypeStruct
  {
    std::string mimeType;
    QmitkPluginListViewItem* parentItem;
  };

  void PluginEventNewStudySelected(const itk::EventObject&);

  void AddElementsToPSContent( mitk::PACSPlugin::PSRelationInformation singleElement, mitk::PACSPlugin::PSRelationInformationList elementList, QListViewItem* parent );

  void SetDataTreeIterator(mitk::DataTreeIteratorBase* it);

  /*!
  * default main widget containing 4 windows showing 3
  * orthogonal slices of the volume and a 3d render window
  */
  QmitkStdMultiWidget * m_MultiWidget;

  /*!
  * controls containing sliders for scrolling through the slices
  */
  QmitkLoadSaveToChiliExampleControls * m_Controls;

};
#endif // !defined(QmitkLoadSaveToChiliExample_H__INCLUDED)

