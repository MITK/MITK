# This is done in the top-level CMakeLists.txt file because
# we need the imported GDCM targets globally visible for transitive
# dependencies.
#find_package(GDCM PATHS ${GDCM_DIR} REQUIRED)

list(APPEND ALL_INCLUDE_DIRECTORIES ${GDCM_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${GDCM_LIBRARIES})
