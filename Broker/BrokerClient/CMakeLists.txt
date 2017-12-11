cmake_minimum_required (VERSION 3.0)

project (lwm2mclient)

option(DTLS "Enable DTLS" OFF)

include(${CMAKE_CURRENT_LIST_DIR}/../../core/wakaama.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../shared/shared.cmake)

add_definitions(-DLWM2M_CLIENT_MODE -DLWM2M_BOOTSTRAP -DLWM2M_SUPPORT_JSON)
add_definitions(${SHARED_DEFINITIONS} ${WAKAAMA_DEFINITIONS})


include_directories (${WAKAAMA_SOURCES_DIR} ${SHARED_INCLUDE_DIRS})
include_directories("/usr/local/include/libmongoc-1.0")
include_directories("/usr/local/include/libbson-1.0")


SET(SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/lwm2mclient.c
    ${CMAKE_CURRENT_LIST_DIR}/lwm2mclient.h
    ${CMAKE_CURRENT_LIST_DIR}/system_api.c
    ${CMAKE_CURRENT_LIST_DIR}/object_security.c
    ${CMAKE_CURRENT_LIST_DIR}/object_server.c
    ${CMAKE_CURRENT_LIST_DIR}/object_device.c
    #${CMAKE_CURRENT_LIST_DIR}/object_firmware.c
	${CMAKE_CURRENT_LIST_DIR}/object_software.c
	${CMAKE_CURRENT_LIST_DIR}/object_device_update.c
	${CMAKE_CURRENT_LIST_DIR}/mongodb_op.c
	${CMAKE_CURRENT_LIST_DIR}/assisst_func.c
	${CMAKE_CURRENT_LIST_DIR}/object_software_provision.c
	
    #${CMAKE_CURRENT_LIST_DIR}/object_location.c
    #${CMAKE_CURRENT_LIST_DIR}/object_connectivity_moni.c
    #${CMAKE_CURRENT_LIST_DIR}/object_connectivity_stat.c
    #${CMAKE_CURRENT_LIST_DIR}/object_access_control.c
    #${CMAKE_CURRENT_LIST_DIR}/test_object.c
	#${CMAKE_CURRENT_LIST_DIR}/update_func.c
	#${CMAKE_CURRENT_LIST_DIR}/update_func.h
	#${CMAKE_CURRENT_LIST_DIR}/cJSON.c
	#${CMAKE_CURRENT_LIST_DIR}/cJSON.h
    )

add_executable(${PROJECT_NAME} ${SOURCES} ${WAKAAMA_SOURCES} ${SHARED_SOURCES})
target_link_libraries(lwm2mclient pthread mongoc-1.0 bson-1.0)
# Add WITH_LOGS to debug variant
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:Debug>:WITH_LOGS>)

SOURCE_GROUP(wakaama FILES ${WAKAAMA_SOURCES})