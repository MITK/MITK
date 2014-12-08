
Supported Platforms  {#SupportedPlatformsPage}
===================

MITK is a cross-platform framework and is available for the following platforms:

- Windows
- Linux/X11
- Mac OS X

Supported Platforms Details
---------------------------

The MITK team provides support for the most frequently used platforms and continuously runs testing procedures
to ensure compatibility. Due to the large amount of possible combinations of operating systems and compiler versions,
we divide platform support into two test categories (Tier 1 and Tier 2).

Although MITK may be built on a range of platform-compiler combinations, only a subset of these are actively
support by the MITK development team.

Tier 1 Platforms
----------------

All Tier 1 platforms are continuously tested by our unit test suite and other internal testing procedures.
Errors or bugs discovered in these platforms are prioritized and corrected as soon as possible.

| Platform                            | Compilers
| ----------------------------------- | -----------------------------
| Ubuntu Linux 12.04 (64-bit)         | gcc as provided by Ubuntu
| Microsoft Windows 7 (64-bit)        | MSVC 2010 Professional SP1 (x64)
| Apple OS X 10.9 "Mavericks"         | LLVM-Clang 6.0 as provided by Apple (XCode)


Tier 2 Platforms
----------------

Tier 2 platforms may or may not be tested on a regular basis. Some Tier 2 platforms are used by individual
members of the MITK development team on a daily basis and some only receive occasional testing. While we
strive to support these platforms, MITK users should note that errors may be present in released versions
as well as in the current master branch.

| Platform                           | Compilers
| ---------------------------------- | -----------------------------
| Fedora 20 (64-bit)                 | gcc as provided by Fedora
| Fedora 20 (64-bit)                 | Clang as provided by Fedora
| Microsoft Windows 7 (32-bit)       | MSVC 2012 Express (x64)
| Apple OS X 10.7 "Lion"             | LLVM-Clang as provided by Apple (XCode)


All platforms not listed above are not officially supported by the MITK team. However, we will happily accept
contributions to improve support for other platforms.

