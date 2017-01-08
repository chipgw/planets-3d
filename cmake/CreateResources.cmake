# Places resource files into a .cpp and .hpp file. (stolen from stackoverflow with some tweaks)

include(CMakeParseArguments)

function(CREATE_RESOURCES)
    cmake_parse_arguments(RESOURCE_ARGS "" "OUTPUT" "TEXT;BIN" ${ARGN})

    set(OUTHPP "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_ARGS_OUTPUT}.hpp")
    set(OUTCPP "${CMAKE_CURRENT_BINARY_DIR}/${RESOURCE_ARGS_OUTPUT}.cpp")

    # Create output files.
    file(WRITE ${OUTHPP} "#pragma once\n#include <cstddef>\n")
    file(WRITE ${OUTCPP} "#include \"${RESOURCE_ARGS_OUTPUT}.hpp\"\n")

    # Iterate through input files
    foreach(FILE ${RESOURCE_ARGS_TEXT})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" FILENAME ${FILE})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" FILENAME ${FILENAME})
        # Read text data from file
        file(READ ${FILE} FILEDATA)
        # Append data to output file
        file(APPEND ${OUTHPP} "extern const unsigned char ${FILENAME}[];\nextern const size_t ${FILENAME}_size;\n")
        file(APPEND ${OUTCPP} "const unsigned char ${FILENAME}[] = R\"(${FILEDATA})\";\nconst size_t ${FILENAME}_size = sizeof(${FILENAME});\n")
    endforeach()
    # Iterate through input files
    foreach(FILE ${RESOURCE_ARGS_BIN})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" FILENAME ${FILE})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" FILENAME ${FILENAME})
        # Read hex data from file
        file(READ ${FILE} FILEDATA HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," FILEDATA ${FILEDATA})
        # Append data to output file
        file(APPEND ${OUTHPP} "extern const unsigned char ${FILENAME}[];\nextern const size_t ${FILENAME}_size;\n")
        file(APPEND ${OUTCPP} "const unsigned char ${FILENAME}[] = {${FILEDATA}0x00};\nconst size_t ${FILENAME}_size = sizeof(${FILENAME});\n")
    endforeach()
endfunction()
