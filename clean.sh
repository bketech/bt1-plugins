#!/bin/bash

set -e
for pat in *.dylib *.so *.o *.a *.obj; do
  set -x
  find . -name "${pat}" -delete
  set +x
done
