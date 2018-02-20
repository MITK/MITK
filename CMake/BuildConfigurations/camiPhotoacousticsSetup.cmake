message(STATUS "Configuring MITK Photoacoustics Setup Build")

# Enable open cv and open igt link, which is a necessary configuration
set(MITK_USE_OpenCV ON CACHE BOOL "MITK Use OpenCV Library" FORCE)
set(MITK_USE_OpenIGTLink ON CACHE BOOL "MITK Use OpenIGTLink Library" FORCE)
set(MITK_USE_US_DiPhAS_SDK ON CACHE BOOL "Use DiPhAS SDK" FORCE)
set(MITK_US_DiPhAS_SDK_PATH "C:/Users/dkfz/Source/Repos/UltrasoundResearchPlatform_SDK/UltrasoundResearchPlatformSDK_Cpp/x64" CACHE PATH "DiPhAS SDK Path")

set(MITK_USE_OPHIR_PYRO_HARDWARE ON CACHE BOOL "Use the Ophir sensor build-in in OPOTEK Phocus Mobile" FORCE)
set(MITK_OPHIR_API_PATH "C:/OphirCppWrapper" CACHE PATH "Ophir API Path")

set(MITK_USE_GALIL_HARDWARE ON CACHE BOOL "Use hardware build-in in OPOTEK Phocus Mobile" FORCE)
set(MITK_GALIL_API_PATH "C:/Program Files (x86)/Galil/gclib" CACHE PATH "Galil API Path")

# Enable default plugins and the navigation modules
set(MITK_CONFIG_PLUGINS
  org.mitk.gui.qt.datamanager
  org.mitk.gui.qt.stdmultiwidgeteditor
  org.mitk.gui.qt.imagenavigator
  org.mitk.gui.qt.properties
  org.mitk.gui.qt.viewnavigator
  org.mitk.gui.qt.ultrasound
  org.mitk.gui.qt.lasercontrol
)

