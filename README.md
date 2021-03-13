# cahpv4-sim

A functional simulator for CAHPv4

## Google spreadsheet to bit pattern

```shell
$ cat isasrc.txt|cut -f1,24-34 | awk -F'\t' '{ if (length($1) != 0) {split($1,inst," ");print "{\"xxxx_xxxx_xxxx_xxxx_xxxx_x" tolower($2) tolower($3) tolower($4) "_" tolower($5) tolower($6) tolower($7) tolower($8) "_" tolower($9) tolower($10) tolower($11) tolower($12) "\", inst_" tolower(inst[1]) "},\t// " inst[1] }}'
```
