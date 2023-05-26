; ModuleID = 'minisolc'
source_filename = "minisolc"

%st = type { i32, double }

@0 = private unnamed_addr constant [5 x i8] c"%lf\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca %st, align 8
  %1 = alloca %st, align 8
  %2 = getelementptr inbounds %st, %st* %0, i32 0, i32 1
  %3 = load double, double* %2, align 8
  store double 1.000000e+00, double* %2, align 8
  %4 = load %st, %st* %0, align 8
  store %st %4, %st* %1, align 8
  %5 = getelementptr inbounds %st, %st* %1, i32 0, i32 1
  %6 = load double, double* %5, align 8
  %7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @0, i32 0, i32 0), double %6)
  %8 = alloca i32, align 4
  store i32 10, i32* %8, align 4
  %9 = load i32, i32* %8, align 4
  %10 = shl i32 %9, 3
  store i32 %10, i32* %8, align 4
  %11 = load i32, i32* %8, align 4
  %12 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i32 %11)
  ret i32 0
}
