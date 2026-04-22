#!/bin/bash
# Comprehensive UTF-8 Language Tests for Alphabet Programming Language

echo "=========================================="
echo "  ALPHABET LANGUAGE - PRODUCTION TESTS"
echo "=========================================="
echo ""

PASS=0
FAIL=0

test_lang() {
    local name="$1"
    local code="$2"
    local expected="$3"
    
    # Try to find alphabet binary (handle .exe on Windows)
    local bin="./build/alphabet"
    if [ ! -f "$bin" ]; then
        if [ -f "./build/alphabet.exe" ]; then
            bin="./build/alphabet.exe"
        elif [ -f "./build/Release/alphabet.exe" ]; then
            bin="./build/Release/alphabet.exe"
        fi
    fi
    
    # Use '-' for stdin instead of /dev/stdin
    # Strip \r from output to handle Windows line endings
    result=$(echo "$code" | "$bin" - 2>&1 | tr -d '\r')
    
    if [ "$result" = "$expected" ]; then
        echo "✓ $name: PASS"
        PASS=$((PASS + 1))
    else
        echo "✗ $name: FAIL"
        echo "  Expected: $expected"
        echo "  Got: $result"
        FAIL=$((FAIL + 1))
    fi
}

cd "$(dirname "$0")"

echo "=== BASIC OUTPUT TESTS ==="
test_lang "English-Basic" '#alphabet<en>
5 x = 42
z.o(x)' "42"

test_lang "Amharic-Basic" '#alphabet<am>
5 ቁ = 123
ውጤት.o(ቁ)' "123"

test_lang "Spanish-Basic" '#alphabet<es>
5 num = 55
imprimir.o(num)' "55"

test_lang "French-Basic" '#alphabet<fr>
5 nb = 44
afficher.o(nb)' "44"

test_lang "German-Basic" '#alphabet<de>
5 zahl = 22
ausgeben.o(zahl)' "22"

echo ""
echo "=== CONTROL FLOW TESTS ==="
test_lang "English-If" '#alphabet<en>
5 x = 5
if (x > 3) { z.o("big") } e { z.o("small") }' "big"

test_lang "Amharic-If" '#alphabet<am>
5 x = 5
ከሆነ (x > 3) { ውጤት.o("big") } አለበለዚህ { ውጤት.o("small") }' "big"

test_lang "Spanish-If" '#alphabet<es>
5 x = 5
si (x > 3) { imprimir.o("big") } sino { imprimir.o("small") }' "big"

test_lang "French-If" '#alphabet<fr>
5 x = 5
si (x > 3) { afficher.o("big") } sinon { afficher.o("small") }' "big"

test_lang "German-If" '#alphabet<de>
5 x = 5
wenn (x > 3) { ausgeben.o("big") } sonst { ausgeben.o("small") }' "big"

echo ""
echo "=== PATTERN MATCHING TESTS ==="
test_lang "English-Match" '#alphabet<en>
5 x = 2
match (x) { 1: z.o("one") 2: z.o("two") e: z.o("other") }' "two"

test_lang "Amharic-Match" '#alphabet<am>
5 x = 2
ምረጥ (x) { 1: ውጤት.o("one") 2: ውጤት.o("two") e: ውጤት.o("other") }' "two"

test_lang "Spanish-Match" '#alphabet<es>
5 x = 2
coincidir (x) { 1: imprimir.o("one") 2: imprimir.o("two") e: imprimir.o("other") }' "two"

test_lang "French-Match" '#alphabet<fr>
5 x = 2
correspondre (x) { 1: afficher.o("one") 2: afficher.o("two") e: afficher.o("other") }' "two"

test_lang "German-Match" '#alphabet<de>
5 x = 2
ubereinstimmen (x) { 1: ausgeben.o("one") 2: ausgeben.o("two") e: ausgeben.o("other") }' "two"

echo ""
echo "=== LOOP TESTS ==="
test_lang "English-Loop" '#alphabet<en>
5 i = 0
l (i < 3) { z.o(i) 5 i = i + 1 }' "0
1
2"

test_lang "Amharic-Loop" '#alphabet<am>
5 i = 0
l (i < 3) { ውጤት.o(i) 5 i = i + 1 }' "0
1
2"

test_lang "Spanish-Loop" '#alphabet<es>
5 i = 0
bucle (i < 3) { imprimir.o(i) 5 i = i + 1 }' "0
1
2"

test_lang "French-Loop" '#alphabet<fr>
5 i = 0
boucle (i < 3) { afficher.o(i) 5 i = i + 1 }' "0
1
2"

test_lang "German-Loop" '#alphabet<de>
5 i = 0
schleife (i < 3) { ausgeben.o(i) 5 i = i + 1 }' "0
1
2"

echo ""
echo "=== TRY-CATCH TESTS ==="
test_lang "English-Try" '#alphabet<en>
t { z.o("try") } h (12 e) { z.o("catch") }
z.o("done")' "try
done"

test_lang "Amharic-Try" '#alphabet<am>
ሞክር { ውጤት.o("try") } ያዟ (12 e) { ውጤት.o("catch") }
ውጤት.o("done")' "try
done"

test_lang "Spanish-Try" '#alphabet<es>
intentar { imprimir.o("try") } capturar (12 e) { imprimir.o("catch") }
imprimir.o("done")' "try
done"

test_lang "French-Try" '#alphabet<fr>
essayer { afficher.o("try") } attraper (12 e) { afficher.o("catch") }
afficher.o("done")' "try
done"

test_lang "German-Try" '#alphabet<de>
versuchen { ausgeben.o("try") } fangen (12 e) { ausgeben.o("catch") }
ausgeben.o("done")' "try
done"

echo ""
echo "=== IMPORT TESTS ==="
export ALPHABET_PATH="$(cd "$(dirname "$0")" && pwd)"
test_lang "English-Import" '#alphabet<en>
import "test.abc"
z.o("ok")' "ok"

test_lang "Amharic-Import" '#alphabet<am>
አስገባ "test.abc"
ውጤት.o("ok")' "ok"

test_lang "Spanish-Import" '#alphabet<es>
importar "test.abc"
imprimir.o("ok")' "ok"

test_lang "French-Import" '#alphabet<fr>
importer "test.abc"
afficher.o("ok")' "ok"

test_lang "German-Import" '#alphabet<de>
importieren "test.abc"
ausgeben.o("ok")' "ok"

echo ""
echo "=========================================="
echo "  FINAL RESULTS"
echo "=========================================="
echo "Passed: $PASS"
echo "Failed: $FAIL"
echo "Total:  $((PASS + FAIL))"
echo "Success Rate: $((PASS * 100 / (PASS + FAIL)))%"
echo "=========================================="

if [ $FAIL -eq 0 ]; then
    echo "✓ ALL TESTS PASSED - PRODUCTION READY!"
    exit 0
else
    echo "✗ SOME TESTS FAILED"
    exit 1
fi
