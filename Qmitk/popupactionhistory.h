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
#ifndef POPUPLISTBOX_H
#define POPUPLISTBOX_H

#include <qlistbox.h>
#include <qvbox.h>
#include <mitkCommon.h>

class QLabel;
    
/// A QListBox that tracks all mouse events and hands them on to its parent. Used by ListBoxPopup
class QMITK_EXPORT PassOnMouseListBox : public QListBox
{

  Q_OBJECT
    
  public:
    PassOnMouseListBox(QWidget* parent = 0, const char* name = 0);  
    
  protected:
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent * e);
    
  private:
    QWidget* my_parent;
};

/// A popup containing a listbox (PassOnMouseListBox) and a label. Tracks all mouse events. Derived classes can implement appropriate behaviour.
class QMITK_EXPORT PopupActionHistory : public QFrame
{

  Q_OBJECT
    
public:
  PopupActionHistory(QWidget* parent = 0, const char*  name=0);
  virtual ~PopupActionHistory() {};

  virtual void popup(QWidget* parent = 0);  /// Call to popup this widget. parent determines popup position
  QListBox* listbox() { return m_listbox; }   /// Can be modified by client. Ideally not during a popup, otherwise behaviour is undefined.
    
protected:
  virtual void mouseMoveEvent(QMouseEvent*);      /// To be overriden by derived classes
  virtual void mouseReleaseEvent(QMouseEvent*);   /// To be overriden by derived classes
  virtual void closeEvent(QCloseEvent*);          /// To be overriden by derived classes

  QLabel* m_label;                     /// The contained QLabel
  PassOnMouseListBox* m_listbox;       /// The contained QListBox with transparent mouse events

private:
  QWidget* m_popupParent;
  int m_moves;
  QWidget* my_parent;
};

#endif

