SET(CPP_FILES
  QcMITKTask.cpp
  QmitkEventCollector.cpp
  # CHILI-Plugin
  mitkChiliPlugin.cpp
  mitkChiliPluginFactory.cpp
  # sub-classes
  mitkCHILIInformation.cpp
  mitkLoadFromCHILI.cpp
  mitkSaveToCHILI.cpp
  mitkParentChild.cpp
  # interfaces
  mitkImageToPicDescriptor.cpp
  mitkPicDescriptorToNode.cpp
  # filter
  mitkImageNumberFilter.cpp
  mitkSingleSpacingFilter.cpp
  mitkSpacingSetFilter.cpp
  mitkStreamReader.cpp
  # mitkConference/mitkChiliConferenceKitFactory.cpp
  mitkConference/mitkConferenceKit.cpp
  mitkConference/mitkConferenceKitFactory.cpp
  mitkConference/mitkConferenceToken.cpp
  mitkConference/QmitkChili3ConferenceKit.cpp
  mitkConference/QmitkChili3ConferenceKitFactory.cpp
  # mitkConference/QmitkQChili3Conference.cpp
  mitkConference/TextLoggerConferenceKit.cpp
  mitkConference/TextLoggerConferenceKitFactory.cpp
)
SET(MOC_H_FILES
  mitkChiliPlugin.h
  QmitkEventCollector.h
)
