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

QmitkStoreSCPLauncher::QmitkStoreSCPLauncher()
{
}

QmitkStoreSCPLauncher::~QmitkStoreSCPLauncher()
{
}

QmitkStoreSCPLauncher::StartStoreSCP()
{
    QStringList argumentList;
    argumentList << "105" << "+x=" << "-d" << "-od" << m_DicomDirectoryListener->GetDicomListenerDirectory();
    QString storescp = GetWorkingDirectory().append("storescp");
    
    //argumentList.append(GetPort());
    //argumentList.append(GetTransferSyntax());
    //argumentList.append(GetMode());
    ////argumentList.append(GetAET());
    //QString DirectoryCMD("-od");
    //argumentList.append(DirectoryCMD);
    //argumentList.append(m_DicomDirectoryListener->GetDicomListenerDirectory());

    //QProcess::startDetached(storescp,argumentList,GetWorkingDirectory());
    m_StoreSCP = new QProcess(this);
    m_StoreSCP->start("cmd.exe");
}

QString QmitkStoreSCPLauncher::GetWorkingDirectory()
{
    QDir root;
    QString currentPath =  root.currentPath();
    currentPath = currentPath.split("MITK").at(0);
    currentPath.append("MITK/CTK-build/DCMTK-build/dcmnet/apps/Release/");
    return currentPath;
}