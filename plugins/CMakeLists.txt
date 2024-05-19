add_subdirectory(first_party)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/user_defined/CMakeLists.txt)
	add_subdirectory(user_defined)
else()
	message("No user defined plugins found")
endif()

# at the moment this will always include the user_defined subdirectory, but when we have a more
# refined solution where it is possible to have plugins with their own git tracking then thiongs might change