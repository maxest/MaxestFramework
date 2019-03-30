TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += object_parallel_to_source

DEFINES += "MAXEST_FRAMEWORK_DESKTOP"
DEFINES += "MAXEST_FRAMEWORK_OPENGL"

LIBS += -lpthread -lGL
LIBS += "../../../../dependencies/SDL2-2.0/lib/libSDL2.so"
LIBS += "../../../../dependencies/glew-2.1.0/lib/libGLEW.a"
LIBS += "../../../../dependencies/FreeImage-3.17.0/lib/libfreeimage.a"
LIBS += "../../../../dependencies/squish-1.15/lib/libsquish.a"

SOURCES += \
    ../../src/main.cpp \
    ../../src/ray_tracer.cpp \
    ../../../../src/system/application.cpp \
    ../../../../src/system/main.cpp \
    ../../../../src/net/connections_manager.cpp \
    ../../../../src/mesh/mesh.cpp \
    ../../../../src/mesh/mesh_ase.cpp \
    ../../../../src/mesh/mesh_obj.cpp \
    ../../../../src/mesh/mesh_unity_exported.cpp \
    ../../../../src/math/matrix.cpp \
    ../../../../src/math/polygon.cpp \
    ../../../../src/image/conversion.cpp \
    ../../../../src/image/filtering.cpp \
    ../../../../src/image/fourier.cpp \
    ../../../../src/image/image.cpp \
    ../../../../src/image/main.cpp \
    ../../../../src/gpu/d3d11.cpp \
    ../../../../src/gpu/profiler.cpp \
    ../../../../src/gpu/utils.cpp \
    ../../../../src/common/jobs.cpp \
    ../../../../src/common/lossless_compression.cpp \
    ../../../../src/common/neural_network.cpp \
    ../../src/scene.cpp

HEADERS += \
    ../../src/brdf.h \
    ../../src/camera.h \
    ../../src/material.h \
    ../../src/ray_tracer.h \
    ../../src/sampler.h \
    ../../src/scene.h \
    ../../src/types.h \
    ../../src/utils.h \
    ../../../../src/main.h \
    ../../../../src/namespaces.h \
    ../../../../src/system/application.h \
    ../../../../src/system/common.h \
    ../../../../src/system/keys.h \
    ../../../../src/system/main.h \
    ../../../../src/system/sockets.h \
    ../../../../src/physics/main.h \
    ../../../../src/physics/rigid_body.h \
    ../../../../src/net/connections_manager.h \
    ../../../../src/net/main.h \
    ../../../../src/net/packet_manager.h \
    ../../../../src/net/web_socket.h \
    ../../../../src/mesh/main.h \
    ../../../../src/mesh/mesh.h \
    ../../../../src/mesh/types.h \
    ../../../../src/math/bounding_box.h \
    ../../../../src/math/camera.h \
    ../../../../src/math/color.h \
    ../../../../src/math/common.h \
    ../../../../src/math/constants.h \
    ../../../../src/math/conversion.h \
    ../../../../src/math/distance.h \
    ../../../../src/math/intersection.h \
    ../../../../src/math/io.h \
    ../../../../src/math/main.h \
    ../../../../src/math/matrix.h \
    ../../../../src/math/plane.h \
    ../../../../src/math/polygon.h \
    ../../../../src/math/probability.h \
    ../../../../src/math/quaternion.h \
    ../../../../src/math/random.h \
    ../../../../src/math/samples.h \
    ../../../../src/math/solver.h \
    ../../../../src/math/stats.h \
    ../../../../src/math/transform.h \
    ../../../../src/math/types.h \
    ../../../../src/math/vector.h \
    ../../../../src/image/conversion.h \
    ../../../../src/image/filtering.h \
    ../../../../src/image/fourier.h \
    ../../../../src/image/image.h \
    ../../../../src/image/main.h \
    ../../../../src/image/types.h \
    ../../../../src/gpu/d3d11.h \
    ../../../../src/gpu/main.h \
    ../../../../src/gpu/ogl.h \
    ../../../../src/gpu/profiler.h \
    ../../../../src/gpu/types.h \
    ../../../../src/gpu/utils.h \
    ../../../../src/essentials/array.h \
    ../../../../src/essentials/assert.h \
    ../../../../src/essentials/bit.h \
    ../../../../src/essentials/coding.h \
    ../../../../src/essentials/common.h \
    ../../../../src/essentials/file.h \
    ../../../../src/essentials/hash.h \
    ../../../../src/essentials/macros.h \
    ../../../../src/essentials/main.h \
    ../../../../src/essentials/stl.h \
    ../../../../src/essentials/string.h \
    ../../../../src/essentials/threads.h \
    ../../../../src/essentials/types.h \
    ../../../../src/common/config_file.h \
    ../../../../src/common/jobs.h \
    ../../../../src/common/lossless_compression.h \
    ../../../../src/common/main.h \
    ../../../../src/common/neural_network.h
