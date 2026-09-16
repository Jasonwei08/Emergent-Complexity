cmake_minimum_required(VERSION 3.15)
project(life_engine_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(life_engine STATIC engine.cpp engine.hpp experiments.cpp experiments.hpp)

add_executable(life main.cpp)
target_link_libraries(life PRIVATE life_engine)

enable_testing()
add_executable(life_tests tests.cpp)
target_link_libraries(life_tests PRIVATE life_engine)
add_test(NAME life_tests COMMAND life_tests)

# Preserve the native targets; add the browser application only under Emscripten.
if(EMSCRIPTEN)
    target_compile_options(life_engine PUBLIC -fexceptions)
    target_link_options(life PRIVATE -fexceptions)
    set_target_properties(life_tests PROPERTIES SUFFIX ".cjs")
    target_link_options(life_tests PRIVATE -fexceptions -sENVIRONMENT=node -sALLOW_MEMORY_GROWTH=1)

    add_executable(life_web wasm_bindings.cpp)
    target_link_libraries(life_web PRIVATE life_engine)
    target_link_options(life_web PRIVATE
        --bind -fexceptions -sMODULARIZE=1 -sEXPORT_ES6=1
        -sENVIRONMENT=web,worker,node -sALLOW_MEMORY_GROWTH=1)
    set_target_properties(life_web PROPERTIES
        OUTPUT_NAME life_engine SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/web")

    # Stage an explicit allowlist: source, assignment, tests and results are not published.
    set(SITE_DIR "${CMAKE_BINARY_DIR}/site")
    set(SITE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/..")
    add_custom_target(site
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SITE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SITE_SOURCE}/index.html" "${SITE_SOURCE}/style.css"
            "${SITE_SOURCE}/app.js" "${SITE_SOURCE}/lab.js"
            "${SITE_SOURCE}/experiment-worker.js" "${SITE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:life_web>" "${CMAKE_BINARY_DIR}/web/life_engine.wasm" "${SITE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E touch "${SITE_DIR}/.nojekyll"
        DEPENDS life_web
        VERBATIM)
endif()
