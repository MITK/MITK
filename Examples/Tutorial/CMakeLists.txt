MITK_CHECK_MODULE(MISSING QmitkExt)
if(MISSING)
  message("Tutorial won't be built because QmitkExt is missing.")
else(MISSING)

  MITK_USE_MODULE(QmitkExt)
  MITK_USE_MODULE(MitkExt)

  include_directories(${ALL_INCLUDE_DIRECTORIES})

  # initialize only the source lists which need additional files
  # Step6 is reused in Step7 and Step8. Step6.cpp has to be added 
  # explicitly since it is not a member of ${STEP6_CPP} at this point
  set(STEP6_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6main.cpp)
  set(STEP7_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step7main.cpp)
  set(STEP8_CPP Step6RegionGrowing1.cpp Step6RegionGrowing2.cpp Step6.cpp Step7.cpp Step8main.cpp)

  # some files need Qt wrapping
  QT4_WRAP_CPP(STEP6_MOC_CPP Step6.h)
  QT4_WRAP_CPP(STEP7_MOC_CPP Step7.h)
  QT4_WRAP_CPP(STEP8_MOC_CPP Step8.h)

  # set additional moc files to build
  set(STEP6_CPP ${STEP6_CPP} ${STEP6_MOC_CPP})
  set(STEP7_CPP ${STEP7_CPP} ${STEP7_MOC_CPP} ${STEP6_MOC_CPP})
  set(STEP8_CPP ${STEP8_CPP} ${STEP8_MOC_CPP} ${STEP7_MOC_CPP} ${STEP6_MOC_CPP})

  # for the others it's all the same
  foreach(stepNo RANGE 1 8) 
    set(STEP${stepNo}_CPP ${STEP${stepNo}_CPP} Step${stepNo}.cpp QtTesting.cpp)
    add_executable(Step${stepNo} ${STEP${stepNo}_CPP})
    target_link_libraries(Step${stepNo} ${ALL_LIBRARIES})
  endforeach(stepNo)

  #step 10 equals 1-8
  set(STEP10_CPP ${STEP10_CPP} Step10.cpp QtTesting.cpp)
  add_executable(Step10 ${STEP10_CPP})
  target_link_libraries(Step10 ${ALL_LIBRARIES})

  add_dependencies(Step7 Step6)
  add_dependencies(Step8 Step7 Step6)
  
  foreach(stepNo 1 2 3 4 5 6 7 8 10)
    # subproject support
    set_property(TARGET Step${stepNo} PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
    foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
      add_dependencies(${subproject} Step${stepNo})
    endforeach()
  endforeach()

  # adding additional example for mitk itk and vtk interaction
  add_executable(mitkWithITKAndVTK mitkWithITKAndVTK.cpp)
  target_link_libraries(mitkWithITKAndVTK ${ALL_LIBRARIES})
  
  # subproject support
  set_property(TARGET mitkWithITKAndVTK PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
  foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
    add_dependencies(${subproject} mitkWithITKAndVTK)
  endforeach()

endif(MISSING)
