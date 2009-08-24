/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-02-10 14:14:32 +0100 (Di, 10 Feb 2009) $
 Version:   $Revision: 16224 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QMITKHOTKEYLINEEDIT_H_
#define QMITKHOTKEYLINEEDIT_H_

#include <QLineEdit>
#include <QKeySequence>

#include "mitkQtDataManagerDll.h"

class MITK_QT_DATAMANAGER QmitkHotkeyLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  static const QString TOOLTIP;

  QmitkHotkeyLineEdit(QWidget* parent = 0);
  QmitkHotkeyLineEdit(const QKeySequence& _QKeySequence, QWidget* parent = 0);
  QmitkHotkeyLineEdit(const QString& _QString, QWidget* parent = 0);

  virtual void SetKeySequence(const QKeySequence& _QKeySequence);
  virtual void SetKeySequence(const QString& _QKeySequenceAsString);
  virtual QKeySequence GetKeySequence();
  virtual QString GetKeySequenceAsString();

protected:
  virtual void keyPressEvent ( QKeyEvent * event );

protected:
  QKeySequence m_KeySequence;

};

#endif /* QMITKHOTKEYLINEEDIT_H_ */
