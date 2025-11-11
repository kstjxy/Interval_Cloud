add_falcor_executable(IntervalCloudSample)

target_sources(IntervalCloudSample PRIVATE
    IntervalCloudSample.cpp
    IntervalCloudSample.h
)

target_copy_shaders(IntervalCloudSample Samples/IntervalCloudSample)

target_source_group(IntervalCloudSample "Samples")
