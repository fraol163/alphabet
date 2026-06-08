" Alphabet Language Syntax
" Place in ~/.vim/syntax/alphabet.vim

if exists("b:current_syntax")
  finish
endif

syn keyword alphabetKeyword i e l b k r c m n v p s t h x q a j
syn keyword alphabetType int str bool list map float void i8 i16 i32 i64 f32 f64 const
syn keyword alphabetBoolean true false
syn match alphabetSystem "z\.\w\+"
syn region alphabetString start=+"+ end=+"+ contains=alphabetEscape
syn match alphabetEscape "\\[nt\\\"0]" contained
syn match alphabetComment "//.*$"
syn match alphabetNumber "\d\+\(\.\d\+\)\?"
syn match alphabetHeader "#alphabet<[^>]*>"

hi def link alphabetKeyword Keyword
hi def link alphabetType Type
hi def link alphabetBoolean Boolean
hi def link alphabetString String
hi def link alphabetEscape Special
hi def link alphabetComment Comment
hi def link alphabetNumber Number
hi def link alphabetHeader PreProc
hi def link alphabetSystem Function

let b:current_syntax = "alphabet"
