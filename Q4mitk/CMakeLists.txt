# stub for Qt4 
#
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(. ${MITK_INCLUDE_DIRS} ${QT_INCLUDES})

SET(RENDERING_CPP
 QmitkAbortEventFilter.cpp
 QmitkApplicationCursor.cpp
 QmitkCallbackFromGUIThread.cpp
 QmitkRenderingManager.cpp
 QmitkRenderingManagerFactory.cpp
 QmitkRenderWindow.cpp
)

SET(MOC_H_FILES
 QmitkCallbackFromGUIThread.h
 QmitkRenderingManager.h
)

QT4_WRAP_CPP(RENDERING_CPP ${MOC_H_FILES})

SET(MISC_CPP
 QtTesting.cpp
)

ADD_EXECUTABLE(Step1 Step1.cpp ${RENDERING_CPP} ${MISC_CPP})
TARGET_LINK_LIBRARIES(Step1 ${MITK_LIBRARIES})

ADD_EXECUTABLE(Step2 Step2.cpp ${RENDERING_CPP} ${MISC_CPP})
TARGET_LINK_LIBRARIES(Step2 ${MITK_LIBRARIES})

ADD_EXECUTABLE(Step3 Step3.cpp ${RENDERING_CPP} ${MISC_CPP})
TARGET_LINK_LIBRARIES(Step3 ${MITK_LIBRARIES})

