file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES

    mitkModuleActivator.cpp

    Classifier/mitkVigraRandomForestClassifier.cpp
    Classifier/mitkPURFClassifier.cpp

    Algorithm/itkHessianMatrixEigenvalueImageFilter.cpp
    Algorithm/itkStructureTensorEigenvalueImageFilter.cpp

    Splitter/mitkAdditionalRFData.cpp
    Splitter/mitkImpurityLoss.cpp
    Splitter/mitkPUImpurityLoss.cpp
    Splitter/mitkLinearSplitting.cpp
    Splitter/mitkThresholdSplit.cpp

    IO/mitkRandomForestIO.cpp
    IO/mitkVigraRandomForestClassifierSerializer.cpp
    IO/mitkDummyLsetReader.cpp
)

set( TOOL_FILES
)
