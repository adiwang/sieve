#!/bin/sh

IMAGE_DIR=/home/hzfy/dist/images

find $IMAGE_DIR -mtime +7 -name "*.*" -exec rm -rf {} \;
