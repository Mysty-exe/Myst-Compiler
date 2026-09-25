dir_path="examples"

if [ "1" = "$1" ]; then
    ./build/mystc "examples/01_hello.ms"
    exit 0
elif [ "2" = "$1" ]; then
    ./build/mystc "examples/02_types_and_vars.ms"
    exit 0
elif [ "3" = "$1" ]; then
    ./build/mystc "examples/03_control_flow.ms"
    echo "working"
    exit 0
elif [ "4" = "$1" ]; then
    ./build/mystc "examples/04_structs.ms"
    echo "working"
    exit 0
elif [ "5" = "$1" ]; then
    ./build/mystc "examples/05_imports.ms"
    exit 0
elif [ "help" = "$1" ]; then
    ./build/mystc "-help"
    exit 0
fi

for file in "$dir_path"/*; do
    if [ -f "$file" ]; then
        ./build/mystc "$file"
        echo
        echo
    fi
done