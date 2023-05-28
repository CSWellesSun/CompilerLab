; ModuleID = 'minisolc'
source_filename = "minisolc"

%person = type { i8*, i32, double }

@0 = private unnamed_addr constant [16 x i8] c"Suzumiya Haruhi\00", align 1
@1 = private unnamed_addr constant [34 x i8] c"name = %s\0Aage = %d\0Aheight = %.2f\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca %person, align 8
  %1 = getelementptr inbounds %person, %person* %0, i32 0, i32 0
  %2 = load i8*, i8** %1, align 8
  store i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i32 0, i32 0), i8** %1, align 8
  %3 = getelementptr inbounds %person, %person* %0, i32 0, i32 1
  %4 = load i32, i32* %3, align 4
  store i32 16, i32* %3, align 4
  %5 = getelementptr inbounds %person, %person* %0, i32 0, i32 2
  %6 = load double, double* %5, align 8
  store double 1.580000e+02, double* %5, align 8
  %7 = alloca %person, align 8
  %8 = load %person, %person* %0, align 8
  store %person %8, %person* %7, align 8
  %9 = getelementptr inbounds %person, %person* %7, i32 0, i32 0
  %10 = load i8*, i8** %9, align 8
  %11 = getelementptr inbounds %person, %person* %7, i32 0, i32 1
  %12 = load i32, i32* %11, align 4
  %13 = getelementptr inbounds %person, %person* %7, i32 0, i32 2
  %14 = load double, double* %13, align 8
  %15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([34 x i8], [34 x i8]* @1, i32 0, i32 0), i8* %10, i32 %12, double %14)
  ret i32 0
}
