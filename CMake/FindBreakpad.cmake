
FIND_PATH(Breakpad_INCLUDE_DIR breakpad_googletest_includes.h DOC  "Directory breakpad/src/" PATHS ${Breakpad_SRC}/src ${Breakpad_DIR})

message(STATUS "FindBreakpad...")
message(STATUS " .. include at ${Breakpad_INCLUDE_DIR}")
message(STATUS " .. link libraries ${Breakpad_LIBRARIES}")
