FIND_PACKAGE(Boost)

#here we could also choose the desired components:
#FIND_PACKAGE(Boost COMPONENTS graph math/special_functions)

LIST(APPEND ALL_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
LIST(APPEND ALL_LIBRARIES ${Boost_LIBRARIES})
link_directories(${Boost_LIBRARY_DIRS})

