#!/bin/sh

fenix-fxc "$@" -o - | fenix-fxi -
