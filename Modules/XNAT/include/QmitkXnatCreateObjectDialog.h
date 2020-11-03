/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
    // PROJECT,
    SUBJECT,
    EXPERIMENT
  };

  QmitkXnatCreateObjectDialog(SpecificType type, QWidget *parent = nullptr);
  ~QmitkXnatCreateObjectDialog() override;

  // Returns a specific xnat object like SpecificType
  ctkXnatObject *GetXnatObject();

protected slots:

  void OnAcceptClicked();
  void OnCancelClicked();

private:
  SpecificType m_Type;
  ctkXnatObject *m_Object;
  QWidget *m_Widget;
};

#endif // QMITKXNATCREATEOBJECTDIALOG_H
