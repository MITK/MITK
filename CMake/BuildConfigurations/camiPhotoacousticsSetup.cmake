message(STATUS "Configuring MITK Photoacoustics Setup Build")

# Enable open cv and open igt link, which is a necessary configuration
set(MITK_USE_OpenCV ON CACHE BOOL "MITK Use OpenCV Library" FORCE)
set(MITK_USE_OpenIGTLink ON CACHE BOOL "MITK Use OpenIGTLink Library" FORCE)
set(MITK_USE_US_DiPhAS_SDK ON CACHE BOOL "Use DiPhAS SDK" FORCE)
#set(MITK_USE_OPOTEK_HARDWARE ON CACHE BOOL "Use hardware build-in in OPOTEK Phocus Mobile" FORCE)
set(MITK_US_DiPhAS_SDK_PATH "C:/Users/dkfz/Source/Repos/UltrasoundResearchPlatform_SDK/UltrasoundResearchPlatformSDK_Cpp/x64/Release" CACHE PATH "DiPhAS SDK Path")

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

