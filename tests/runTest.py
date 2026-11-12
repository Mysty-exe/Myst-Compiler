import os
import subprocess
import sys

compiler = sys.argv[1]
source_file = sys.argv[2]
expected_file = sys.argv[3]

# Check if Valgrind should be used on the compiler
use_valgrind = os.environ.get("USE_VALGRIND", "0") == "1"

# Determine the generated binary path based on how your compiler names it
base_name = os.path.splitext(os.path.basename(source_file))[0]
generated_binary = "../tests/integrations/" + base_name

if os.path.exists(generated_binary):
    os.remove(generated_binary)

# Phase 1: Compile (Run compiler, optionally wrapped in Valgrind)
if use_valgrind:
    compile_cmd = [
        "valgrind",
        "--leak-check=full",
        "--error-exitcode=1",
        compiler,
        source_file,
    ]
else:
    compile_cmd = [compiler, source_file]

compile_result = subprocess.run(compile_cmd, capture_output=True, text=True)

if compile_result.returncode != 0:
    print(f"FAILED: Compilation or Valgrind error for {source_file}")
    print(compile_result.stderr)
    sys.exit(1)

if not os.path.exists(generated_binary):
    print(f"FAILED: Compiler did not produce an executable at {generated_binary}")
    sys.exit(1)

# Phase 2: Run the generated binary to check its stdout
run_result = subprocess.run([generated_binary], capture_output=True, text=True)

if run_result.returncode != 0:
    print(f"FAILED: Generated binary crashed with exit code {run_result.returncode}")
    print(run_result.stderr)
    sys.exit(1)

# Phase 3: Compare standard output against the expected .out file
actual_output = run_result.stdout.strip()

with open(expected_file, "r") as f:
    expected_output = f.read().strip()

# Cleanup binary
if os.path.exists(generated_binary):
    os.remove(generated_binary)

if actual_output == expected_output:
    print(f"PASSED: {source_file}")
    sys.exit(0)
else:
    print(f"FAILED: Output mismatch for {source_file}")
    print(f"Expected:\n{expected_output}")
    print(f"Got:\n{actual_output}")
    sys.exit(1)