# on windows we need to make all the drive letters (C:) to lowercase, so clangd can actually fully work
# without this ATLEAST the renames from clangd won't work
file(READ ${CMAKE_BINARY_DIR}/compile_commands.json COMPILE_COMMANDS)
string(REGEX REPLACE "C:" "c:" COMPILE_COMMANDS "${COMPILE_COMMANDS}")
file(WRITE ${CMAKE_BINARY_DIR}/compile_commands.json "${COMPILE_COMMANDS}")