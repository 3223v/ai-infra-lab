# ==============================================================================
# Sanitizer 支持 (ASan, UBSan, TSan)
# ==============================================================================

function(enable_sanitizers target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    if(TINYINFER_ENABLE_ASAN)
      target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
      target_link_options(${target}    PRIVATE -fsanitize=address)
      message(STATUS "ASan enabled for ${target}")
    endif()

    if(TINYINFER_ENABLE_UBSAN)
      target_compile_options(${target} PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
      target_link_options(${target}    PRIVATE -fsanitize=undefined)
      message(STATUS "UBSan enabled for ${target}")
    endif()

    if(TINYINFER_ENABLE_TSAN)
      target_compile_options(${target} PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
      target_link_options(${target}    PRIVATE -fsanitize=thread)
      message(STATUS "TSan enabled for ${target}")
    endif()
  endif()
endfunction()
