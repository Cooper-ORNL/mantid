if(CXXTEST_FOUND)
  include_directories(SYSTEM
                      ${CXXTEST_INCLUDE_DIR}
)
  include_directories(../../TestHelpers/inc)
  # This variable is used within the cxxtest_add_test macro to build these
  # helper classes into the test executable. It will go out of scope at the end
  # of this file so doesn't need un-setting
  set(TESTHELPER_SRCS ../../TestHelpers/src/ParallelRunner.cpp)

  cxxtest_add_test(IndexingTest ${TEST_FILES} ${GMOCK_TEST_FILES})
  target_link_libraries(IndexingTest
                        PRIVATE
                        ${TCMALLOC_LIBRARIES_LINKTIME}
                        ${MANTIDLIBS}
                        Indexing
                        Parallel
                        gmock
  )
  if(OpenMP_CXX_FOUND)
    # Access to config service is required
    target_link_libraries(IndexingTest PRIVATE Kernel)
  endif()

  add_dependencies(FrameworkTests IndexingTest)
  # Add to the 'FrameworkTests' group in VS
  set_property(TARGET IndexingTest PROPERTY FOLDER "UnitTests")
endif()
