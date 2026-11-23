import os
import platform
import re
import subprocess
import sys
import tempfile
from pathlib import Path


def strip_ansi(text: str) -> str:
    """Removes ANSI color escape sequences like \x1b[31m and \x1b[0m."""
    ansi_escape = re.compile(r"\x1b\[[0-9;]*m")
    return ansi_escape.sub("", text)


def check_valgrind_log(log_text: str) -> bool:
    """Returns True if Valgrind reported any memory errors or leaks."""
    if not log_text:
        return False
    has_errors = re.search(r"ERROR SUMMARY:\s+[1-9]\d*\s+errors", log_text) is not None
    has_leaks = (
        re.search(r"(?:definitely|indirectly|possibly) lost:\s+[1-9]", log_text)
        is not None
    )
    return has_errors or has_leaks


# Extract flags (e.g., --expect-error) vs positional file path arguments
flags = [a for a in sys.argv[1:] if a.startswith("--")]
pos_args = [a for a in sys.argv[1:] if not a.startswith("--")]

if len(pos_args) < 3:
    print(
        "Usage: python runTest.py [--expect-error] <compiler> <source_file> <expected_file>"
    )
    sys.exit(1)

# Resolve all paths relative to current environment
compiler_path = Path(pos_args[0]).resolve()
source_path = Path(pos_args[1]).resolve()
expected_path = Path(pos_args[2]).resolve()

# Determine if this is an expected-error test
is_expect_error = "--expect-error" in flags or "errors" in source_path.parts

# Check if Valgrind should be used on the compiler
use_valgrind = os.environ.get("USE_VALGRIND", "0") == "1"

# Automatically infer binary destination relative to the source file's directory
base_name = source_path.stem
exe_suffix = ".exe" if platform.system() == "Windows" else ""
generated_binary = source_path.with_name(base_name + exe_suffix)

# Read expected output file
if not expected_path.exists():
    print(f"FAILED: Expected output file not found at {expected_path}")
    sys.exit(1)

expected_output = expected_path.read_text(encoding="utf-8").strip()

# Phase 1: Compile
valgrind_log_path = None
if use_valgrind:
    tmp_log = tempfile.NamedTemporaryFile(delete=False, suffix=".log")
    tmp_log.close()
    valgrind_log_path = Path(tmp_log.name)
    compile_cmd = [
        "valgrind",
        "--leak-check=full",
        "--error-exitcode=1",
        f"--log-file={valgrind_log_path}",
        str(compiler_path),
        str(source_path),
    ]
else:
    compile_cmd = [str(compiler_path), str(source_path)]

try:
    compile_result = subprocess.run(compile_cmd, capture_output=True, text=True)
    valgrind_log = (
        valgrind_log_path.read_text(encoding="utf-8", errors="replace")
        if valgrind_log_path and valgrind_log_path.exists()
        else ""
    )
finally:
    if valgrind_log_path and valgrind_log_path.exists():
        valgrind_log_path.unlink()

try:
    # Phase 2A: Handle Expected Semantic Error Test
    if is_expect_error:
        if use_valgrind and check_valgrind_log(valgrind_log):
            print(f"FAILED: Valgrind error/leak for {source_path.name}")
            print(valgrind_log.strip())
            sys.exit(1)

        if compile_result.returncode == 0:
            print(
                f"FAILED: Expected semantic error for {source_path.name}, but compilation succeeded!"
            )
            sys.exit(1)

        actual_error = (compile_result.stdout + compile_result.stderr).strip()

        # 1. Strip ANSI color escape codes (\x1b[31m etc.)
        actual_error_clean = strip_ansi(actual_error)

        # 2. Normalize absolute paths down to filename
        actual_error_clean = actual_error_clean.replace(
            str(source_path), source_path.name
        )
        actual_error_clean = actual_error_clean.replace(
            str(source_path.resolve()), source_path.name
        )

        # 3. Normalize path slashes across OS platforms
        actual_error_clean = actual_error_clean.replace("\\", "/").strip()
        expected_output_clean = expected_output.replace("\\", "/").strip()

        if actual_error_clean == expected_output_clean:
            print(f"PASSED (Expected Error): {source_path.name}")
            sys.exit(0)
        else:
            print(f"FAILED: Error message mismatch for {source_path.name}")
            print(f"Reading expected from: {expected_path}")
            print("--- Expected ---")
            print(repr(expected_output_clean))
            print("--- Got ---")
            print(repr(actual_error_clean))
            sys.exit(1)

    # Phase 2B: Handle Normal Compilation Test
    if compile_result.returncode != 0:
        print(f"FAILED: Compilation or Valgrind error for {source_path.name}")
        if compile_result.stderr:
            print(compile_result.stderr.strip())
        if use_valgrind and valgrind_log.strip():
            print(valgrind_log.strip())
        sys.exit(1)

    if not generated_binary.exists():
        print(
            f"FAILED: Compiler did not produce an executable at {generated_binary}"
        )
        sys.exit(1)

    # Phase 3: Run the generated binary to check its stdout
    run_result = subprocess.run([str(generated_binary)], capture_output=True, text=True)

    if run_result.returncode != 0:
        print(
            f"FAILED: Generated binary crashed with exit code {run_result.returncode}"
        )
        if run_result.stderr:
            print(run_result.stderr.strip())
        sys.exit(1)

    actual_output = run_result.stdout.strip()

    if actual_output == expected_output:
        print(f"PASSED: {source_path.name}")
        sys.exit(0)
    else:
        print(f"FAILED: Output mismatch for {source_path.name}")
        print(f"Reading expected from: {expected_path}")
        print("--- Expected ---")
        print(repr(expected_output))
        print("--- Got ---")
        print(repr(actual_output))
        sys.exit(1)
finally:
    if generated_binary.exists():
        generated_binary.unlink()