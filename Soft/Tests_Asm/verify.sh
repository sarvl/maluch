#!/bin/bash

test_file() {
    local base_name=$1
    local asm_file="asm/${base_name}.asm"
    local gen_bin="asm/${base_name}.bin"
    local ref_bin="out/${base_name}.bin"

    echo -n "Testing ${base_name}... "

    # 
    if [[ "$base_name" == "g01t04" ]]; then
        fasmg "$asm_file" "$gen_bin" > /dev/null 2>&1
        local status=$?
        
        if [ $status -eq 2 ]; then
            echo "PASS (Expected failure)"
            rm -f "$gen_bin"
            return 0
        else
            echo "FAIL (Expected exit status 2, got $status)"
            rm -f "$gen_bin"
            return 1
        fi
    fi

    if ! fasmg "$asm_file" "$gen_bin" > /dev/null 2>&1; then
        echo "FAIL (Assembly failed)"
        return 1
    fi

    if cmp -s "$gen_bin" "$ref_bin"; then
        echo "PASS"
        rm -f "$gen_bin"
        return 0
    else
        echo "FAIL (Binaries differ)"
        rm -f "$gen_bin"
        return 1
    fi
}

FAILURES=0

for i in {00..35}; do
    test_file "g00t${i}" || ((FAILURES++))
done

for i in {00..04}; do
    test_file "g01t${i}" || ((FAILURES++))
done

if [ "$FAILURES" -gt 0 ]; then
    echo "Total failures: $FAILURES"
    exit 1
else
    echo "All tests passed successfully."
    exit 0
fi
