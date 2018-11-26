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

Supported Platforms
-----------------------------

MITK is a cross-platform C++ toolkit and officially supports:

 - Windows
 - Linux
 - macOS

For details, please read the [Supported Platforms][platforms] page.

License
-----------

Copyright (c) [German Cancer Research Center][dkfz].

MITK is available as free open-source software under a [BSD-style license][license].

Download
---------

The MITK source code and binaries for the *MitkWorkbench* application are released regularly according to the [MITK release cycle][release-cycle]. See the [Download][download] page for a list of releases.

The official MITK source code is available in the [MITK Git repository][diffusion]. The Git clone command is

    git clone https://phabricator.mitk.org/source/mitk.git MITK

Active development takes place in the MITK master branch and its usage is advised for advanced users only.

How to Contribute
--------------

Contributions of all kind are happily accepted. However, to make the contribution process as smooth as possible, please read the [How to contribute to MITK][contribute] page if you plan to contribute to MITK.


Build Instructions
------------------------

MITK uses [CMake][cmake] to configure a build tree. The following is a crash course about cloning, configuring, and building MITK on a Linux/Unix system:

    git clone https://phabricator.mitk.org/source/mitk.git MITK
    mkdir MITK-build
    cd MITK-build
    cmake ../MITK
    make -j4

Read the comprehensive [Build Instructions][build] page for details.

Useful Links
------------------

 - [Homepage][mitk]
 - [Download][download]
 - [Mailing List][mailinglist]
 - [Bug Tracker][bugs]


[logo]: https://github.com/MITK/MITK/raw/master/mitk.png
[mitk]: http://mitk.org
[itk]: https://itk.org
[vtk]: http://vtk.org
[mitk-overview]: http://docs.mitk.org/nightly/Overview.html
[mitk-usermanual]: http://docs.mitk.org/nightly/UserManualPortal.html
[mitk-devmanual]: http://docs.mitk.org/nightly/DeveloperManualPortal.html
[mitk-apiref]: http://docs.mitk.org/nightly/usergroup0.html
[platforms]: http://docs.mitk.org/nightly/SupportedPlatformsPage.html
[dkfz]: https://www.dkfz.de
[license]: https://github.com/MITK/MITK/blob/master/LICENSE.txt
[release-cycle]: http://mitk.org/MitkReleaseCycle
[download]: http://mitk.org/Download
[diffusion]: https://phabricator.mitk.org/source/mitk/
[contribute]: http://mitk.org/How_to_contribute
[cmake]: https://www.cmake.org
[build]: http://docs.mitk.org/nightly/BuildInstructionsPage.html
[mailinglist]: http://mitk.org/Mailinglist
[bugs]: https://phabricator.mitk.org/maniphest/
