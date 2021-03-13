#!/usr/bin/bash

failwith() {
    echo -e "\e[31m[ERROR]\e[m testentry \"$1\" \"$2\" \"$3\" \"$4\""

    if [ -n "$5" ]; then
        echo "===== RESULT FROM HERE ====="
        echo "$5"
        echo "===== RESULT TO HERE ====="
    fi

    exit 1
}

testentry() {
    res=$(./cahp-sim -t "$2" -c "$1")
    [ "$?" -eq 0 ] || failwith "$1" "$2" "$3"
    echo "$res" | egrep "$3" > /dev/null
    [ "$?" -eq 0 ] || failwith "$1" "$2" "$3" "$res"
}

### lb x2, -500(x1)
testentry 1 \
    ":reg: 0, 1F4 \
     :ram: FA \
     :rom: 89, 0F, 82, C1" \
    "x2=65530"

### lbu x2, -500(x1)
testentry 1 \
    ":reg: 0, 1F4 \
     :ram: FA \
     :rom: 89, 0A, 82, C1" \
    "x2=250"

### lw x2, -500(x1)
testentry 1 \
    ":reg: 0, 1F4 \
     :ram: 2A, 00 \
     :rom: 09, 0A, 82, C1" \
    "x2=42"

### sb x3, -500(x1)
### lbu x2, -500(x1)
testentry 2 \
    ":reg: 0, 1F4, 0, 112A \
     :rom: 82, 0A, 6C, C0, 89, 0A, 82, C1" \
    "x2=42"

### sw x3, -500(x1)
### lw x2, -500(x1)
testentry 2 \
    ":reg: 0, 1F4, 0, 112A  \
     :rom: 02, 0A, 6C, C0, 09, 0A, 82, C1" \
    "x2=4394"

##############################

### add x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 02, 03  \
     :rom: 08, 10, 61, 00" \
    "x1=5"

### add x16, x17, x18
testentry 1 \
    ":reg: 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 02, 03  \
     :rom: 08, 88, 50, 02" \
    "x16=5"

### sub x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 02, 03  \
     :rom: 18, 10, 61, 00" \
    "x1=65535"

### xor x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 04, 05  \
     :rom: 28, 10, 61, 00" \
    "x1=1"

### or x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 05, 03  \
     :rom: 38, 10, 61, 00" \
    "x1=7"

### and x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 05, 03  \
     :rom: 48, 10, 61, 00" \
    "x1=1"

### sll x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 05, 03  \
     :rom: 98, 10, 61, 00" \
    "x1=40"

### srl x1, x2, x3
testentry 1 \
    ":reg: 00, 00, 15, 03  \
     :rom: A8, 10, 61, 00" \
    "x1=2"

### sra x1, x2, x3
testentry 1 \
    ":reg: 00, 00, FFF0, 04  \
     :rom: C8, 10, 61, 00" \
    "x1=65535"

### slt x1, x2, x3
testentry 1 \
    ":reg: 00, 00, FFF0, 04  \
     :rom: 08, 15, 61, 00" \
    "x1=1"

### sltu x1, x2, x3
testentry 1 \
    ":reg: 00, 00, FFF0, 04  \
     :rom: 08, 11, 61, 00" \
    "x1=0"

##############################

# FIXME: mul, div, rem

##############################

### addi x1, x2, -5
testentry 1 \
    ":reg: 00, 00, 10 \
     :rom: 08, 12, 61, FF" \
    "x1=11"

### xori x1, x2, 5
testentry 1 \
    ":reg: 00, 00, 03 \
     :rom: 28, 12, A1, 00" \
    "x1=6"

### ori x1, x2, 5
testentry 1 \
    ":reg: 00, 00, 03 \
     :rom: 38, 12, A1, 00" \
    "x1=7"

### andi x1, x2, 5
testentry 1 \
    ":reg: 00, 00, 03 \
     :rom: 48, 12, A1, 00" \
    "x1=1"

### slli x1, x2, 9
testentry 1 \
    ":reg: 00, 00, 05 \
     :rom: 98, 12, 21, 01" \
    "x1=2560"

### srli x1, x2, 9
testentry 1 \
    ":reg: 00, 00, FA00 \
     :rom: A8, 12, 21, 01" \
    "x1=125"

### srai x1, x2, 9
testentry 1 \
    ":reg: 00, 00, FA00 \
     :rom: C8, 12, 21, 01" \
    "x1=65533"

### slti x1, x2, -2
testentry 1 \
    ":reg: 00, 00, FFFF \
     :rom: 08, 17, C1, FF" \
    "x1=0"

# FIXME: sltiu

### li x1, -500
testentry 1 \
    ":reg: 00, 01  \
     :rom: 08, FE, 81, C1" \
    "x1=65036"

##############################

### beq x1, x2, 16
testentry 1 \
    ":reg: 00, 01, FFF0  \
     :rom: 14, 0A, 50, 00" \
    "pc=4"

### beq x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 14, 0A, 50, 00" \
    "pc=16"

### bne x1, x2, 16
testentry 1 \
    ":reg: 00, 01, FFF0  \
     :rom: 04, 0A, 50, 00" \
    "pc=16"

### bne x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 04, 0A, 50, 00" \
    "pc=4"

### blt x1, x2, 16
testentry 1 \
    ":reg: 00, FFF0, 10 \
     :rom: 44, 0E, 50, 00" \
    "pc=16"

### blt x1, x2, 16
testentry 1 \
    ":reg: 00, 10, FFF0 \
     :rom: 44, 0E, 50, 00" \
    "pc=4"

### blt x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 44, 0E, 50, 00" \
    "pc=4"

### bge x1, x2, 16
testentry 1 \
    ":reg: 00, FFF0, 10 \
     :rom: 34, 0E, 50, 00" \
    "pc=4"

### bge x1, x2, 16
testentry 1 \
    ":reg: 00, 10, FFF0 \
     :rom: 34, 0E, 50, 00" \
    "pc=16"

### bge x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 34, 0E, 50, 00" \
    "pc=16"

### bltu x1, x2, 16
testentry 1 \
    ":reg: 00, FFF0, 10 \
     :rom: 44, 0A, 50, 00" \
    "pc=4"

### bltu x1, x2, 16
testentry 1 \
    ":reg: 00, 10, FFF0 \
     :rom: 44, 0A, 50, 00" \
    "pc=16"

### bltu x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 44, 0A, 50, 00" \
    "pc=4"

### bgeu x1, x2, 16
testentry 1 \
    ":reg: 00, FFF0, 10 \
     :rom: 34, 0A, 50, 00" \
    "pc=16"

### bgeu x1, x2, 16
testentry 1 \
    ":reg: 00, 10, FFF0 \
     :rom: 34, 0A, 50, 00" \
    "pc=4"

### bgeu x1, x2, 16
testentry 1 \
    ":reg: 00, 01, 01  \
     :rom: 34, 0A, 50, 00" \
    "pc=16"

### jal x1, 60000
testentry 1 \
    ":reg: 00 00 \
     :rom: FC, D6, 01, CC" \
    "x1=4.+pc=60000"

### jalr x1, x2, -500
testentry 1 \
    ":reg: 00, 00, EC54 \
     :rom: FC, 13, 81, C1" \
    "x1=4.+pc=60000"

echo "ok"
