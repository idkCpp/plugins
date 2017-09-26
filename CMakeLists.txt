# Do not export symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Set the RPATH for the library lookup
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/albert")

# Extension Plugins
add_subdirectory(applications)
add_subdirectory(calculator)
add_subdirectory(chromebookmarks)
add_subdirectory(externalextensions)
add_subdirectory(files)
add_subdirectory(firefoxbookmarks)
add_subdirectory(hashgenerator)
add_subdirectory(kvstore)
add_subdirectory(mpris)
add_subdirectory(ssh)
add_subdirectory(system)
add_subdirectory(terminal)
add_subdirectory(virtualbox)
add_subdirectory(websearch)
add_subdirectory(xcbws)

# Frontend Plugins
add_subdirectory(qmlboxmodel)
add_subdirectory(widgetboxmodel)

# Non-release plugins
if(${BUILD_DEBUG_EXTENSIONS})
    add_subdirectory(debug)
    add_subdirectory(templateExtension)
endif(${BUILD_DEBUG_EXTENSIONS})
