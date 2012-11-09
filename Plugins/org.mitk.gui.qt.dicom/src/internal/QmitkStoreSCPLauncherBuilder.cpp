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

#include "QmitkStoreSCPLauncherBuilder.h"

QmitkStoreSCPLauncherBuilder::QmitkStoreSCPLauncherBuilder()
: m_Port(new QString())
, m_AETitle(new QString())
, m_TransferSyntax(new QString())
, m_OtherNetworkOptions(new QString())
, m_Mode(new QString())
, m_OutputDirectory(new QString())
{
}

QmitkStoreSCPLauncherBuilder::~QmitkStoreSCPLauncherBuilder()
{
    delete m_Port;
    delete m_AETitle;
    delete m_TransferSyntax;
    delete m_OtherNetworkOptions;
    delete m_Mode;
    delete m_OutputDirectory;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddPort(const QString& port)
{
    m_Port->clear();
    m_Port->append(port);
    return this;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddAETitle(const QString& aeTitle)
{
    m_AETitle->clear();
    m_AETitle->append(aeTitle);
    return this;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddTransferSyntax(const QString& transferSyntax)
{
    m_TransferSyntax->clear();
    m_TransferSyntax->append(transferSyntax);
    return this;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddOtherNetworkOptions(const QString& otherNetworkOptions)
{
    m_OtherNetworkOptions->clear();
    m_OtherNetworkOptions->append(otherNetworkOptions);
    return this;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddMode(const QString& mode)
{
    m_Mode->clear();
    m_Mode->append(mode);
    return this;
}

QmitkStoreSCPLauncherBuilder* QmitkStoreSCPLauncherBuilder::AddOutputDirectory(const QString& outputDirectory)
{
    m_OutputDirectory->clear();
    m_OutputDirectory->append(outputDirectory);
    return this;
}

QString* QmitkStoreSCPLauncherBuilder::GetPort()
{
    return m_Port;
}

QString* QmitkStoreSCPLauncherBuilder::GetAETitle()
{
    return m_AETitle;
}

QString* QmitkStoreSCPLauncherBuilder::GetTransferSyntax()
{
    return m_TransferSyntax;
}

QString* QmitkStoreSCPLauncherBuilder::GetOtherNetworkOptions()
{
    return m_OtherNetworkOptions;
}

QString* QmitkStoreSCPLauncherBuilder::GetMode()
{
    return m_Mode;
}

QString* QmitkStoreSCPLauncherBuilder::GetOutputDirectory()
{
    return m_OutputDirectory;
}