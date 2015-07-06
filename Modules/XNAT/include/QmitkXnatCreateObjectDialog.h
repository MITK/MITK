/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKXNATCREATEOBJECTDIALOG_H
#define QMITKXNATCREATEOBJECTDIALOG_H

#include <MitkXNATExports.h>

// Qt
#include <QDialog>
#include <QWidget>

class ctkXnatObject;

class MITKXNAT_EXPORT QmitkXnatCreateObjectDialog : public QDialog
{
  Q_OBJECT

public:
  enum SpecificType
  {
    //PROJECT,
    SUBJECT,
    EXPERIMENT
  };

  QmitkXnatCreateObjectDialog(SpecificType type, QWidget* parent = 0);
  virtual ~QmitkXnatCreateObjectDialog();

  // Returns a specific xnat object like SpecificType
  ctkXnatObject* GetXnatObject();

  protected slots:

  void OnAcceptClicked();
  void OnCancelClicked();

private:
  SpecificType m_Type;
  ctkXnatObject* m_Object;
  QWidget* m_Widget;
};

#endif // QMITKXNATCREATEOBJECTDIALOG_H
