cmake_minimum_required(VERSION 2.6)

project( NormalizeGradientVectors )

if(NOT SETIFEMPTY)
  macro(SETIFEMPTY)
    set(KEY ${ARGV0})
    set(VALUE ${ARGV1})
    if(NOT ${KEY})
      set(${KEY} ${VALUE})
    endif(NOT ${KEY})
  endmacro(SETIFEMPTY KEY VALUE)
endif(NOT SETIFEMPTY)

SETIFEMPTY(INSTALL_RUNTIME_DESTINATION bin)

find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )

add_executable( NormalizeGradientVectors NormalizeGradientVectors.cxx )

target_link_libraries( NormalizeGradientVectors ${ITK_LIBRARIES} )

install( TARGETS NormalizeGradientVectors RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION} )
