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

#ifndef QmitkStoreSCPLauncherBuilder_h
#define QmitkStoreSCPLauncherBuilder_h

#include <QString>
#include <QObject>

class QmitkStoreSCPLauncherBuilder : public QObject
{
    Q_OBJECT

public:
    QmitkStoreSCPLauncherBuilder();
    virtual ~QmitkStoreSCPLauncherBuilder();
    QmitkStoreSCPLauncherBuilder* AddPort(const QString& port = QString("105"));
    QmitkStoreSCPLauncherBuilder* AddAETitle(const QString& aeTitle = QString("STORESCP"));
    QmitkStoreSCPLauncherBuilder* AddTransferSyntax(const QString& transferSyntax = QString("+x="));
    QmitkStoreSCPLauncherBuilder* AddOtherNetworkOptions(const QString& otherNetworkOptions = QString("-pm"));
    QmitkStoreSCPLauncherBuilder* AddMode(const QString& mode = QString("-d"));
    QmitkStoreSCPLauncherBuilder* AddOutputDirectory(const QString& outputDirectory);

    QString* GetPort();
    QString* GetAETitle();
    QString* GetTransferSyntax();
    QString* GetOtherNetworkOptions();
    QString* GetMode();
    QString* GetOutputDirectory();

private:
    QString* m_Port;
    QString* m_AETitle;
    QString* m_TransferSyntax;
    QString* m_OtherNetworkOptions;
    QString* m_Mode;   
    QString* m_OutputDirectory;
};
#endif QmitkStoreSCPLauncherBuilder_h