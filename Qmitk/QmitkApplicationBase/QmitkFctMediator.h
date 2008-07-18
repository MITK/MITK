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


#ifndef QMITK_FCTMEDIATOR_H
#define QMITK_FCTMEDIATOR_H

#include "QmitkFunctionality.h"
#include "QmitkDialogBar.h"

#include "mitkPropertyList.h"

#include <qptrlist.h>

class QWidgetStack;
class QButtonGroup;
class QToolBar;
class QActionGroup;
class QVBox;
class QComboBox;
class QLineEdit;
class QAction;

/**
* \brief Coordinates management of functionalities' control widgets.
*
* During startup, QmitkFctMediator creates a widget stack of all available
* functionalities, for both their control and main widgets (if available),
* and for each functionality adds its icon to the toolbar.
*
* Besides managing functionalities (QmitkFunctionality), QmitkFctMediator
* also holds a list of all available dialog bars (QmitkDialogBar). Dialog
* bars are small control widgets displayed below the functionalities'
* control panel and can be toggled on/off via their toolbar icons.
*
* The class interface provides methods for querying and selecting specific
* functionalities and tool bars.
*
* \ingroup Functionalities
*/
class QMITK_EXPORT QmitkFctMediator : public QObject
{
  Q_OBJECT

public:
  QmitkFctMediator( QObject *parent=0, const char *name=0 );
  virtual ~QmitkFctMediator();

  /**
  * \brief Initializes the mediator with the main window template's QWidget
  * and the list of default application properties (used for
  * activating/deactivating dialog bars by default).
  */
  virtual void Initialize( QWidget *aLayoutTemplate );

  /** \brief Adds the specified functionality; both its control widget and
  * main widget (if available) are added to the respective widget stacks,
  * and the functionality icon is added to the toolbar.
  */
  virtual bool AddFunctionality( QmitkFunctionality *functionality );

  /** \brief Adds the specified dialog bar; its control widget is added to the
  * dialog bar panel (below the functionality panel), and the dialog bar icon
  * is added to the toolbar.
  */
  virtual bool AddDialogBar( QmitkDialogBar *dialogBar );

  /** \brief Adds a separator at the current position in the toolbar */
  virtual void AddSeparator();

  virtual QmitkFunctionality *GetFunctionalityByName( const char *name );
  virtual QmitkFunctionality *GetFunctionalityById( int id );
  virtual QmitkFunctionality *GetActiveFunctionality();
  virtual int GetFunctionalityIdByName( const char *name );
  virtual int GetActiveFunctionalityId() const;

  virtual QmitkDialogBar *GetDialogBarByName( const char *name );
  virtual QmitkDialogBar *GetDialogBarById( int id );
  virtual int GetDialogBarIdByName( const char *name );

  virtual QWidget *GetMainParent();
  virtual QWidget *GetControlParent();
  virtual QWidget *GetToolBar();
  virtual QWidget *GetDefaultMain();

  virtual unsigned int GetFunctionalityCount();
  virtual unsigned int GetDialogBarCount();

  virtual void ApplyOptionsToDialogBars( mitk::PropertyList::Pointer options );

  virtual void InitToolbars();

  public slots:
    virtual void HideControls( bool hide );

    virtual void RaiseFunctionality( int id );
    virtual void RaiseFunctionality( QmitkFunctionality *aFunctionality );

    virtual void EnableDialogBar( int id, bool enable = true );
    virtual void EnableDialogBar( QmitkDialogBar *dialogBar, bool enable = true );
    virtual void PopulateSearchResults();
    virtual void ConvSearchSelection(const QString & str);

    protected slots:
      virtual void RaiseFunctionality( QAction *action );
      virtual void ToggleDialogBar( QAction *action );
      virtual void TrackFunctionalityCall( QAction *action );

signals:
      void searchSelected( int n );
      void functionalityRaised( QAction *action );

protected:
  QWidget *m_LayoutTemplate;
  QWidgetStack *m_MainStack;
  QWidgetStack *m_ControlStack;
  QVBox *m_DialogBarsFrame;
  QToolBar *m_ToolBar;
  QComboBox *m_ToolbarComboBox;
  QComboBox *m_SearchComboBox;
  QStringList m_FunctionalitySearchList;
  QLineEdit *m_SearchBar;
  QToolBar *m_FunctionalityToolBar;
  QPtrList< QAction > m_FavFunctionalityActions;
  QToolBar *m_FavoritesToolBar;
  QToolBar *m_SearchToolBar;
  QToolBar *m_IconToolBar;



  QWidget *m_DefaultMain;

  mitk::PropertyList::Pointer m_Options;

  int m_NumberOfFunctionalities;
  int m_NumberOfDialogBars;

  QActionGroup *m_FunctionalityActionGroup;
  int m_CurrentFunctionality;

  QPtrList< QmitkFunctionality > m_Functionalities;
  QPtrList< QmitkDialogBar > m_DialogBars;

  QPtrList< QWidget > m_DialogBarControls;

  QPtrList< QAction > m_FunctionalityActions;
  QPtrList< QAction > m_DialogBarActions;
};

#endif
