if (_ADD_EXECUTABLE_HOOK)
	return()
endif()

function(add_executable ...)

	set_compile_flags(	MCU_SYSTEM_FILES_SOURCES
						SAVE_CMAKE_C_FLAGS_RELEASE
						SAVE_CMAKE_CXX_FLAGS_RELEASE
						SAVE_CMAKE_ASM_FLAGS_RELEASE)

	string (TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)

	if(DEBUG_HAL)
		set_compile_flags(	MCU_HAL_FILES_SOURCES
							SAVE_CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}
							SAVE_CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}
							SAVE_CMAKE_ASM_FLAGS_${CMAKE_BUILD_TYPE_UPPER})
	else()
		set_compile_flags(	MCU_HAL_FILES_SOURCES
							SAVE_CMAKE_C_FLAGS_RELEASE
							SAVE_CMAKE_CXX_FLAGS_RELEASE
							SAVE_CMAKE_ASM_FLAGS_RELEASE)
	endif()

	set (LIST_SOURCES_FILE ${ARGV})
	list (REMOVE_AT LIST_SOURCES_FILE 0)

	set_compile_flags(	LIST_SOURCES_FILE
						SAVE_CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}
						SAVE_CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}
						SAVE_CMAKE_ASM_FLAGS_${CMAKE_BUILD_TYPE_UPPER})

	_add_executable(${ARGV} 	${STM32F1_HAL_FILES_SOURCES}
								${STM32F1_SYSTEM_FILES_HEADERS}
								${STM32F1_SYSTEM_FILES_SOURCES})

	list (GET ARGV 0 _executable_name)
	add_dependencies(${_executable_name} _stm32f10x_core_library)
endfunction()



set (_ADD_EXECUTABLE_HOOK on)