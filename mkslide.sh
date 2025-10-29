#!/usr/bin/env bash

# 0 - stdin
# 1 - stdout
# 2 - stderr

exec 4>&1 # stdout duplicado ( FD 4 tem as mesmas caracteristicas
# do STDOUT), aponta para o terminal.

exec 1>slide.md # stdout aponta para `out.txt'

#echo Estou escrevendo no arquivo.               # echo "TEXTO" > out.txt
#echo Estou escrevendo no terminal. >&4          # echo "TEXTO"
#echo Estou escrevendo no arquivo novamente.     # echo "TEXTO" > out.txt

cat <<EOF
---
title:  ponteiros em C Ansi
author: Ivan Lopes (lopesivan.ufrj@gmail.com)
date:   2021-11-22
---

descrição

EOF

prefix=ex-
ls ${prefix}*.c | sort -n -t'-' -k2 |
    sed \
        -e 'h' \
        -e 'g;s/^/\n---\n# /' \
        -e 'p' \
        -e 'i\`\`\`file' \
        -e 'x;s/^/path: .\//' \
        -e 'alang: c\n\`\`\`'

cat <<EOF

---
# Contato

**lopesivan.ufrj**@gmail.com

/* -*- vim: set ts=4 sw=4 tw=64 ft=markdown: -*- */
EOF

echo lookatme -e file_loader slide.md >&4

exec 1>&4 # FD 1 recebe as caracteristicas de FD 4.
# FD 1 ressetado.

exec 4>&- # FD 4 eh liberado.

exit 0
