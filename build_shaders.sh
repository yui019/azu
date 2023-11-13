#!/bin/sh

cd shaders
glslc -c *.vert
glslc -c *.frag

cd ..