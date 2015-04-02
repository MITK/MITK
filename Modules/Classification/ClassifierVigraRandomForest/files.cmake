file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES

    mitkModuleActivator.cpp

    Classifier/mitkVigraRandomForestClassifier.cpp

    Splitter/mitkImpurityLoss.cpp
    Splitter/mitkLinearSplitting.cpp
    Splitter/mitkThresholdSplit.cpp

    IO/mitkRandomForestIO.cpp
)

set( TOOL_FILES
)
