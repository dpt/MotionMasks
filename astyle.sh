#!/bin/bash
astyle \
  --mode=c \
  --style=ansi \
  --indent=spaces=2 \
  --max-instatement-indent=120 \
  --unpad-paren \
  --pad-oper \
  --pad-header \
  --convert-tabs \
  --align-pointer=name \
  --recursive \
  "*.h" \
  "*.c"
