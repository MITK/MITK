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
#ifndef UNDOBUTTON_H_1329857WEG
#define UNDOBUTTON_H_1329857WEG

#include <qframe.h>
#include <qwidgetplugin.h>
#include "mitkCommon.h"

//class QPushButton;
class QToolButton;
class QFrame;
class QHBoxLayout;
class QListBox;
class QListBoxItem;
class PopupUndoRedoListBox;

/*!
 * Nice undo or redo button with a popup list showing the last actions.
 * 
 * This class does just the display job. The derived QmitkUndoRedoButton 
 * connects the real mitk::VerboseLimitedLinearUndo to this widget.
 */
class QMITK_EXPORT
QUndoRedoButton : public QFrame
{
  Q_OBJECT;
  Q_ENUMS( Mode );
  Q_PROPERTY( Mode mode READ getMode WRITE setMode );
  Q_PROPERTY( QPixmap pixmap READ pixmap WRITE setPixmap );
  Q_PROPERTY( QString actionName READ actionName WRITE setActionName );

  public:

    /// Based on this, different pixmaps will be displayed on the button and some strings will change
    enum Mode { Undo, Redo };

    QUndoRedoButton( QWidget* parent = 0, const char* name = 0 );
    virtual ~QUndoRedoButton();

    /// Returns the displayed pixmap
    const QPixmap *pixmap() const;

    /// Sets a different pixmap
    void setPixmap( const QPixmap& pm );

    /// "Undo" or "Redo" most of the time
    const QString& actionName() const;
    void setActionName( const QString& description );

    /// Direct access to the contained QListbox
    QListBox* listbox() const;
   
    /// Will be called before showing the popup list.
    /// Inherited classes can fill the list in this method.
    virtual void beforePopup() {};
   
    /// List pops up on mouse release, so this is needed
    virtual void mouseReleaseEvent(QMouseEvent*);
   
    /// Returns the current mode (undo/redo)
    Mode getMode() const;

    /// Following methods are just reached through to m_popup->listbox(), this simplifies usage of this class
    void insertStringList( const QStringList& sl, int index=-1 );

    void insertItem( const QListBoxItem* lbi, int index=-1 );
    void insertItem( const QListBoxItem* lbi, const QListBoxItem *after );
    void insertItem( const QString &text, int index=-1 );
    void insertItem( const QPixmap &pixmap, int index=-1 );
    void insertItem( const QPixmap &pixmap, const QString &text, int index=-1 );

    void removeItem( int index );

    void changeItem( const QListBoxItem* lbi, int index );
    void changeItem( const QString &text, int index );
    void changeItem( const QPixmap &pixmap, int index );
    void changeItem( const QPixmap &pixmap, const QString &text, int index );

  signals:
    /// Emitted, when a number of actions is selected
    void undoRedoLast(int); 
    /// Emitted, when the button is clicked
    void buttonClicked();   

  public slots:
    /// Removes all items from the listbox.
    void clear();
    void setMode( Mode m );

  protected:
    Mode m_Mode;

  protected slots:
    /// Called by the popup list to tell us, how many items are selected
    void popupItemsSelected(int num);
    /// Connected to the contained pixmap-button's click event
    void iconButtonClicked();
    /// Connected to the contained arrow-button's click event
    void arrowButtonClicked();
    
  private:
    QToolButton* m_iconbutton;
    QToolButton* m_arrowbutton;
    QHBoxLayout* m_layout;
    PopupUndoRedoListBox* m_popup;
    QString m_actionName;
};

#endif
