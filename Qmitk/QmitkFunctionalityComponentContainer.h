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

#ifndef QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H
#define QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H

#include "QmitkBaseFunctionalityComponent.h"

#include "mitkDataTree.h"
#include <qaction.h>
#include <qwidget.h>
#include <qwidgetstack.h>
#include "mitkDataTree.h"

class QmitkFunctionalityComponentContainerGUI;

/**
 * \brief ContainerClass for components 
 * \ingroup QmitkFunctionalityComponent
 * 
 * 
 * - \ref QmitkBaseFunctionalityComponent

 * 
 * \section QmitkFunctionalityComponentContainer Overview
 * 
 * The FunctionalityComponentContainer is a containerclass for several components inherit from it.
 * All Components including to one FunctionalityComponentContainer can be used together as one applied 
 * functionality. I.e. in particular that new functionalities can be combined with different components.
 * As common ground for all inherit classes the FunctionalityComponentContainer includes a selector for
 * marked data like image(s), segmentation(s) or model(s). 
 */


class QmitkFunctionalityComponentContainer : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT

  public:

     /** \brief Constructor. */
    QmitkFunctionalityComponentContainer(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = NULL);
     /** \brief Destructor. */
    ~QmitkFunctionalityComponentContainer();


    virtual QString GetFunctionalityName();
    void SetFunctionalityName(QString name);
    virtual QAction * CreateAction(QActionGroup* m_FunctionalityComponentActionGroup);
    virtual void CreateConnections();
    virtual QString GetFunctionalityComponentName();
    void QmitkFunctionalityComponentContainer::SetFunctionalityName(QString* name);
    virtual void Activated();
    void CreateSelector(QWidget* parent);


protected slots:  
  void TreeChanged();
  /*
   * just an example slot for the example TreeNodeSelector widget
   */
  void ImageSelected(mitk::DataTreeIteratorClone imageIt);

  protected:
    QString m_Name;
    QmitkFunctionalityComponentContainerGUI * m_GUI;

  private:



};

#endif

