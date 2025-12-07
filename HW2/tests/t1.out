└──Funcs
    ├──FuncDecl
    │   ├──ID: main
    │   ├──Type: void
    │   ├──Formals
    │   └──Statements
    │       ├──VarDecl
    │       │   ├──ID: x
    │       │   ├──Type: int
    │       │   └──BinOp: *
    │       │       ├──BinOp: +
    │       │       │   ├──Num: 4
    │       │       │   └──Num: 7
    │       │       └──Num: 12
    │       ├──VarDecl
    │       │   ├──ID: y
    │       │   ├──Type: byte
    │       │   └──NumB: 100
    │       ├──Call
    │       │   ├──ID: printOk
    │       │   └──ExpList
    │       │       └──And
    │       │           ├──RelOp: >
    │       │           │   ├──ID: x
    │       │           │   └──ID: y
    │       │           └──RelOp: <
    │       │               ├──ID: x
    │       │               └──Num: 100
    │       └──VarDecl
    │           ├──ID: z
    │           └──Type: int
    └──FuncDecl
        ├──ID: printOk
        ├──Type: void
        ├──Formals
        │   └──Formal
        │       ├──ID: isOk
        │       └──Type: bool
        └──Statements
            ├──VarDecl
            │   ├──ID: i
            │   ├──Type: int
            │   └──Num: 0
            ├──While
            │   ├──RelOp: <
            │   │   ├──ID: i
            │   │   └──Num: 10
            │   └──Statements
            │       ├──Call
            │       │   ├──ID: print
            │       │   └──ExpList
            │       │       └──String: i = 
            │       ├──Call
            │       │   ├──ID: printi
            │       │   └──ExpList
            │       │       └──ID: i
            │       └──Assign
            │           ├──ID: i
            │           └──BinOp: +
            │               ├──ID: i
            │               └──Num: 1
            └──If
                ├──ID: isOk
                ├──Statements
                │   ├──VarDecl
                │   │   ├──ID: x
                │   │   ├──Type: int
                │   │   └──Num: 0
                │   └──Call
                │       ├──ID: print
                │       └──ExpList
                │           └──String: ok
                └──Call
                    ├──ID: print
                    └──ExpList
                        └──String: not ok
