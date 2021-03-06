project (PowerGraphApps CXX C)
cmake_minimum_required (VERSION 2.6)

if (NOT POWERGRAPH_HOME)
    set(POWERGRAPH_HOME $ENV{POWERGRAPH_HOME})
endif ()

if (NOT POWERGRAPH_HOME)
    message(FATAL_ERROR "Before running cmake, please specify $POWERGRAPH_HOME in the environment or using -D")
endif ()

if (NOT NO_MPI)
    set(NO_MPI $ENV{NO_MPI})
endif ()

if (NOT NO_JVM)
    set(NO_JVM $ENV{NO_JVM})
endif ()

if (GRANULA)
    add_definitions(-DGRANULA=1)
endif ()

find_package(OpenMP REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

find_package(Threads REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fopenmp -Wno-unused-variable -Wno-unused-parameter")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pedantic -Wextra -g3")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math -fomit-frame-pointer")
set (CMAKE_EXPORT_COMPILE_COMMANDS 1)

include_directories (SYSTEM ${POWERGRAPH_HOME}/deps/local/include)
include_directories (SYSTEM ${POWERGRAPH_HOME}/src)

link_directories (${POWERGRAPH_HOME}/deps/local/lib)
link_directories (${POWERGRAPH_HOME}/release/src/graphlab)

set (LIBS ${LIBS} graphlab)
set (LIBS ${LIBS} zookeeper_mt zookeeper_st)
set (LIBS ${LIBS} boost_context boost_filesystem boost_program_options boost_system boost_iostreams boost_date_time)
set (LIBS ${LIBS} tcmalloc event event_pthreads z)

if (NOT NO_MPI)
    find_package(MPI REQUIRED)

    include_directories (${MPI_CXX_INCLUDE_PATH})
    set (LIBS ${LIBS} ${MPI_CXX_LIBRARIES})
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_LINK_FLAGS}")

    add_definitions (-DHAS_MPI)
endif ()

if (NOT NO_JVM)
    set (LIBS "${LIBS} jvm hdfs")
endif ()

add_executable (main main.cpp)
target_link_libraries (main ${LIBS})
