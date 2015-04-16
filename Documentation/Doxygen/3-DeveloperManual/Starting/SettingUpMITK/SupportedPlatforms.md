
Supported Platforms  {#SupportedPlatformsPage}
===================

MITK is a cross-platform framework that is available for the following platforms:

- Windows
- Linux/X11
- Mac OS X

Supported Platforms Details
---------------------------

The MITK team provides support for the most frequently used platforms and continuously runs testing procedures
to ensure compatibility. Due to the large amount of possible combinations of operating systems and compiler versions,
we divide platform support into two test categories: Tier 1 and Tier 2.

Although MITK may be built on a broader range of platform-compiler combinations, only a subset of these are actively
supported by the MITK development team.

Tier 1 Platforms
----------------

All Tier 1 platforms are continuously tested by our unit test suite and other internal testing procedures.
Errors or bugs discovered in these platforms are prioritized and corrected as soon as possible.

| Platform                            | Compilers
| ----------------------------------- | --------------------------------------------------
| Ubuntu Linux 14.04 (x64)            | GNU 4.8
| Ubuntu Linux 12.04 (x64)            | GNU 4.7 as provided by ppa:ubuntu-toolchain-r/test
| Microsoft Windows 7 (x64)           | MSVC 2012 Express Update 4
| Apple OS X 10.10 "Yosemite"         | Clang 6.0


Tier 2 Platforms
----------------

Tier 2 platforms may or may not be tested on a regular basis. Some Tier 2 platforms are used by individual
members of the MITK development team on a daily basis and some only receive occasional testing. While we
strive to support these platforms, MITK users should note that errors may be present in released versions
as well as in the current master branch.

| Platform                           | Compilers
| ---------------------------------- | --------------------------------------------------
| Ubuntu Linux 12.04 (x86)           | GNU 4.7 as provided by ppa:ubuntu-toolchain-r/test
| Microsoft Windows 7 (x64)          | MSVC 2012 Express Update 4 (x86)
| Microsoft Windows 8 (x64)          | MSVC 2013 Community Update 4
| Apple OS X 10.9 "Mavericks"        | Clang 5.0

All platforms not listed above are not officially supported by the MITK team. However, we will happily accept
contributions to improve support for other platforms.
