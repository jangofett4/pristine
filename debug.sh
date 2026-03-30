#!/bin/sh
set -e
. ./toolchain.sh
bear --append -- make DEBUG=1 all
make debug