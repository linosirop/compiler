# MiniCompiler IR

function main: void ()
  entry:
    x_0 = ALLOCA int    # declare int x
    STORE [x_0], 0    # x = initializer
    t1 = LOAD [x_0]    # load x
    t2 = CMP_LT t1, 10    # t1 < 10
    JUMP_IF t2, L_then1
    JUMP L_else2
  L_then1:
    t3 = LOAD [x_0]    # load x
    t4 = ADD t3, 1    # t3 + 1
    STORE [x_0], t4    # x = value
    JUMP L_endif3
  L_else2:
    t5 = LOAD [x_0]    # load x
    t6 = SUB t5, 1    # t5 - 1
    STORE [x_0], t6    # x = value
    JUMP L_endif3
  L_endif3:
    RETURN
  # Variable mapping:
  # x -> [x_0]
