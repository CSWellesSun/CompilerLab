; ModuleID = 'minisolc'
source_filename = "minisolc"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca i32, align 4
  store i32 0, i32* %0, align 4
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %2

2:                                                ; preds = %2, %entry
  %3 = load i32, i32* %0, align 4
  %4 = add i32 %3, 1
  store i32 %4, i32* %0, align 4
  %5 = load i32, i32* %1, align 4
  %6 = load i32, i32* %0, align 4
  %7 = add i32 %5, %6
  store i32 %7, i32* %1, align 4
  %8 = load i32, i32* %0, align 4
  %9 = icmp ult i32 %8, 10
  br i1 %9, label %2, label %10

10:                                               ; preds = %2
  %11 = load i32, i32* %1, align 4
  %12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %11)
  ret i32 0
}
