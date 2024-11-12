file(REMOVE_RECURSE
  "../../../build/lib/libns3.42-ai-default.pdb"
  "../../../build/lib/libns3.42-ai-default.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/libai.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
