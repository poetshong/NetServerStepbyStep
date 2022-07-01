#!/bin/bash
echo "*.cpp文件行数:"
cat src/*.cpp | wc -l
echo "*.h文件行数:"
cat src/*.h | wc -l
