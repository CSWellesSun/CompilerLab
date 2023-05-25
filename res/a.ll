; ModuleID = 'minisolc'
source_filename = "minisolc"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca i32, i32 10, align 4
  %1 = getelementptr inbounds i32, i32* %0, i32 0
  %2 = load i32, i32* %1, align 4
  store i32 123, i32* %1, align 4
  %3 = getelementptr inbounds i32, i32* %0, i32 0
  %4 = load i32, i32* %3, align 4
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %4)
  ret i32 0
}
