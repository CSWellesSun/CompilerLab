; ModuleID = 'minisolc'
source_filename = "minisolc"

@0 = private unnamed_addr constant [13 x i8] c"Hello World!\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @scanf(...)

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = alloca i8*, align 8
  store i8* getelementptr inbounds ([13 x i8], [13 x i8]* @0, i32 0, i32 0), i8** %0, align 8
  %1 = load i8*, i8** %0, align 8
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i8* %1)
  %3 = alloca i32, i32 10, align 4
  %4 = getelementptr inbounds i32, i32* %3, i32 1
  %5 = load i32, i32* %4, align 4
  store i32 123, i32* %4, align 4
  %6 = getelementptr inbounds i32, i32* %3, i32 1
  %7 = load i32, i32* %6, align 4
  %8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i32 %7)
  ret i32 0
}
