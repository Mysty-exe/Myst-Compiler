#include <compiler/llvm.h>

std::string getTargetTriple()
{
#if defined(_WIN32)
    return "x86_64-pc-windows-msvc";
#elif defined(__linux__)
    return "x86_64-pc-linux-gnu";
#else
    return "x86_64-unknown-unknown";
#endif
}

void addRuntimeErrors(std::string &ir)
{
    ir += R"(@div_by_zero_msg = private unnamed_addr constant [33 x i8] c"Runtime Error: Division by zero\0A\00", align 1

define void @print_runtime_error(ptr %msg) {
    call i32 (ptr, ...) @printf(ptr %msg)
    ret void
})";
}

void addPrintFunctions(std::string &ir)
{
    ir += R"(declare i32 @printf(ptr, ...)

@char_fmt = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@int_fmt    = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@double_fmt = private unnamed_addr constant [4 x i8] c"%lf\00", align 1

define void @print_char(i8 %val) {
    %val_ext = zext i8 %val to i32
    %nl_ext = zext i8 10 to i32    
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %val_ext)
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %nl_ext)
    ret void
}

define void @print_char_end(i8 %val1, i8 %val2) {
    %val1_ext = zext i8 %val1 to i32    
    %val2_ext = zext i8 %val2 to i32    
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %val1_ext)
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %val2_ext)
    ret void
}

define void @print_int(i32 %val) {
    call i32 (ptr, ...) @printf(ptr @int_fmt, i32 %val)

    %nl_ext = zext i8 10 to i32
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %nl_ext)

    ret void
}

define void @print_int_end(i32 %val1, i8 %val2) {
    call i32 (ptr, ...) @printf(ptr @int_fmt, i32 %val1)

    %val2_ext = zext i8 %val2 to i32
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %val2_ext)

    ret void
}

define void @print_double(double %val) {
    call i32 (ptr, ...) @printf(ptr @double_fmt, double %val)

    %nl_ext = zext i8 10 to i32
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %nl_ext)
    
    ret void
}

define void @print_double_end(double %val1, i8 %val2) {
    call i32 (ptr, ...) @printf(ptr @double_fmt, double %val1)

    %val2_ext = zext i8 %val2 to i32
    call i32 (ptr, ...) @printf(ptr @char_fmt, i32 %val2_ext)

    ret void
}
)";
}

void addExponentFunctions(std::string &ir)
{
    ir += R"(
declare double @llvm.pow.f64(double, double)

define i64 @pow_int(i64 %base, i64 %exp) {
entry:
    %cmp_zero = icmp sle i64 %exp, 0
    br i1 %cmp_zero, label %exit, label %loop

loop:
    %res = phi i64 [ 1, %entry ], [ %res_mul, %loop ]
    %i = phi i64 [ 0, %entry ], [ %i_next, %loop ]
    
    %res_mul = mul i64 %res, %base
    %i_next = add i64 %i, 1
    
    %done = icmp eq i64 %i_next, %exp
    br i1 %done, label %exit, label %loop

exit:
    %final = phi i64 [ 1, %entry ], [ %res_mul, %loop ]
    ret i64 %final
}

define double @pow_double(double %base, double %exp) {
entry:
    ; Perform the power calculation using the intrinsic
    %result = call double @llvm.pow.f64(double %base, double %exp)
    ret double %result
}
)";
}

void addExitFunction(std::string &ir)
{
#if defined(_WIN32)
    ir += R"(declare void @ExitProcess(i32)

define void @exit(i64 %status) {
    ; Cast the 64-bit status down to a 32-bit integer for Windows
    %status32 = trunc i64 %status to i32
    
    ; Call Windows ExitProcess
    call void @ExitProcess(i32 %status32)
    unreachable
}
)";
#else
    ir += R"(define void @exit(i64 %status) {
    call void asm sideeffect "syscall", "{rax},{rdi},~{dirflag},~{fpsr},~{flags},~{memory}" (
        i64 60,
        i64 %status
    )
    unreachable
}

)";
#endif
}

std::string execCommand(const std::string &cmd)
{
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}

void LLVM::runLLVM(const std::string &outputFile, RootNode *root)
{
    mapToIR(root);
    generateExecutable(outputFile);
}

void LLVM::mapToIR(RootNode *root)
{
    std::vector<std::vector<std::vector<std::string>>> tempvarMap = {{}};
    std::ofstream tempFile("temp.ll");
    if (!tempFile.is_open())
    {
        std::cerr << "\033[31m";
        std::cerr << "Error: ";
        std::cerr << "\033[0m";
        std::cout << "Couldn't open intermediate artifact." << std::endl;
    }

    std::string ir;
    addExitFunction(ir);
    addRuntimeErrors(ir);
    addPrintFunctions(ir);
    addExponentFunctions(ir);

    BlockNode *global = static_cast<BlockNode *>(root->getGlobalCode());
    global->mapToGlobalIR(ir);

    for (Node *node : root->getChildren())
    {
        FuncNode *function = static_cast<FuncNode *>(node);
        function->mapToIR(ir, tempvarMap, false);
    }

    global->mapToIR(ir, tempvarMap, true);
    tempFile << "target triple = \"" << getTargetTriple() << "\"\n\n";
    tempFile << ir;
    tempFile.close();
}

void LLVM::generateExecutable(const std::string &outputFile)
{
    try
    {
#if defined(_WIN32)
        std::string output = execCommand("clang -Wno-override-module temp.ll -o " + outputFile + ".exe");
#else
        std::string output = execCommand("clang -Wno-override-module temp.ll -o " + outputFile + " -lm");
#endif
        std::cout << output;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // std::filesystem::remove("temp.ll");
}
