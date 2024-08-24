LLVM_OPT_PATH=${LLVM_OPT_PATH:-../llvm-project/build/bin/opt}

for file in "tests"/*.ll; do
    # Check if the file actually exists
    if [ -f "$file" ]; then
        echo "Optimizing $file"
        $LLVM_OPT_PATH $file -passes=helloworld -S
    fi
done