#!/bin/sh

TAR="mda-vst-bin-lin.tar"

tar -cvvf $TAR *.so
gzip --best $TAR
