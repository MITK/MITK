SET(CPP_FILES

## Module Activator
mitkUSActivator.cpp

## Model Classes
USModel/mitkUSImage.cpp
USModel/mitkUSImageMetadata.cpp
USModel/mitkUSDevice.cpp
USModel/mitkUSIGTLDevice.cpp
USModel/mitkUSVideoDevice.cpp
USModel/mitkUSVideoDeviceCustomControls.cpp
USModel/mitkUSProbe.cpp
USModel/mitkUSDevicePersistence.cpp
USModel/mitkUSDeviceReaderWriterConstants.cpp
USModel/mitkUSDeviceReaderXML.cpp
USModel/mitkUSDeviceWriterXML.cpp

## Filters and Sources
USFilters/mitkUSImageLoggingFilter.cpp
USFilters/mitkUSImageSource.cpp
USFilters/mitkUSImageVideoSource.cpp
USFilters/mitkIGTLMessageToUSImageFilter.cpp

## Control Interfaces
USControlInterfaces/mitkUSAbstractControlInterface.cpp
USControlInterfaces/mitkUSControlInterfaceBMode.cpp
USControlInterfaces/mitkUSControlInterfaceProbes.cpp
USControlInterfaces/mitkUSControlInterfaceDoppler.cpp
)

set(RESOURCE_FILES
Interactions/USPointMarkInteractions.xml
Interactions/USZoneInteractions.xml
Interactions/USZoneInteractionsHold.xml
)
