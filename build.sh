#!/bin/bash
clang main.c transform.c complex.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
