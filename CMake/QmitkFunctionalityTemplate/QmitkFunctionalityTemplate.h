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

#if !defined(QMITK_FUNCTIONALITYTEMPLATE_H__INCLUDED)
#define QMITK_FUNCTIONALITYTEMPLATE_H__INCLUDED

#include "QmitkFunctionality.h"
#include <mitkOperationActor.h>
#include <mitkProperties.h>
class QmitkStdMultiWidget;
class QmitkFunctionalityTemplateControls;

namespace mitk { 
  class DisplayInteractor; 
}
/*
!\class
\brief Functionality Template 
*/
class QmitkFunctionalityTemplate : public QmitkFunctionality
{  
  Q_OBJECT
  
  public:  
  /*!  
  \brief default constructor  
  */  
  QmitkFunctionalityTemplate(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkFunctionalityTemplate();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual QWidget * createControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * createMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void createConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * createAction(QActionGroup *parent);

  virtual void activated();

protected slots:  
  void treeChanged();

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * multiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkFunctionalityTemplateControls * controls;

};
#endif // !defined(QMITK_FUNCTIONALITYTEMPLATE_H__INCLUDED)
