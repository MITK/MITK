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


// QmitkFctCompMediator.h: interface for the QmitkFctCompMediator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUSFUNCTIONALITYCOMPONENTMEDIATORMANAGER_H_INCLUDED_)
#define AFX_QUSFUNCTIONALITYCOMPONENTMEDIATORMANAGER_H_INCLUDED_

#include <QmitkFunctionalityComponentContainer.h>

#include <qptrlist.h>
#include <qobject.h>
#include <qaction.h>

class QWidget;
class QWidgetStack;
class QButtonGroup;
class QToolBar;
class QActionGroup;

/*!
\brief Mediator between functionalities

A QmitkFctCompMediator object gets passed a reference of a functionality and positions 
the widgets in the application window controlled by a layout template.

\ingroup Functionalities
*/
class QMITK_EXPORT QmitkFctCompMediator : public QObject
{
  Q_OBJECT
protected:

  QActionGroup* m_FunctionalityComponentActionGroup;

  QPtrList<QmitkFunctionalityComponentContainer> qfl;
  QPtrList<QAction> qal;

protected slots:
//  virtual void Selecting(int id);
//  virtual void FunctionalityComponentSelected(int id);
//  virtual void RaiseFunctionalityComponent(QAction* action);
public slots:
//  virtual void RaiseFunctionalityComponent(int id);
//  virtual void RaiseFunctionalityComponent(QmitkFunctionalityComponentContainer* aFunctionalityComponent);
public:
 virtual void Initialize(QWidget *aLayoutTemplate);
 virtual bool AddFunctionalityComponent(QmitkFunctionalityComponentContainer * functionalityComponent);
// virtual QmitkFunctionalityComponentContainer* GetFunctionalityComponentByName(const char * name);
// virtual int GetActiveFunctionalityComponentId() const;
//  virtual QmitkFunctionalityComponentContainer* GetActiveFunctionalityComponent();
//  virtual QmitkFunctionalityComponentContainer* GetFunctionalityComponentById(int id);
//  virtual unsigned int GetFunctionalityComponentCount();
  
  QmitkFctCompMediator(QWidget *aLayoutTemplate);
  virtual ~QmitkFctCompMediator();

};

#endif // !defined(AFX_QUSFUNCTIONALITYCOMPONENTMEDIATORMANAGER_H_INCLUDED_)

