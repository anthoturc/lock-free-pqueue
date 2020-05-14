# Add current directory to include search path (so tests can find headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Link threading library to all executables in this assignment (needed for C++ threads)
link_libraries(Threads::Threads) 

set(CMAKE_BUILD_TYPE Debug)

# Modify this file as you need for your project (modeled on the most relevant assignment), e.g.
# uncomment the following and add the relevant files for your project
add_executable(project-main
  main.cc
  lock-free-pqueue.cc
  lock-free-pqueue.h
)