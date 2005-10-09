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


// QmitkFctMediator.h: interface for the QmitkFctMediator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QUSFUNCTIONALITYMANAGER_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
#define AFX_QUSFUNCTIONALITYMANAGER_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_

#include <QmitkFunctionality.h>

#include <qptrlist.h>

class QWidgetStack;
class QButtonGroup;
class QToolBar;
class QActionGroup;

/*!
\brief Mediator between functionalities

A QmitkFctMediator object gets passed a reference of a functionality and positions 
the widgets in the application window controlled by a layout template.

\ingroup Functionalities
*/
class QmitkFctMediator : public QObject
{
  Q_OBJECT
protected:
  QWidget*      m_LayoutTemplate;
  QWidgetStack* m_MainStack;
  QWidgetStack* m_ControlStack;
  QButtonGroup* m_ButtonMenu;
  QWidget*      m_ToolBar;

  QWidget* m_DefaultMain;

  int m_NumOfFuncs;
  int m_CurrentFunctionality;

  QActionGroup* m_FunctionalityActionGroup;

  QPtrList<QmitkFunctionality> qfl;
  QPtrList<QAction> qal;

protected slots:
  void Slot_dummy_protected() {};
  virtual void Selecting(int id);
  virtual void FunctionalitySelected(int id);
  virtual void RaiseFunctionality(QAction* action);
public slots:
  void Slot_dummy() {};

  virtual void HideControls(bool hide);
  virtual void HideMenu(bool hide);

  virtual void CheckAvailability();

  virtual void RaiseFunctionality(int id);
  virtual void RaiseFunctionality(QmitkFunctionality* aFunctionality);
public:
  virtual void Initialize(QWidget *aLayoutTemplate);

  virtual bool AddFunctionality(QmitkFunctionality * functionality);

  virtual QmitkFunctionality* GetFunctionalityByName(const char * name);
  virtual int GetFunctionalityIdByName( const char * name );
  virtual QWidget * GetMainParent();
  virtual QWidget * GetControlParent();
  virtual QButtonGroup * GetButtonMenu();
  virtual QWidget * GetToolBar();
  virtual QWidget * GetDefaultMain();

  QmitkFctMediator(QObject *parent=0, const char *name=0);
  virtual ~QmitkFctMediator();

};

#endif // !defined(AFX_QUSFUNCTIONALITYMANAGER_H__1DC0BA6E_9B8D_4D63_8A63_5B661CE33712__INCLUDED_)
