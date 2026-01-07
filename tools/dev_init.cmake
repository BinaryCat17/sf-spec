# SionFlow Developer Auto-Touch (Shared from sf-spec)
# This script ensures vcpkg rebuilds local ports when source code changes.
execute_process(COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/dev_touch.py)
