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

#include <qaction.h>
#include <qwidget.h>
#include <qwidgetstack.h>

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

  public:

     /** \brief Constructor. */
    QmitkFunctionalityComponentContainer( );

     /** \brief Destructor. */
    ~QmitkFunctionalityComponentContainer();


    virtual QString GetFunctionalityName();
    void SetFunctionalityName(QString name);
    virtual QAction * CreateAction(QActionGroup* m_FunctionalityComponentActionGroup);
    virtual QWidget * CreateMainWidget(QWidgetStack* m_MainStack);
    virtual QWidget * CreateControlWidget(QWidgetStack* m_ControlStack);
    virtual void CreateConnections();
    virtual QString GetFunctionalityComponentName();
    virtual void Activated();
    void CreateSelector();




  protected:
    QString m_Name;

  private:



};

#endif