; ModuleID = 'minisolc'
source_filename = "minisolc"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define void @foo(i32 %day) {
entry:
  %0 = alloca i32, align 4
  store i32 0, i32* %0, align 4
  store i32 %day, i32* %0, align 4
  %1 = load i32, i32* %0, align 4
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %1)
  ret void
}

define i32 @main() {
entry:
  %0 = alloca i32, align 4
  store i32 5, i32* %0, align 4
  %1 = load i32, i32* %0, align 4
  call void @foo(i32 %1)
  ret i32 0
}
