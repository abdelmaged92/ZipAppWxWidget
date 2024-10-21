cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(ZipApp LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(wxBUILD_SHARED OFF)

message(STATUS "Fetching wxWidgets...")

FetchContent_Declare(
	wxWidgets 
	GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
	GIT_TAG        v3.2.6
	GIT_SHALLOW     TRUE
)
FetchContent_GetProperties(wxWidgets)	
FetchContent_MakeAvailable(wxWidgets)

set(SRCS main.cpp)

add_executable(ZipApp main.cpp)
target_link_libraries(ZipApp PRIVATE wxcore wxnet)