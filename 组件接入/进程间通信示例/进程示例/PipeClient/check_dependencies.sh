#!/bin/bash
# 检查 PipeClient.so 的运行时依赖脚本

if [ $# -eq 0 ]; then
    echo "Usage: $0 <path_to_libPipeClient.so>"
    echo "Example: $0 ./build/libPipeClient.so"
    exit 1
fi

LIB_PATH="$1"

if [ ! -f "$LIB_PATH" ]; then
    echo "Error: File not found: $LIB_PATH"
    exit 1
fi

echo "Checking dependencies for: $LIB_PATH"
echo "======================================"
echo ""

# 使用 ldd 检查依赖
echo "Dynamic library dependencies:"
ldd "$LIB_PATH" 2>&1

echo ""
echo "======================================"
echo ""

# 检查是否有缺失的库
MISSING=$(ldd "$LIB_PATH" 2>&1 | grep "not found")

if [ -z "$MISSING" ]; then
    echo "✓ All dependencies are satisfied"
    exit 0
else
    echo "✗ Missing dependencies found:"
    echo "$MISSING"
    echo ""
    echo "To install missing libraries:"
    echo "  Ubuntu/Debian: sudo apt-get install <package-name>"
    echo "  CentOS/RHEL:   sudo yum install <package-name>"
    exit 1
fi

