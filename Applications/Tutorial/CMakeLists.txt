INCLUDE_DIRECTORIES(${Q${KITNAME}_INCLUDE_DIRS})
INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(.)

# initialize only the source lists which need additional files
# Step6 is reused in Step7 and Step8. Step6.cpp has to be added 
# explicitly since it is not a member of ${STEP6_CPP} at this point
SET(STEP6_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6main.cpp)
SET(STEP7_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step7main.cpp)
SET(STEP8_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step8main.cpp)

# some files need Qt wrapping
QT4_WRAP_CPP(STEP6_CPP Step6.h)
QT4_WRAP_CPP(STEP7_CPP Step6.h Step7.h)
QT4_WRAP_CPP(STEP8_CPP Step6.h Step8.h)


# for the others it's all the same
FOREACH(stepNo RANGE 1 3) # exclude steps 4 to 8 for now
  SET(STEP${stepNo}_CPP ${STEP${stepNo}_CPP} Step${stepNo}.cpp QtTesting.cpp)
  ADD_EXECUTABLE(Step${stepNo} ${STEP${stepNo}_CPP})
  TARGET_LINK_LIBRARIES(Step${stepNo} Qmitk ${QT_LIBRARIES})
ENDFOREACH(stepNo)

