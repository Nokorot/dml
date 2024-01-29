"Vim syntax file
" Language:	generic configure file
" Maintainer:	Bram Moolenaar <Bram@vim.org>
" Last Change:	2021 May 01

setlocal filetype=dml

syn keyword	dmlTodo	contained TODO FIXME XXX
" Avoid matching "text#text", used in /etc/disktab and /etc/gettytab
syn match	dmlComment	"^#.*" contains=dmlTodo,@Spell
syn match	dmlComment	"\s#.*"ms=s+1 contains=dmlTodo,@Spell
syn region	dmlString	start=+"+ skip=+\\\\\|\\"+ end=+"+ oneline
syn region	dmlString	start=+'+ skip=+\\\\\|\\'+ end=+'+ oneline

" Add additional keywords for mytype
syntax match dmlKeyword "%include"
syntax match dmlKeyword "%if"
syntax match dmlKeyword "%else"
syntax match dmlKeyword "%elseif"
syntax match dmlKeyword "%endif"
syntax match dmlKeyword "%pregen"
syntax match dmlKeyword "%check"
syntax match dmlKeyword "%check\*"

" Define the default highlighting.
" Only used when an item doesn't have highlighting yet
hi def link dmlComment	  Comment
hi def link dmlTodo	      Todo
hi def link dmlString	    String
highlight def link dmlKeyword	  Keyword

" let b:current_syntax = "dml"

