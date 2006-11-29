INCLUDE(files.cmake)
ADD_LIBRARY(mitkDataStructures ${DATA_CPP})
TARGET_LINK_LIBRARIES(mitkDataStructures ${LIBRARIES_FOR_${KITNAME}_CORE} ipFunc)
