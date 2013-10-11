SET(CPP_FILES

mitkUSActivator.cpp

## Model classes
USModel/mitkUSImage.cpp
USModel/mitkUSImageMetadata.cpp
USModel/mitkUSDevice.cpp
USModel/mitkUSVideoDevice.cpp
USModel/mitkUSVideoDeviceCustomControls.cpp
USModel/mitkUSProbe.cpp
USModel/mitkUSDevicePersistence.cpp

## Services
USService/mitkUSDeviceService.cpp

## Filters and sources
USFilters/mitkUSImageSource.cpp
USFilters/mitkUSImageVideoSource.cpp
USFilters/mitkUSImageToUSImageFilter.cpp

## Control Interfaces
USControlInterfaces/mitkUSAbstractControlInterface.cpp
USControlInterfaces/mitkUSControlInterfaceBMode.cpp
USControlInterfaces/mitkUSControlInterfaceProbes.cpp
USControlInterfaces/mitkUSControlInterfaceDoppler.cpp
)
