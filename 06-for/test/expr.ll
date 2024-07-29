; ModuleID = 'expr'
source_filename = "expr"

@0 = private unnamed_addr constant [14 x i8] c"expr val: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %aa = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 4, ptr %b, align 4
  %b1 = load i32, ptr %b, align 4
  store i32 1, ptr %aa, align 4
  %aa2 = load i32, ptr %aa, align 4
  store i32 -2, ptr %b, align 4
  %b3 = load i32, ptr %b, align 4
  store i32 %b3, ptr %aa, align 4
  %aa4 = load i32, ptr %aa, align 4
  br label %cond

cond:                                             ; preds = %entry
  %b5 = load i32, ptr %b, align 4
  %0 = icmp ne i32 %b5, 0
  %1 = sext i1 %0 to i32
  %2 = icmp ne i32 %1, 0
  br i1 %2, label %then, label %else

then:                                             ; preds = %cond
  br label %cond6

else:                                             ; preds = %cond
  store i32 19, ptr %b, align 4
  %b13 = load i32, ptr %b, align 4
  br label %last

last:                                             ; preds = %else, %last9
  br label %for.init

cond6:                                            ; preds = %then
  br i1 false, label %then7, label %else8

then7:                                            ; preds = %cond6
  store i32 9, ptr %b, align 4
  %b10 = load i32, ptr %b, align 4
  store i32 19, ptr %aa, align 4
  %aa11 = load i32, ptr %aa, align 4
  br label %last9

else8:                                            ; preds = %cond6
  store i32 30, ptr %aa, align 4
  %aa12 = load i32, ptr %aa, align 4
  br label %last9

last9:                                            ; preds = %else8, %then7
  br label %last

for.init:                                         ; preds = %last
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  %i14 = load i32, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %for.init
  %i15 = load i32, ptr %i, align 4
  %3 = icmp slt i32 %i15, 100
  %4 = sext i1 %3 to i32
  %5 = icmp ne i32 %4, 0
  br i1 %5, label %for.body, label %for.last

for.inc:                                          ; preds = %for.last27
  %i39 = load i32, ptr %i, align 4
  %6 = add nsw i32 %i39, 1
  store i32 %6, ptr %i, align 4
  %i40 = load i32, ptr %i, align 4
  br label %for.cond

for.body:                                         ; preds = %for.cond
  %aa16 = load i32, ptr %aa, align 4
  %i17 = load i32, ptr %i, align 4
  %7 = add nsw i32 %aa16, %i17
  store i32 %7, ptr %aa, align 4
  %aa18 = load i32, ptr %aa, align 4
  br label %cond19

for.last:                                         ; preds = %then20, %for.cond
  %aa41 = load i32, ptr %aa, align 4
  %b42 = load i32, ptr %b, align 4
  %8 = mul nsw i32 %b42, 9
  %9 = add nsw i32 %aa41, %8
  %10 = call i32 (ptr, ...) @printf(ptr @0, i32 %9)
  ret i32 0

cond19:                                           ; preds = %for.body
  %i22 = load i32, ptr %i, align 4
  %11 = icmp sge i32 %i22, 55
  %12 = sext i1 %11 to i32
  %13 = icmp ne i32 %12, 0
  br i1 %13, label %then20, label %last21

then20:                                           ; preds = %cond19
  br label %for.last

last21:                                           ; preds = %for.break.death, %cond19
  br label %for.init23

for.break.death:                                  ; No predecessors!
  br label %last21

for.init23:                                       ; preds = %last21
  %j = alloca i32, align 4
  store i32 1, ptr %j, align 4
  %j28 = load i32, ptr %j, align 4
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc25, %for.init23
  %j29 = load i32, ptr %j, align 4
  %14 = icmp slt i32 %j29, 10
  %15 = sext i1 %14 to i32
  %16 = icmp ne i32 %15, 0
  br i1 %16, label %for.body26, label %for.last27

for.inc25:                                        ; preds = %last32, %then31
  %j37 = load i32, ptr %j, align 4
  %17 = add nsw i32 %j37, 2
  store i32 %17, ptr %j, align 4
  %j38 = load i32, ptr %j, align 4
  br label %for.cond24

for.body26:                                       ; preds = %for.cond24
  br label %cond30

for.last27:                                       ; preds = %for.cond24
  br label %for.inc

cond30:                                           ; preds = %for.body26
  %i33 = load i32, ptr %i, align 4
  %18 = icmp eq i32 %i33, 10
  %19 = sext i1 %18 to i32
  %20 = icmp ne i32 %19, 0
  br i1 %20, label %then31, label %last32

then31:                                           ; preds = %cond30
  br label %for.inc25

last32:                                           ; preds = %for.continue.death, %cond30
  %b34 = load i32, ptr %b, align 4
  %j35 = load i32, ptr %j, align 4
  %21 = mul nsw i32 %b34, %j35
  store i32 %21, ptr %b, align 4
  %b36 = load i32, ptr %b, align 4
  br label %for.inc25

for.continue.death:                               ; No predecessors!
  br label %last32
}
