---
description: Apply to all source code files in the project to provide consistent coding guidelines and project context for AI-generated content.
applyTo: '**/*.{c,cpp,h,py}'
# applyTo: 'Describe when these instructions should be loaded by the agent based on task context' # when provided, instructions will automatically be added to the request context when the pattern matches an attached file
---

<!-- Tip: Use /create-instructions in chat to generate content with agent assistance -->

- simplicity and readability are priortized
- optimize the code for clarity and maintainability. For performance optimizations, ensure they do not compromise readability unless necessary for critical sections of code.
- all implementations should be aligned to the specified CCSDS standards in main README.md file and the documentation in docs/ folder
- use snake_case for variable and function names
- for function names, use module name as prefix, e.g., `tc_router_route_packet()`
- for struct names, use snake_case with _t suffix, e.g., `tc_router_config_t`
- for enum names, use snake_case with _t suffix, e.g., `tc_router_status_t`
- for macro names, use uppercase with underscores, e.g., `TC_ROUTER_MAX_PACKET_SIZE`
- for file names, use lowercase with underscores, e.g., `tc_router.c`,
- single function should not exceed 50 lines of code
- use the '{' on the same line as the function declaration, e.g., `void tc_router_init() {`
- write in C99 for C code
- use tabs for indentation in C code, with a tab width of 4 spaces
- for python code use also tabs for indentation, with a tab width of 4 spaces
- if the function has descriptive name, do not add redundant comments, e.g., `tc_router_init()` does not need a comment saying "Initialize the TC router"
- for complex logic, add comments explaining the reasoning behind the code, not just what the code does, e.g., "Check if the packet is valid by verifying the APID and length fields, as per the CCSDS standard"

