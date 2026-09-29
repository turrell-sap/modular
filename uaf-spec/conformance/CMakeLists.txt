# UAF-SPEC-001 v2.1 conformance suite.
#
# v2.0's Section 12 declared UAF_BUILD_TESTS and defined no tests. This
# builds the reference pieces the specification pins -- wire codec, CRC32C,
# ring discipline, key model, QP state machine, DST mapping, CM tie-break --
# and runs the eleven conformance cases of Section 12.3 against them.
cmake_minimum_required(VERSION 3.20)
project(uaf_conformance VERSION 2.1.4 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(UAF_BUILD_TESTS "Build and register the conformance suite" ON)

add_library(uaf_ref STATIC
    src/uaf_crc32c.c
    src/uaf_wire.c
    src/uaf_conn.c
    src/uaf_ring.c
    src/uaf_dst.c
    src/uaf_rkey.c
    src/uaf_qp_state.c
    src/uaf_cm.c
    src/uaf_rmt_cq.c
)
target_include_directories(uaf_ref PUBLIC include)
target_compile_options(uaf_ref PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror)

if(UAF_BUILD_TESTS)
    enable_testing()
    # Each name is a conformance case in Section 12.3.
    set(UAF_CONFORMANCE_CASES
        abi           # C-1  ABI layout is pinned
        crc32c        # C-2  CRC32C known answers and the golden header
        wire          # C-3  encode/decode, framing, bad magic, bad CRC
        qp_state      # C-4  legal and illegal QP transitions, attr masks
        rkey          # C-5  key generation, binding, bounds, failure throttle
        ring          # C-6  power-of-two depth, full ring, wrap
        cqe_phase     # C-7  completion phase tag, 64-bit wr_id restoration
        dst_map       # C-8  NVMe opcode map, NLB, PRP/SGL, cmd_id, status map
        conn_wire     # C-9  connection exchange serialisation
        udp_loopback  # C-10 end-to-end RDMA WRITE over UDP
        cm            # C-11 CM sizing, MAC input, simultaneous open
        rmt_cq        # C-12 intra-host RMT completion ring (UAF-D)
    )
    foreach(case IN LISTS UAF_CONFORMANCE_CASES)
        add_executable(test_${case} tests/test_${case}.c)
        target_link_libraries(test_${case} PRIVATE uaf_ref)
        target_include_directories(test_${case} PRIVATE include tests)
        target_compile_options(test_${case} PRIVATE
            -Wall -Wextra -Wpedantic -Werror)
        add_test(NAME C-${case} COMMAND test_${case})
    endforeach()
endif()
