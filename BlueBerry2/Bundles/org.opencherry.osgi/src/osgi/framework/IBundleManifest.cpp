/*=========================================================================
 
Program:   openCherry Platform
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

#include "IBundleManifest.h"


namespace osgi {

namespace framework {

const std::string IBundleManifest::BUNDLE_ACTIVATOR = "Bundle-Activator";
const std::string IBundleManifest::BUNDLE_COPYRIGHT = "Bundle-Copyright";
const std::string IBundleManifest::BUNDLE_ACTIVATION_POLICY = "Bundle-ActivationPolicy";
const std::string IBundleManifest::BUNDLE_NAME = "Bundle-Name";
const std::string IBundleManifest::BUNDLE_SYMBOLICNAME = "Bundle-SymbolicName";
const std::string IBundleManifest::BUNDLE_VENDOR = "Bundle-Vendor";
const std::string IBundleManifest::BUNDLE_VERSION = "Bundle-Version";
const std::string IBundleManifest::MANIFEST_VERSION = "Manifest-Version";
const std::string IBundleManifest::SYSTEM_BUNDLE = "Cherry-SystemBundle";
const std::string IBundleManifest::REQUIRE_BUNDLE = "Require-Bundle";

 

const std::string IBundleManifest::VERSION = "1.0";

} } // namespace
