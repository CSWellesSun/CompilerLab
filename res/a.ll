; ModuleID = 'minisolc'
source_filename = "minisolc"

@0 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca i32, i32 10, align 4
  %1 = getelementptr inbounds i32, i32* %0, i32 0
  %2 = load i32, i32* %1, align 4
  store i32 1, i32* %1, align 4
  %3 = getelementptr inbounds i32, i32* %0, i32 1
  %4 = load i32, i32* %3, align 4
  store i32 1, i32* %3, align 4
  %5 = alloca i32, align 4
  store i32 2, i32* %5, align 4
  %6 = load i32, i32* %5, align 4
  %7 = icmp ult i32 %6, 10
  br i1 %7, label %8, label %25

8:                                                ; preds = %8, %entry
  %9 = load i32, i32* %5, align 4
  %10 = getelementptr inbounds i32, i32* %0, i32 %9
  %11 = load i32, i32* %10, align 4
  %12 = load i32, i32* %5, align 4
  %13 = sub i32 %12, 1
  %14 = getelementptr inbounds i32, i32* %0, i32 %13
  %15 = load i32, i32* %14, align 4
  %16 = load i32, i32* %5, align 4
  %17 = sub i32 %16, 2
  %18 = getelementptr inbounds i32, i32* %0, i32 %17
  %19 = load i32, i32* %18, align 4
  %20 = add i32 %15, %19
  store i32 %20, i32* %10, align 4
  %21 = load i32, i32* %5, align 4
  %22 = add i32 %21, 1
  store i32 %22, i32* %5, align 4
  %23 = load i32, i32* %5, align 4
  %24 = icmp ult i32 %23, 10
  br i1 %24, label %8, label %25

25:                                               ; preds = %8, %entry
  %26 = alloca i32, align 4
  store i32 0, i32* %26, align 4
  %27 = load i32, i32* %26, align 4
  %28 = icmp ult i32 %27, 10
  br i1 %28, label %29, label %38

29:                                               ; preds = %29, %25
  %30 = load i32, i32* %26, align 4
  %31 = getelementptr inbounds i32, i32* %0, i32 %30
  %32 = load i32, i32* %31, align 4
  %33 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @0, i32 0, i32 0), i32 %32)
  %34 = load i32, i32* %26, align 4
  %35 = add i32 %34, 1
  store i32 %35, i32* %26, align 4
  %36 = load i32, i32* %26, align 4
  %37 = icmp ult i32 %36, 10
  br i1 %37, label %29, label %38

38:                                               ; preds = %29, %25
  %39 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0))
  ret i32 0
}
