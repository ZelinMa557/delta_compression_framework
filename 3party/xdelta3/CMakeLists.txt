include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(xdelta3 STATIC xdelta3.c)
target_compile_definitions(xdelta3 PRIVATE -DREGRESSION_TEST=0 -DXD3_DEBUG=0 
                            -DSECONDARY_DJW=1 -DSECONDARY_FGK=1 -DXD3_MAIN=0)