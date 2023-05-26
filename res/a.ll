; ModuleID = 'minisolc'
source_filename = "minisolc"

%st = type { i32, double }

@0 = private unnamed_addr constant [5 x i8] c"%lf\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca %st, align 8
  %1 = getelementptr inbounds %st, %st* %0, i32 0, i32 1
  %2 = load double, double* %1, align 8
  store double 1.000000e+00, double* %1, align 8
  %3 = getelementptr inbounds %st, %st* %0, i32 0, i32 1
  %4 = load double, double* %3, align 8
  %5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @0, i32 0, i32 0), double %4)
  ret i32 0
}
