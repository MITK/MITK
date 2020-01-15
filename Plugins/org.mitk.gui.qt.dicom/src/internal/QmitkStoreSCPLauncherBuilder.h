/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkStoreSCPLauncherBuilder_h
#define QmitkStoreSCPLauncherBuilder_h

#include <QString>
#include <QObject>

/**
* \brief QmitkStoreSCPLauncherBuilder it builds the arguments for QmitkStoreSCPLauncher.
*/
class QmitkStoreSCPLauncherBuilder : public QObject
{
    Q_OBJECT

public:

    /**
    * \brief QmitkStoreSCPLauncherBuilder constructor.
    */
    QmitkStoreSCPLauncherBuilder();

    /**
    * \brief QmitkStoreSCPLauncherBuilder destructor.
    */
    ~QmitkStoreSCPLauncherBuilder() override;

    /**
    * \brief Adds port to this object.
    * \param port default is 105
    */
    QmitkStoreSCPLauncherBuilder* AddPort(const QString& port = QString("105"));

    /**
    * \brief Adds aetitle to this object.
    * \param aeTitle default is STORESCP
    */
    QmitkStoreSCPLauncherBuilder* AddAETitle(const QString& aeTitle = QString("STORESCP"));

    /**
    * \brief Adds transfer syntax to this object.
    * \param transferSyntax default is +x= which means prefer uncompressed.
    */
    QmitkStoreSCPLauncherBuilder* AddTransferSyntax(const QString& transferSyntax = QString("+x="));

    /**
    * \brief Adds other network option to this object.
    * \param otherNetworkOptions default is -pm which means promiscuous mode, accept unknown SOP classes.
    */
    QmitkStoreSCPLauncherBuilder* AddOtherNetworkOptions(const QString& otherNetworkOptions = QString("-pm"));

    /**
    * \brief Adds mode option to this object.
    * \param mode default is -v which means promiscuous verbose mode.
    */
    QmitkStoreSCPLauncherBuilder* AddMode(const QString& mode = QString("-v"));

    /**
    * \brief Adds output directory to this object.
    * \param outputDirectory path to directory.
    */
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
#endif
