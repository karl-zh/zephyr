
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/protocol.c)
target_sources(app PRIVATE src/pdump.c)
target_sources(app PRIVATE src/lqueue.c)
target_sources(app PRIVATE src/mqtt_pkt.c)
target_include_directories(app PRIVATE $ENV{ZEPHYR_BASE}/samples/v8m-test/ext/inc)
target_link_libraries(app $ENV{ZEPHYR_BASE}/samples/v8m-test/ext/veneers/s_veneers.o)
