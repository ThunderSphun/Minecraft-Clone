cmake_minimum_required(VERSION 3.12)

set(STB_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stb)

set(STB_HEADERS ${STB_HEADERS_DIR}/stb_perlin.h ${STB_HEADERS_DIR}/stb_image.h)

foreach(header_file ${STB_HEADERS})
	get_filename_component(header ${header_file} NAME_WE)
	string(REPLACE "stb_" "" module ${header})

	message(DEBUG "making library for ${header}.h")

    # Create macro name: e.g. "STB_IMAGE_IMPLEMENTATION"
    string(TOUPPER ${header} macro_name)
    set(macro_name "${macro_name}_IMPLEMENTATION")

    # Generate implementation .cpp from template
    set(output_cpp "${CMAKE_CURRENT_BINARY_DIR}/${header}_impl.cpp")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/generate_impl.cpp.in
        ${output_cpp}
        @ONLY
    )

    set(lib_name stb_${module})
    set(module_name stb::${module})

    add_library(${lib_name} STATIC ${output_cpp})
    target_include_directories(${lib_name} PUBLIC ${STB_HEADERS_DIR})
    add_library(${module_name} ALIAS ${lib_name})

	get_filename_component(output_cpp ${output_cpp} NAME)
	message(DEBUG "created library ${lib_name} (${module_name}) with generated file ${output_cpp}")
	message(DEBUG "")
endforeach()
