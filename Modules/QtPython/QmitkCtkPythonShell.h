/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCtkPythonShell_h
#define QmitkCtkPythonShell_h

#include <ctkPythonConsole.h>
#include <QString>
#include <MitkQtPythonExports.h>

///
/// forward declarations
///
struct QmitkCtkPythonShellData;
class ctkAbstractPythonManager;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;

///
/// Reimplements the ctkPythonConsole with drag and drop functionality for text
/// Furthermore it calls NotifyObserver() on the IPythonService to inform listeners
///
class MITKQTPYTHON_EXPORT QmitkCtkPythonShell : public ctkPythonConsole
{
  Q_OBJECT

public:
  QmitkCtkPythonShell(QWidget* parent = nullptr);
  ~QmitkCtkPythonShell() override;
public slots:
  void Paste( const QString& command );
protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  bool canInsertFromMimeData( const QMimeData *source ) const;
  void executeCommand(const QString& command) override;

private:
  QmitkCtkPythonShellData* d;
};




#endif // QmitkCtkPythonShell_h

