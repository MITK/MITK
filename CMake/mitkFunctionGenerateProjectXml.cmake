

function(mitkFunctionGenerateProjectXml dir name targets is_superbuild)
  if(NOT EXISTS ${dir})
    message(FATAL_ERROR "Directory ${dir} doesn't exist!")
  endif()

  set(xml_subprojects )

  if(${is_superbuild})
    set(xml_subprojects ${xml_subprojects} "  <SubProject name=\"SuperBuild\">\n")
  endif()

  foreach(target ${targets})

    # Remarks: Project.xml should contains all sub-project. That way
    # all dashboards should submit a similar file.

    set(xml_subprojects ${xml_subprojects} "  <SubProject name=\"${target}\">\n")

    if(${is_superbuild})
      set(xml_subprojects ${xml_subprojects} "    <Dependency name=\"SuperBuild\"/>\n")
    endif()

    # Generate XML related to the dependencies
    #foreach(dependency_name ${dependencies})
    #  set(xml_subprojects ${xml_subprojects} "    <Dependency name=\"${dependency_name}\"/>\n")
    #endforeach()

    set(xml_subprojects ${xml_subprojects} "  </SubProject>\n")
  endforeach()

  set(xml_subprojects ${xml_subprojects} "  <SubProject name=\"Documentation\">\n")
  foreach(subproject ${targets})
    set(xml_subprojects ${xml_subprojects} "    <Dependency name=\"${subproject}\"/>\n")
  endforeach()
  set(xml_subprojects ${xml_subprojects} "  </SubProject>\n")

  set(xml_content "<Project name=\"${name}\">\n${xml_subprojects}</Project>")
  set(filename "${dir}/Project.xml")

  file(WRITE ${filename} ${xml_content})
  message(STATUS "Generated: ${filename}")
endfunction()

