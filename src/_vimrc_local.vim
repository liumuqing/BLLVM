"let g:ale_cpp_clangd_options = '--completion-style=detailed --clang-tidy'
let g:ale_cpp_clangd_executable= 'clangd-11'
let g:ale_cpp_clang_options = '-std=c++20 -I../binaryninja-api/ -I../3rd_party/argparse/include -I../3rd_party/plog/include'
"let g:ale_cpp_cc_options = '-std=c++20 -I../binaryninja-api/ -I../3rd_party/argparse/include'
"let g:ale_cpp_cc_executable = 'clang++'
"let g:ale_cpp_clangtidy_options = '-std=c++20 -I../binaryninja-api/ -I../3rd_party/argparse/include'
let g:ale_linters_explicit = 1
let g:ale_pattern_options_enabled = 1
let g:ale_linters = {}
let g:ale_pattern_options = {
      \   '\.cpp$': {
      \       'ale_linters': ['clangd', 'clang-format'],
      \   },
      \   '\.hpp$': {
      \       'ale_linters': ['clangd', 'clang-format'],
      \   },
	  \}
"let g:ale_fixers = {
"    \ '*': ['clangtidy'],
"    \ }
