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

#ifndef QmitkStoreSCPLauncher_h
#define QmitkStoreSCPLauncher_h

#include <QProcess>
#include <QString>
#include "QmitkStoreSCPLauncherBuilder.h"

class QmitkStoreSCPLauncher : public QObject
{
    Q_OBJECT

public:
    QmitkStoreSCPLauncher(QmitkStoreSCPLauncherBuilder* builder);
    virtual ~QmitkStoreSCPLauncher();

    public slots:
        void StartStoreSCP();
        void OnProcessError(QProcess::ProcessError error);
        void OnStateChanged(QProcess::ProcessState status);


private:
    void FindPathToStoreSCP();
    void SetArgumentList(QmitkStoreSCPLauncherBuilder* builder);
    QString ArgumentListToQString();
    QString m_PathToStoreSCP;

    QProcess* m_StoreSCP;
    QStringList m_ArgumentList;
};
#endif //QmitkStoreSCPLauncher_h