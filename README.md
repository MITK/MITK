![MITK Logo][logo]

The [Medical Imaging Interaction Toolkit][mitk] (MITK) is a free open-source software
system for development of interactive medical image processing software. MITK
combines the [Insight Toolkit][itk] (ITK) and the [Visualization Toolkit][vtk] (VTK) with an application framework.

The links below provide high-level and reference documentation targeting different
usage scenarios:

 - Get a [high-level overview][mitk-overview] about MITK with pointers to further
   documentation
 - End-users looking for help with MITK applications should read the
   [MITK User Manual][mitk-usermanual]
 - Developers contributing to or using MITK, please see the [MITK Developer Manual][mitk-devmanual]
   as well as the [MITK API Reference][mitk-apiref]

See the [MITK homepage][mitk] for details.

Supported platforms
-------------------

MITK is a cross-platform C++ toolkit and officially supports:

 - Windows
 - Linux
 - macOS

For details, please read the [Supported Platforms][platforms] page.

License
-------

Copyright (c) [German Cancer Research Center (DKFZ)][dkfz]. All rights reserved.

MITK is available as free open-source software under a [3-clause BSD license][license].

Download
--------

The *MitkWorkbench* application and a bunch of command-line apps are released twice per year on our [Download][download] page and the [GitHub Releases][releases] page.

The official MITK source code is available in the [MITK Git repository][git_repo]. The Git clone command is

    git clone https://github.com/MITK/MITK.git

Active development takes place in the MITK develop branch and its usage is advised for advanced users only.

How to contribute
-----------------

Contributions are encouraged. To make the contribution process as smooth as possible, please read [Contributing to MITK][contribute] before.

Build instructions
------------------

MITK uses [CMake][cmake] to configure a build tree. The following is a crash course about cloning, configuring, and building MITK with Ninja on Linux or macOS when all [prerequisites][prerequisites] are met:

    git clone https://github.com/MITK/MITK.git
    mkdir MITK-superbuild
    cmake -S MITK -B MITK-superbuild -G "Ninja" -D CMAKE_BUILD_TYPE=Release
    cmake --build MITK-superbuild

On Windows, configuring and building with Visual Studio/MSBuild would look something like this:

    cmake -S MITK -B MITK-superbuild -G "Visual Studio 17 2022"
    cmake --build MITK-superbuild --config Release -- -m

Read the comprehensive [build instructions][build] page for details.

Useful links
------------

 - [Homepage][mitk]
 - [Download][download]
 - [Create an issue/ask for help][issues]

[logo]: https://github.com/MITK/MITK/raw/master/mitk.png
[mitk]: https://www.mitk.org
[itk]: https://itk.org
[vtk]: https://vtk.org
[mitk-overview]: https://docs.mitk.org/nightly/
[mitk-usermanual]: https://docs.mitk.org/nightly/UserManualPortal.html
[mitk-devmanual]: https://docs.mitk.org/nightly/DeveloperManualPortal.html
[mitk-apiref]: https://docs.mitk.org/nightly/usergroup0.html
[platforms]: https://docs.mitk.org/nightly/SupportedPlatformsPage.html
[prerequisites]: https://docs.mitk.org/nightly/BuildInstructionsPage.html#BuildInstructions_Prerequisites
[build]: https://docs.mitk.org/nightly/BuildInstructionsPage.html
[dkfz]: https://www.dkfz.de
[license]: https://github.com/MITK/MITK/blob/master/LICENSE
[download]: https://www.mitk.org/Download
[releases]: https://github.com/MITK/MITK/releases
[git_repo]: https://github.com/MITK/MITK
[contribute]: https://github.com/MITK/MITK/blob/master/CONTRIBUTING.md
[cmake]: https://www.cmake.org
[issues]: https://github.com/MITK/MITK/issues
