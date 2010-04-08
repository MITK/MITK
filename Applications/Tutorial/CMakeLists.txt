MITK_CHECK_MODULE(MISSING QmitkExt)
IF (MISSING) 
MESSAGE("Tutorial won't be built because QmitkExt is missing.")
ELSE(MISSING) 
MITK_USE_MODULE(QmitkExt)

INCLUDE_DIRECTORIES(${ALL_INCLUDE_DIRECTORIES})


# initialize only the source lists which need additional files
# Step6 is reused in Step7 and Step8. Step6.cpp has to be added 
# explicitly since it is not a member of ${STEP6_CPP} at this point
SET(STEP6_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6main.cpp)
SET(STEP7_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step7main.cpp)
SET(STEP8_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step8main.cpp)

# some files need Qt wrapping
QT4_WRAP_CPP(STEP6_MOC_CPP Step6.h)
QT4_WRAP_CPP(STEP7_MOC_CPP Step7.h)
QT4_WRAP_CPP(STEP8_MOC_CPP Step8.h)

# set additional moc files to build
SET(STEP6_CPP ${STEP6_CPP} ${STEP6_MOC_CPP})
SET(STEP7_CPP ${STEP7_CPP} ${STEP7_MOC_CPP} ${STEP6_MOC_CPP})
SET(STEP8_CPP ${STEP8_CPP} ${STEP8_MOC_CPP} ${STEP7_MOC_CPP} ${STEP6_MOC_CPP})

# for the others it's all the same
FOREACH(stepNo RANGE 1 8) 
  SET(STEP${stepNo}_CPP ${STEP${stepNo}_CPP} Step${stepNo}.cpp QtTesting.cpp)
  ADD_EXECUTABLE(Step${stepNo} ${STEP${stepNo}_CPP})
  TARGET_LINK_LIBRARIES(Step${stepNo} ${ALL_LIBRARIES})
  
ENDFOREACH(stepNo)

ADD_DEPENDENCIES(Step7 Step6)
ADD_DEPENDENCIES(Step8 Step7 Step6)

ENDIF(MISSING)
