/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYSYSTEMBUNDLEMANIFEST_H
#define BERRYSYSTEMBUNDLEMANIFEST_H

#include "berryIBundleManifest.h"

#include <QHash>

namespace berry {

class SystemBundleManifest : public IBundleManifest
{
public:
    SystemBundleManifest();

    virtual const std::string& GetActivatorClass() const;
    virtual const std::string& GetActivatorLibrary() const;
    virtual const std::string& GetCopyright() const;
    virtual ActivationPolicy GetActivationPolicy() const;
    virtual bool IsSystemBundle() const;
    virtual const std::string& GetName() const;
    virtual const Dependencies& GetRequiredBundles() const;
    virtual const std::string& GetSymbolicName() const;
    virtual const std::string& GetVendor() const;

private:

    Dependencies dependencies;
    QHash<QString, QString> manifestHeaders;

    std::string activatorClass;
    std::string activatorLib;
    std::string copyright;
    std::string name;
    std::string symbolicName;
    std::string vendor;
};

}

#endif // BERRYSYSTEMBUNDLEMANIFEST_H
