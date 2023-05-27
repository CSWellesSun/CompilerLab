; ModuleID = 'minisolc'
source_filename = "minisolc"

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define double @a() {
entry:
  %0 = alloca double, align 8
  store double 2.000000e+00, double* %0, align 8
  %1 = load double, double* %0, align 8
  ret double %1
}

define void @main() {
entry:
  %0 = alloca double, align 8
  %1 = call double @a()
  store double %1, double* %0, align 8

2:                                                ; preds = %2
  %3 = load double, double* %0, align 8
  %4 = fcmp ult double %3, i32 10
  br i1 %4, label %2, label %5

5:                                                ; preds = %2
  ret void
}
