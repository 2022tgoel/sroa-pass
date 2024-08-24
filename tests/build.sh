for file in "src"/*.cpp; do
    # Check if the file actually exists
    if [ -f "$file" ]; then
        
        # Run clang to generate the LLVM IR in text format
        clang -emit-llvm -O0 -S "$file"
        
        echo "Generated LLVM IR for $file"
    else
        echo "No .cpp files found"
    fi
done