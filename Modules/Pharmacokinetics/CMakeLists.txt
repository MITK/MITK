MITK_CREATE_MODULE(Pharmacokinetics
  INCLUDE_DIRS
    PUBLIC ${MITK_BINARY_DIR}
    PRIVATE src/Common src/Functors src/Models src/DescriptionParameters src/SimulationFramework
  DEPENDS
    PUBLIC MitkCore MitkModelFit
	PRIVATE MitkMultilabel
)

if(BUILD_TESTING)
  ADD_SUBDIRECTORY(test)
endif(BUILD_TESTING)

ADD_SUBDIRECTORY(autoload/Models)
ADD_SUBDIRECTORY(cmdapps)
