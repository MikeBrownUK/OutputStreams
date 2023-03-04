# CMake Master Project File
# By Mike Brown, 2022
#
cmake_minimum_required (VERSION 3.16)

set( CMakeRoot ${CMAKE_CURRENT_SOURCE_DIR} )

set( ProjectName "streamtest" )

# Executable or Runtime Library
set( IsARuntimeLib FALSE )

# Include Directories
set ( IncludeDirs
)

set ( IncludeDirsAbsolute
	"${CMAKE_CURRENT_SOURCE_DIR}/"
#set a GOOGLE_TEST_PATH environment variable or change these directly and also the two source files in ExtraFilesAbsolute
	"$ENV{GOOGLE_TEST_PATH}/include"
	"$ENV{GOOGLE_TEST_PATH}"
)

# Set paths to directories where all source within should be added to the compile list (though no recursion)
set( SourceDirectories
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"Utilities"
	"OutputStreams"
)

# any other single files here
set( ExtraFiles
# the test program itself
	"${CMAKE_CURRENT_SOURCE_DIR}/StreamTest.cpp"

)

set( ExtraFilesAbsolute
	"$ENV{GOOGLE_TEST_PATH}/src/gtest-all.cc"
	"$ENV{GOOGLE_TEST_PATH}/src/gtest_main.cc"
)

if(UNIX)
set( CompileDefinitions
	"STREAM_TEST_SUITE"
	#"STREAM_OUTPUT_STRIP"
)
elseif(WIN32)
set( CompileDefinitions
	"STREAM_TEST_SUITE"
	"_CRTDBG_MAP_ALLOC"
	#"STREAM_OUTPUT_STRIP"
)
set( LinkOptions )
endif()

set( LinuxLibraries
)

set( WindowsLibraries
)

function( RemoveRootPath )
if( "${ARGC}" GREATER "0" )
string( FIND "${ARGV0}" "${CMAKE_SOURCE_DIR}" Loc )
if( Loc EQUAL "0" )
string( LENGTH "${CMAKE_SOURCE_DIR}" SourceLen )
string( LENGTH "${ARGV0}" ParamLen)
if( SourceLen EQUAL ParamLen )
set( ARGV0 "" )
else()
math(EXPR Len "${SourceLen} + 1" OUTPUT_FORMAT DECIMAL) 
string( SUBSTRING "${ARGV0}" "${SourceLen}" -1 ARGV0 )
endif()
endif()
else()
endif()
set( ARGV0 "${ARGV0}" PARENT_SCOPE )
endfunction()

# Output various data to aid script debugging
message( "Project: ${ProjectName}" )
project ( ${ProjectName} )

if( IsARunTimeLib )
message( "Target is a Runtime Library" )
else()
message( "Target is an Executable" )
endif()

message( "Additional Source Directories:" )
foreach( Dir IN LISTS SourceDirectories )
RemoveRootPath( ${Dir} )
set( Dir "${ARGV0}" )
message( "      ${CMakeRoot}/${Dir}" )
set( cpp "${CMakeRoot}/${Dir}/*.cpp" )
set( h "${CMakeRoot}/${Dir}/*.h" )
file( GLOB SourceDirFiles "${cpp}" "${h}" )
if( AllSourceDirFiles )
set( AllSourceDirFiles "${AllSourceDirFiles}" "${SourceDirFiles}" )
else()
set( AllSourceDirFiles "${SourceDirFiles}" )
endif()
endforeach()

message( "All Source Files:" )
foreach( MyFile IN LISTS ExtraFiles )
RemoveRootPath( ${MyFile} )
set( MyFile "${ARGV0}" )
set( ExtraFile "${CMakeRoot}/${MyFile}" )
message( "     ${CMakeRoot}/${MyFile}" )
if( ExtraSourceFiles )
set( ExtraSourceFiles "${ExtraSourceFiles}" "${ExtraFile}" )
else()
set( ExtraSourceFiles "${ExtraFile}" )
endif()
endforeach()

foreach( MyFile IN LISTS ExtraFilesAbsolute )
message( "     ${MyFile}" )
if( ExtraSourceFiles )
set( ExtraSourceFiles "${ExtraSourceFiles}" "${MyFile}" )
else()
set( ExtraSourceFiles "${MyFile}" )
endif()
endforeach()
set( AllSourceDirFiles "${AllSourceDirFiles}" "${ExtraSourceFiles}" )


foreach( Dir IN LISTS IncludeDirs )
set( ThisDir "${CMakeRoot}/${Dir}" )
if( AllIncludeDirs )
set( AllIncludeDirs "${AllIncludeDirs}" "${ThisDir}" )
else()
set( AllIncludeDirs "${ThisDir}" )
endif()
endforeach()

foreach( Dir IN LISTS IncludeDirsAbsolute )
if( AllIncludeDirs )
set( AllIncludeDirs "${AllIncludeDirs}" "${Dir}" )
else()
set( AllIncludeDirs "${Dir}" )
endif()
endforeach()

# Add source to this project's executable.
add_executable ( ${ProjectName} "${AllSourceDirFiles}" )

target_compile_options( ${ProjectName} PRIVATE ${CompileOptions} )
target_compile_definitions( ${ProjectName} PRIVATE ${CompileDefinitions} )
target_link_options( ${ProjectName} PRIVATE ${LinkOptions} )

if(UNIX)
find_package (Threads REQUIRED)
target_link_libraries( ${ProjectName} ${CMAKE_THREAD_LIBS_INIT} ${LinuxLibraries} )
else()
target_link_libraries( ${ProjectName} ${WindowsLibraries} )
endif()

# Add and list all the include paths
foreach( Dir IN LISTS AllIncludeDirs )
message( "Include Dirs:" )
message( "   ${Dir}" )
endforeach()
target_include_directories( ${ProjectName} PUBLIC "${AllIncludeDirs}" )
# TODO: Add tests and install targets if needed.
