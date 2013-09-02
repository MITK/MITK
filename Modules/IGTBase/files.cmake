set(CPP_FILES
 mitkRealTimeClock.cpp
)

if(WIN32)
  set(CPP_FILES ${CPP_FILES} mitkWindowsRealTimeClock.cpp)
else()
  set(CPP_FILES ${CPP_FILES} mitkLinuxRealTimeClock.cpp)
endif(WIN32)
