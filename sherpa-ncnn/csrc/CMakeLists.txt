include_directories(${CMAKE_SOURCE_DIR})

if(SHERPA_NCNN_ENABLE_PYTHON)
  message(STATUS "PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}" -c "import sys; print('.'.join(sys.version.split('.')[:2]))"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PYTHON_VERSION
  )
  message(STATUS "PYTHON_VERSION: ${PYTHON_VERSION}")
endif()

set(sherpa_ncnn_core_srcs
  context-graph.cc
  conv-emformer-model.cc
  decoder.cc
  endpoint.cc
  features.cc
  greedy-search-decoder.cc
  hypothesis.cc
  lstm-model.cc
  meta-data.cc
  model.cc
  modified-beam-search-decoder.cc
  poolingmodulenoproj.cc
  recognizer.cc
  resample.cc
  simpleupsample.cc
  stack.cc
  stream.cc
  symbol-table.cc
  tensorasstrided.cc
  wave-reader.cc
  zipformer-model.cc
)
add_library(sherpa-ncnn-core ${sherpa_ncnn_core_srcs})
target_link_libraries(sherpa-ncnn-core PUBLIC kaldi-native-fbank-core ncnn)

if(SHERPA_NCNN_ENABLE_PYTHON AND WIN32)
  install(TARGETS sherpa-ncnn-core DESTINATION ..)
else()
  install(TARGETS sherpa-ncnn-core DESTINATION lib)
endif()

if(SHERPA_NCNN_ENABLE_BINARY)
  add_executable(sherpa-ncnn sherpa-ncnn.cc)
  target_link_libraries(sherpa-ncnn sherpa-ncnn-core)
  install(TARGETS sherpa-ncnn DESTINATION bin)

  set(main_exes sherpa-ncnn)

  if(SHERPA_NCNN_HAS_ALSA)
    add_executable(sherpa-ncnn-alsa sherpa-ncnn-alsa.cc alsa.cc)
    target_link_libraries(sherpa-ncnn-alsa sherpa-ncnn-core)

    if(DEFINED ENV{SHERPA_NCNN_ALSA_LIB_DIR})
      target_link_libraries(sherpa-ncnn-alsa -L$ENV{SHERPA_NCNN_ALSA_LIB_DIR} -lasound)
    else()
      target_link_libraries(sherpa-ncnn-alsa asound)
    endif()
    install(TARGETS sherpa-ncnn-alsa DESTINATION bin)
    list(APPEND main_exes sherpa-ncnn-alsa)
  endif()

  if(SHERPA_NCNN_ENABLE_PORTAUDIO)
    add_executable(sherpa-ncnn-microphone
      sherpa-ncnn-microphone.cc
      microphone.cc
    )
    list(APPEND main_exes sherpa-ncnn-microphone)

    if(BUILD_SHARED_LIBS)
      set(PA_LIB portaudio)
    else()
      set(PA_LIB portaudio_static)
    endif()

    target_link_libraries(sherpa-ncnn-microphone ${PA_LIB} sherpa-ncnn-core)

    install(TARGETS sherpa-ncnn-microphone DESTINATION bin)
  endif()

  if(SHERPA_NCNN_ENABLE_GENERATE_INT8_SCALE_TABLE)
    add_executable(generate-int8-scale-table generate-int8-scale-table.cc)
    target_link_libraries(generate-int8-scale-table sherpa-ncnn-core)
  endif()

  if(NOT WIN32)
    foreach(exe IN LISTS main_exes)
      target_link_libraries(${exe} "-Wl,-rpath,${SHERPA_NCNN_RPATH_ORIGIN}/../lib")
      target_link_libraries(${exe} "-Wl,-rpath,${SHERPA_NCNN_RPATH_ORIGIN}/../../../sherpa_ncnn/lib")

      if(SHERPA_NCNN_ENABLE_PYTHON)
        target_link_libraries(${exe} "-Wl,-rpath,${SHERPA_NCNN_RPATH_ORIGIN}/../lib/python${PYTHON_VERSION}/site-packages/sherpa_ncnn/lib")
      endif()
    endforeach()
  endif()
endif()

if(SHERPA_NCNN_ENABLE_TEST)
  add_executable(test-resample test-resample.cc)
  target_link_libraries(test-resample sherpa-ncnn-core)
endif()
