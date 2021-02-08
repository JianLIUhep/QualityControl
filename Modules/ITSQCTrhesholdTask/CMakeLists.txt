# ---- Library ----

add_library(QcITSQCTrhesholdTask)

target_sources(QcITSQCTrhesholdTask PRIVATE src/ITSQCTrhesholdTask.cxx)

target_include_directories(
  QcITSQCTrhesholdTask
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(QcITSQCTrhesholdTask PUBLIC QualityControl O2::ITSBase O2::ITSMFTBase O2::ITSMFTReconstruction PRIVATE ROOT::Gpad O2::DPLUtils O2::Framework)


install(TARGETS QcITSQCTrhesholdTask
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


add_root_dictionary(QcITSQCTrhesholdTask
                    HEADERS
                    include/ITSQCTrhesholdTask/ITSQCTrhesholdTask.h
                    LINKDEF include/ITSQCTrhesholdTask/LinkDef.h
                    BASENAME QcITSQCTrhesholdTask)


# ---- Test(s) ----

set(TEST_SRCS test/testQcITSQCTrhesholdTask.cxx)

foreach(test ${TEST_SRCS})
  get_filename_component(test_name ${test} NAME)
  string(REGEX REPLACE ".cxx" "" test_name ${test_name})

  add_executable(${test_name} ${test})
  target_link_libraries(${test_name}
                        PRIVATE QcITSQCTrhesholdTask Boost::unit_test_framework)
  add_test(NAME ${test_name} COMMAND ${test_name})
  set_property(TARGET ${test_name}
    PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 20)
endforeach()

set(EXE_SRCS src/runITSQCTrhesholdTask.cxx)

set(EXE_NAMES qcRunThreshold)

# These were the original names before the convention changed. We will get rid
# of them but for the time being we want to create symlinks to avoid confusion.
#set(EXE_OLD_NAMES qcRunIts)

list(LENGTH EXE_SRCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET EXE_SRCS ${i} src)
  list(GET EXE_NAMES ${i} name)
 # list(GET EXE_OLD_NAMES ${i} oldname)
  add_executable(${name} ${src})
  target_link_libraries(${name} PRIVATE QualityControl CURL::libcurl O2::ITSQCDataReaderWorkflow O2::DetectorsBase ROOT::Gpad O2::DPLUtils O2::Framework)
  #install_symlink(${name} ${CMAKE_INSTALL_FULL_BINDIR}/${oldname})
endforeach()

install(
  TARGETS ${EXE_NAMES}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ---- Extra scripts ----

install(FILES ITSQCTrhesholdTask.json DESTINATION etc)

get_property(dirs
             DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()



