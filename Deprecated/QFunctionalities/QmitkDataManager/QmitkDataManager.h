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


#ifndef Q_MITK_DATA_MANAGER_H
#define Q_MITK_DATA_MANAGER_H

#include "QmitkFunctionality.h"

class QmitkStdMultiWidget;
class QmitkDataManagerControls;

/*!
\brief Basic visual data management functionality for all MITK applications
\ingroup Functionalities
*/
class QmitkDataManager : public QmitkFunctionality
{
  Q_OBJECT

public slots:

public:

  /*!
  \brief default constructor
  */
  QmitkDataManager(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!
  \brief default destructor
  */
  virtual ~QmitkDataManager();

  /*!
  \brief method for creating the widget containing the application 
  controls, like sliders, buttons etc.
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
  \brief method for creating an QAction object, i.e. button & menu entry
  @param parent the parent QWidget
  */
  virtual QAction * CreateAction(QActionGroup *parent);

  /*! 
  * \brief returns the name of this functionality object
  */
  virtual QString GetFunctionalityName();


  /*!
  \brief  inits the widgets of a functionality, 
  i.e. initializing slider etc.
  */
  void initWidgets();

  /*!
  \brief  update the functionality specific props
  */
  void update();

  virtual void Activated();
  virtual void TreeChanged() 
  {
    update();
  };
  static bool test();
  static QmitkFunctionality* CreateInstance( QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL );
protected:

  /*!
  * controls containing sliders for scrolling through the slices
  */
  QmitkDataManagerControls * m_Controls;

  /*!
  * default main widget containing 4 windows showing 3 
  * orthogonal slices of the volume and a 3d render window
  */
  QmitkStdMultiWidget * m_MultiWidget;

};

#endif // Q_MITK_DATA_MANAGER_H
