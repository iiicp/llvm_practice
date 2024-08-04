; ModuleID = 'expr'
source_filename = "expr"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %aa = alloca i32, align 4
  store i32 1, ptr %aa, align 4
  %aa1 = load i32, ptr %aa, align 4
  %b = alloca i32, align 4
  store i32 1, ptr %b, align 4
  %b2 = load i32, ptr %b, align 4
  %aa6 = load i32, ptr %aa, align 4
  %0 = icmp ne i32 %aa6, 0
  br i1 %0, label %trueBB, label %nextBB5

nextBB:                                           ; preds = %mergeBB16
  %b18 = load i32, ptr %b, align 4
  %1 = icmp ne i32 %b18, 0
  br i1 %1, label %nextBB17, label %falseBB20

nextBB3:                                          ; preds = %mergeBB13
  %aa14 = load i32, ptr %aa, align 4
  %2 = icmp ne i32 %aa14, 0
  %3 = zext i1 %2 to i32
  br label %mergeBB16

nextBB4:                                          ; preds = %mergeBB10
  %b11 = load i32, ptr %b, align 4
  %4 = icmp ne i32 %b11, 0
  %5 = zext i1 %4 to i32
  br label %mergeBB13

nextBB5:                                          ; preds = %entry
  %b8 = load i32, ptr %b, align 4
  %6 = icmp ne i32 %b8, 0
  br i1 %6, label %nextBB7, label %falseBB

nextBB7:                                          ; preds = %nextBB5
  %aa9 = load i32, ptr %aa, align 4
  %7 = icmp ne i32 %aa9, 0
  %8 = zext i1 %7 to i32
  br label %mergeBB

falseBB:                                          ; preds = %nextBB5
  br label %mergeBB

mergeBB:                                          ; preds = %falseBB, %nextBB7
  %9 = phi i32 [ %8, %nextBB7 ], [ 0, %falseBB ]
  %10 = icmp ne i32 %9, 0
  %11 = zext i1 %10 to i32
  br label %mergeBB10

trueBB:                                           ; preds = %entry
  br label %mergeBB10

mergeBB10:                                        ; preds = %trueBB, %mergeBB
  %12 = phi i32 [ %11, %mergeBB ], [ 1, %trueBB ]
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %trueBB12, label %nextBB4

trueBB12:                                         ; preds = %mergeBB10
  br label %mergeBB13

mergeBB13:                                        ; preds = %trueBB12, %nextBB4
  %14 = phi i32 [ %5, %nextBB4 ], [ 1, %trueBB12 ]
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %trueBB15, label %nextBB3

trueBB15:                                         ; preds = %mergeBB13
  br label %mergeBB16

mergeBB16:                                        ; preds = %trueBB15, %nextBB3
  %16 = phi i32 [ %3, %nextBB3 ], [ 1, %trueBB15 ]
  %17 = icmp ne i32 %16, 0
  br i1 %17, label %trueBB22, label %nextBB

nextBB17:                                         ; preds = %nextBB
  %aa19 = load i32, ptr %aa, align 4
  %18 = icmp ne i32 %aa19, 0
  %19 = zext i1 %18 to i32
  br label %mergeBB21

falseBB20:                                        ; preds = %nextBB
  br label %mergeBB21

mergeBB21:                                        ; preds = %falseBB20, %nextBB17
  %20 = phi i32 [ %19, %nextBB17 ], [ 0, %falseBB20 ]
  %21 = icmp ne i32 %20, 0
  %22 = zext i1 %21 to i32
  br label %mergeBB23

trueBB22:                                         ; preds = %mergeBB16
  br label %mergeBB23

mergeBB23:                                        ; preds = %trueBB22, %mergeBB21
  %23 = phi i32 [ %22, %mergeBB21 ], [ 1, %trueBB22 ]
  store i32 %23, ptr %aa, align 4
  %aa24 = load i32, ptr %aa, align 4
  %aa25 = load i32, ptr %aa, align 4
  %24 = shl i32 %aa25, 3
  store i32 %24, ptr %aa, align 4
  %aa26 = load i32, ptr %aa, align 4
  %aa27 = load i32, ptr %aa, align 4
  %b28 = load i32, ptr %b, align 4
  %25 = add nsw i32 %aa27, %b28
  %26 = call i32 (ptr, ...) @printf(ptr @0, i32 %25)
  ret i32 0
}
