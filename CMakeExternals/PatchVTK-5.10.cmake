# Called by VTK.cmake (ExternalProject_Add) as a patch for VTK 5.10
if(APPLE)
  # patch for VTK 5.10 to work on Mac OS X

  # Updates vtkQtBarChart.cxx which fails to build on Mac OS X. The Mac OS X compiler can not resolve a isnan call of vtk.
  # Calling std::isnan solves this problem. But std::isnan is part of c++ 11 which most likely will not be recognized
  # by windows and linux compilers so this patch is needed only for Mac OS X systems
  # read whole file vtkQtBarChart.cxx
  file(STRINGS GUISupport/Qt/Chart/vtkQtBarChart.cxx sourceCode NEWLINE_CONSUME)

  # substitute dependency to gdcmMSFF by dependencies for more libraries
    string(REGEX REPLACE "[(]isnan" "(std::isnan" sourceCode ${sourceCode})

  # set variable CONTENTS, which is substituted in TEMPLATE_FILE
  set(CONTENTS ${sourceCode})
  configure_file(${TEMPLATE_FILE} GUISupport/Qt/Chart/vtkQtBarChart.cxx @ONLY)

endif()

if (WIN32)
  # patch for VTK 5.10, bug http://paraview.org/Bug/view.php?id=14122
  # fix is included in VTK 6, so we can remove this part as soon as VTK 6 is our default

  # complete patched file is in ${WIN32_OPENGL_RW_FILE}

  file(STRINGS ${WIN32_OPENGL_RW_FILE} sourceCode NEWLINE_CONSUME)
  set(CONTENTS ${sourceCode})
  configure_file(${TEMPLATE_FILE} Rendering/vtkWin32OpenGLRenderWindow.cxx @ONLY)
endif()
