cmake_minimum_required(VERSION 3.16)
project(AquaLanguage VERSION 0.1.0 LANGUAGES CXX)

# Configurações do C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Flags de compilação
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Opções do projeto
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_DOCS "Build documentation" OFF)

# Diretórios
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Incluir diretórios
include_directories(${CMAKE_SOURCE_DIR}/src)

# Runtime library
set(RUNTIME_SOURCES
    src/runtime/runtime.cpp
)

add_library(aqua_runtime STATIC ${RUNTIME_SOURCES})
target_include_directories(aqua_runtime PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Configurações específicas da plataforma
if(UNIX AND NOT APPLE)
    # Linux
    target_compile_definitions(aqua_runtime PRIVATE _GNU_SOURCE)
    find_library(UCONTEXT_LIBRARY ucontext)
    if(UCONTEXT_LIBRARY)
        target_link_libraries(aqua_runtime ${UCONTEXT_LIBRARY})
    endif()
elseif(APPLE)
    # macOS
    target_compile_definitions(aqua_runtime PRIVATE _XOPEN_SOURCE)
elseif(WIN32)
    # Windows
    target_compile_definitions(aqua_runtime PRIVATE WIN32_LEAN_AND_MEAN)
    # Para Windows, seria necessário implementar fibras nativas ou usar uma biblioteca externa
endif()

# Exemplos
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Testes
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentação
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        
        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    endif()
endif()

# Instalação
install(TARGETS aqua_runtime
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY src/runtime/
    DESTINATION include/aqua
    FILES_MATCHING PATTERN "*.hpp")

# Configuração do pacote
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/AquaLanguageConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/AquaLanguageConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/AquaLanguageConfig.cmake"
    INSTALL_DESTINATION lib/cmake/AquaLanguage)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AquaLanguageConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/AquaLanguageConfigVersion.cmake"
    DESTINATION lib/cmake/AquaLanguage)

# Mensagem de status
message(STATUS "Aqua Language ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Build examples: ${BUILD_EXAMPLES}")
message(STATUS "Build docs: ${BUILD_DOCS}")
