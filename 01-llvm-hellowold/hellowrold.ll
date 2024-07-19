; ModuleID = 'helloworld'
source_filename = "helloworld"

@gStr = private constant [12 x i8] c"hello,world\00"

declare i32 @puts(ptr)

define i32 @main() {
entry:
  %call_puts = call i32 @puts(ptr @gStr)
  ret i32 0
}
