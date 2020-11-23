set(EXENAME port-forwarding)

set(SOURCES_SUB ${SOURCES_SUB}
  main.cpp
)

set(HEADERS_SUB
  src/Module.hpp
  src/core/config.hpp

  BookFiler-Module-HTTP/Interface.hpp
)

include_directories(
  ../../include
  ../../third_party/rapidjson/include
  ../../third_party/Module-Manager/include
  ../../third_party/Settings-Manager/include
)

link_directories(
  ../../third_party/Module-Manager/lib
  ../../third_party/Settings-Manager/lib
)

add_executable(${EXENAME} ${SOURCES_SUB})

set(LIBRARIES
    # Boost
    Boost::system
    Boost::filesystem

    ModuleManager
    SettingsManager
)

if(WIN32)
    set(LIBRARIES ${LIBRARIES}
        # Windows Libraries

    )
elseif(UNIX)
    set(LIBRARIES ${LIBRARIES}
        # Unix Libraries
        dl
    )
endif()

target_link_libraries(${EXENAME} ${LIBRARIES})
