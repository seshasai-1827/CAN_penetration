cmake_minimum_required(VERSION 3.16)

project(SocketCAN_ISOTP_Test
    VERSION 1.0
    LANGUAGES C
)

# Use the C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Compiler warnings
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# ISO-TP receiver
add_executable(isotp_receiver
    isotp_reciever.c
)

# ISO-TP sender
add_executable(isotp_sender
    isotp_sender.c
)

# Raw CAN monitor
add_executable(raw_monitor
    raw_monitor.c
)

# Malformed-frame test utility
add_executable(malformed_test
    malformed_test.c
)

# Linux SocketCAN headers
# No external CAN library is required:
# SocketCAN is provided by the Linux kernel API.