
Supported Platforms  {#SupportedPlatformsPage}
===================

MITK is a cross-platform framework that is available for the following platforms:

- Windows
- Linux
- macOS

Supported Platforms Details
---------------------------

The MITK team provides support for the most frequently used platforms and continuously runs testing procedures
to ensure compatibility. Due to the large amount of possible combinations of operating systems and compiler versions,
we divide platform support into two test categories: Tier 1 and Tier 2.

Although MITK may be built on a broader range of platform-compiler combinations, only a subset of these are actively
supported by the MITK development team.

In general, only 64-bit builds are supported.

Tier 1 Platforms
----------------

All Tier 1 platforms are continuously tested by our unit test suite and other internal testing procedures.
Errors or bugs discovered in these platforms are prioritized and corrected as soon as possible.

| Platform                            | Compilers
| ----------------------------------- | --------------------------------------------------
| Microsoft Windows 10                | Visual Studio 2019 (latest update)
| Linux Ubuntu 22.04                  | Default GCC version
| Linux Ubuntu 20.04                  | Default GCC version

Tier 2 Platforms
----------------

Tier 2 platforms may or may not be tested on a regular basis. Some Tier 2 platforms are used by individual
members of the MITK development team on a daily basis and some only receive occasional testing. While we
strive to support these platforms, MITK users should note that errors may be present in released versions
as well as in the current master branch.

| Platform                            | Compilers
| ----------------------------------- | --------------------------------------------------
| Microsoft Windows 10                | Visual Studio 2022 (latest update)
| Apple macOS 11 "Big Sur"            | Default Apple Clang version
| Apple macOS 10.15 "Catalina"        | Default Apple Clang version

All platforms not listed above are not officially supported by the MITK team. However, we will happily accept
contributions to improve support for other platforms as long as we have the hardware and capacity for maintenance.

CI Build Clients
----------------

To get an overview of currently tested platforms, see the <a href="https://cdash.mitk.org/index.php?project=MITK">build reports on our CDash site</a>.
