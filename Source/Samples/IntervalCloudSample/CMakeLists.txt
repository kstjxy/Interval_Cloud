add_falcor_executable(IntervalCloudSample)

target_sources(IntervalCloudSample PRIVATE
    IntervalCloudSample.cpp
    IntervalCloudSample.h
    passes/ClearPass.cpp
    passes/ClearPass.h
    passes/IntervalPass.cpp
    passes/IntervalPass.h
    passes/DebugViewPass.cpp
    passes/DebugViewPass.h
)

target_copy_shaders(IntervalCloudSample Samples/IntervalCloudSample)

target_source_group(IntervalCloudSample "Samples")
