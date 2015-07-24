cmake_minimum_required(VERSION 2.6)

project( NormalizeDWIGradientVectors )

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

add_executable( NormalizeDWIGradientVectors NormalizeDWIGradientVectors.cxx )

target_link_libraries( NormalizeDWIGradientVectors ${ITK_LIBRARIES} )

install( TARGETS NormalizeDWIGradientVectors RUNTIME DESTINATION ${INSTALL_RUNTIME_DESTINATION} )
