#!/bin/bash
BINARY="./build/alphabet"
PASS=0
FAIL=0
ERRORS=""

run_test() {
    local lang="$1"
    local name="$2"
    local code="$3"
    local expected="$4"
    
    tmpfile=$(mktemp /tmp/alphabet_test_XXXXXX.abc)
    printf '%s' "$code" > "$tmpfile"
    
    output=$("$BINARY" "$tmpfile" 2>&1)
    exit_code=$?
    rm -f "$tmpfile"
    
    if [ $exit_code -ne 0 ]; then
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\nFAIL [$lang] $name:\n  $(echo "$output" | head -3)\n"
        return
    fi
    
    if [ -n "$expected" ]; then
        trimmed=$(echo "$output" | grep -v "^$" | grep -v "Alphabet Language" | grep -v "Developed by" | grep -v "Compiled with" | grep -v "Type 'help'" | head -20)
        if echo "$trimmed" | grep -qF "$expected"; then
            PASS=$((PASS + 1))
        else
            FAIL=$((FAIL + 1))
            ERRORS="$ERRORS\nFAIL [$lang] $name: WRONG OUTPUT\n  Expected: $expected\n  Got: $(echo "$trimmed" | head -3)\n"
        fi
    else
        if [ $exit_code -eq 0 ]; then
            PASS=$((PASS + 1))
        else
            FAIL=$((FAIL + 1))
            ERRORS="$ERRORS\nFAIL [$lang] $name: exit code $exit_code\n"
        fi
    fi
}

echo "========================================="
echo " ALPHABET EXHAUSTIVE TEST SUITE"
echo " 5 languages x all patterns"
echo "========================================="
echo ""

###############################################################################
# PART 1: UNIVERSAL TESTS (single-letter bytecode, all 5 languages)
###############################################################################
echo "=== PART 1: Universal tests (bytecode keywords, all languages) ==="

for LANG in en am es fr de; do

run_test "$LANG" "var_number" "#alphabet<$LANG>
5 x = 10
5 y = 20
z.o(x + y)" "30"

run_test "$LANG" "var_string" "#alphabet<$LANG>
12 s = \"hello\"
z.o(s)" "hello"

run_test "$LANG" "var_negative" "#alphabet<$LANG>
5 x = -5
z.o(x)" "-5"

run_test "$LANG" "string_concat" "#alphabet<$LANG>
12 a = \"hello\"
12 b = \" world\"
z.o(a + b)" "hello world"

run_test "$LANG" "comparison_gt" "#alphabet<$LANG>
5 a = 10
5 b = 5
z.o(a > b)" "1"

run_test "$LANG" "comparison_lt" "#alphabet<$LANG>
5 a = 3
5 b = 7
z.o(a < b)" "1"

run_test "$LANG" "comparison_eq" "#alphabet<$LANG>
5 a = 5
5 b = 5
z.o(a == b)" "1"

run_test "$LANG" "comparison_neq" "#alphabet<$LANG>
5 a = 5
5 b = 3
z.o(a != b)" "1"

run_test "$LANG" "comparison_gte" "#alphabet<$LANG>
5 a = 5
5 b = 5
z.o(a >= b)" "1"

run_test "$LANG" "comparison_lte" "#alphabet<$LANG>
5 a = 3
5 b = 7
z.o(a <= b)" "1"

run_test "$LANG" "logical_and" "#alphabet<$LANG>
5 a = 1
5 b = 1
z.o(a && b)" "1"

run_test "$LANG" "logical_or" "#alphabet<$LANG>
5 a = 1
5 b = 0
z.o(a || b)" "1"

run_test "$LANG" "logical_not" "#alphabet<$LANG>
5 a = 0
z.o(!a)" "1"

run_test "$LANG" "unary_minus" "#alphabet<$LANG>
5 a = 10
z.o(-a)" "-10"

run_test "$LANG" "z_sqrt" "#alphabet<$LANG>
z.o(z.sqrt(16))" "4"

run_test "$LANG" "z_abs" "#alphabet<$LANG>
z.o(z.abs(-42))" "42"

run_test "$LANG" "z_pow" "#alphabet<$LANG>
z.o(z.pow(2, 10))" "1024"

run_test "$LANG" "z_floor" "#alphabet<$LANG>
z.o(z.floor(3.7))" "3"

run_test "$LANG" "z_ceil" "#alphabet<$LANG>
z.o(z.ceil(3.2))" "4"

run_test "$LANG" "z_len_str" "#alphabet<$LANG>
12 s = \"hello\"
z.o(z.len(s))" "5"

run_test "$LANG" "z_type_num" "#alphabet<$LANG>
z.o(z.type(42))" "number"

run_test "$LANG" "z_type_str" "#alphabet<$LANG>
z.o(z.type(\"hi\"))" "string"

run_test "$LANG" "z_upper" "#alphabet<$LANG>
z.o(z.upper(\"hello\"))" "HELLO"

run_test "$LANG" "z_lower" "#alphabet<$LANG>
z.o(z.lower(\"HELLO\"))" "hello"

run_test "$LANG" "z_split" "#alphabet<$LANG>
5 p = z.split(\"a,b,c\", \",\")
z.o(z.len(p))" "3"

run_test "$LANG" "z_join" "#alphabet<$LANG>
5 p = z.split(\"a,b,c\", \",\")
z.o(z.join(p, \"-\"))" "a-b-c"

run_test "$LANG" "z_starts" "#alphabet<$LANG>
z.o(z.starts_with(\"hello\", \"hel\"))" "1"

run_test "$LANG" "z_ends" "#alphabet<$LANG>
z.o(z.ends_with(\"hello\", \"llo\"))" "1"

run_test "$LANG" "z_replace" "#alphabet<$LANG>
z.o(z.replace(\"hello world\", \"world\", \"abc\"))" "hello abc"

run_test "$LANG" "z_trim" "#alphabet<$LANG>
z.o(z.trim(\"  hi  \"))" "hi"

run_test "$LANG" "z_tostr" "#alphabet<$LANG>
z.o(z.tostr(42))" "42"

run_test "$LANG" "z_contains" "#alphabet<$LANG>
z.o(z.contains(\"hello\", \"ell\"))" "1"

run_test "$LANG" "z_min" "#alphabet<$LANG>
z.o(z.min(3, 7))" "3"

run_test "$LANG" "z_max" "#alphabet<$LANG>
z.o(z.max(3, 7))" "7"

run_test "$LANG" "z_tonum_int" "#alphabet<$LANG>
z.o(z.tonum(\"42\"))" "42"

run_test "$LANG" "z_tonum_float" "#alphabet<$LANG>
z.o(z.tonum(\"3.14\"))" "3.14"

run_test "$LANG" "z_chr" "#alphabet<$LANG>
z.o(z.chr(65))" "A"

run_test "$LANG" "z_ord" "#alphabet<$LANG>
z.o(z.ord(\"A\"))" "65"

run_test "$LANG" "if_basic" "#alphabet<$LANG>
5 x = 10
i (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "$LANG" "if_else" "#alphabet<$LANG>
5 x = 3
i (x > 5) {
  z.o(\"big\")
} e {
  z.o(\"small\")
}" "small"

run_test "$LANG" "if_elif" "#alphabet<$LANG>
5 x = 7
i (x > 10) {
  z.o(\"big\")
} e i (x > 5) {
  z.o(\"mid\")
} e {
  z.o(\"small\")
}" "mid"

run_test "$LANG" "nested_if" "#alphabet<$LANG>
5 a = 10
5 b = 20
i (a > 5) {
  i (b > 15) {
    z.o(\"both\")
  }
}" "both"

run_test "$LANG" "loop_counter" "#alphabet<$LANG>
5 cnt = 0
l (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "$LANG" "loop_sum" "#alphabet<$LANG>
5 sum = 0
5 cnt = 0
l (cnt < 5) {
  sum = sum + cnt
  cnt = cnt + 1
}
z.o(sum)" "10"

run_test "$LANG" "nested_loops" "#alphabet<$LANG>
5 total = 0
5 ci = 0
l (ci < 3) {
  5 cj = 0
  l (cj < 3) {
    total = total + 1
    cj = cj + 1
  }
  ci = ci + 1
}
z.o(total)" "9"

run_test "$LANG" "loop_break" "#alphabet<$LANG>
5 cnt = 0
l (cnt < 100) {
  cnt = cnt + 1
  i (cnt == 5) {
    b
  }
}
z.o(cnt)" "5"

run_test "$LANG" "loop_continue" "#alphabet<$LANG>
5 sum = 0
5 cnt = 0
l (cnt < 5) {
  cnt = cnt + 1
  i (cnt == 3) {
    k
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "$LANG" "list_literal" "#alphabet<$LANG>
5 a = [1, 2, 3, 4, 5]
z.o(z.len(a))" "5"

run_test "$LANG" "list_index" "#alphabet<$LANG>
5 a = [10, 20, 30]
z.o(a[1])" "20"

run_test "$LANG" "nested_list" "#alphabet<$LANG>
5 a = [[1, 2], [3, 4]]
z.o(a[0][1])" "2"

run_test "$LANG" "map_literal" "#alphabet<$LANG>
5 m = {\"a\": 1, \"b\": 2}
z.o(m[\"a\"])" "1"

run_test "$LANG" "map_mutation" "#alphabet<$LANG>
5 m = {\"x\": 1}
m[\"x\"] = 99
z.o(m[\"x\"])" "99"

run_test "$LANG" "empty_list" "#alphabet<$LANG>
5 a = []
z.o(z.len(a))" "0"

run_test "$LANG" "empty_map" "#alphabet<$LANG>
5 m = {}
z.o(z.len(m))" "0"

run_test "$LANG" "z_append" "#alphabet<$LANG>
5 a = [1, 2]
z.append(a, 3)
z.o(z.len(a))" "3"

run_test "$LANG" "z_sort" "#alphabet<$LANG>
5 a = [3, 1, 2]
z.sort(a)
z.o(a[0])" "1"

run_test "$LANG" "z_reverse" "#alphabet<$LANG>
5 a = [1, 2, 3]
z.reverse(a)
z.o(a[0])" "3"

run_test "$LANG" "z_keys" "#alphabet<$LANG>
5 m = {\"x\": 1, \"y\": 2}
5 k = z.keys(m)
z.o(z.len(k))" "2"

run_test "$LANG" "z_remove" "#alphabet<$LANG>
5 a = [1, 2, 3]
z.remove(a, 1)
z.o(z.len(a))" "2"

run_test "$LANG" "z_len_map" "#alphabet<$LANG>
5 m = {\"a\": 1, \"b\": 2, \"c\": 3}
z.o(z.len(m))" "3"

run_test "$LANG" "list_strings" "#alphabet<$LANG>
5 a = [\"x\", \"y\", \"z\"]
z.o(z.len(a))" "3"

run_test "$LANG" "deep_nest" "#alphabet<$LANG>
5 a = [[[1]]]
z.o(a[0][0][0])" "1"

run_test "$LANG" "string_cond" "#alphabet<$LANG>
12 s = \"hello\"
i (z.len(s) > 3) {
  z.o(\"long\")
}" "long"

run_test "$LANG" "try_catch" "#alphabet<$LANG>
t {
  5 x = 1 / 0
} h (5 err) {
  z.o(\"caught\")
}" "caught"

run_test "$LANG" "z_output" "#alphabet<$LANG>
z.o(\"test\")" "test"

run_test "$LANG" "z_type_str" "#alphabet<$LANG>
z.o(z.type(\"hello\"))" "string"

run_test "$LANG" "z_type_null" "#alphabet<$LANG>
z.o(z.type(null))" "null"

run_test "$LANG" "contains_false" "#alphabet<$LANG>
z.o(z.contains(\"hello\", \"xyz\"))" "0"

run_test "$LANG" "starts_false" "#alphabet<$LANG>
z.o(z.starts_with(\"hello\", \"xyz\"))" "0"

run_test "$LANG" "ends_false" "#alphabet<$LANG>
z.o(z.ends_with(\"hello\", \"xyz\"))" "0"

run_test "$LANG" "pow_zero" "#alphabet<$LANG>
z.o(z.pow(5, 0))" "1"

run_test "$LANG" "sqrt_zero" "#alphabet<$LANG>
z.o(z.sqrt(0))" "0"

run_test "$LANG" "abs_zero" "#alphabet<$LANG>
z.o(z.abs(0))" "0"

run_test "$LANG" "floor_neg" "#alphabet<$LANG>
z.o(z.floor(-3.2))" "-4"

run_test "$LANG" "ceil_neg" "#alphabet<$LANG>
z.o(z.ceil(-3.7))" "-3"

run_test "$LANG" "chained_comp" "#alphabet<$LANG>
5 x = 5
5 r = (x > 3) && (x < 10)
z.o(r)" "1"

run_test "$LANG" "nested_call" "#alphabet<$LANG>
z.o(z.abs(z.min(-5, -3)))" "3"

run_test "$LANG" "replace_all" "#alphabet<$LANG>
z.o(z.replace(\"aaa\", \"a\", \"b\"))" "bbb"

run_test "$LANG" "split_empty" "#alphabet<$LANG>
5 a = z.split(\"\", \",\")
z.o(z.len(a))" "1"

run_test "$LANG" "join_single" "#alphabet<$LANG>
5 a = [\"hello\"]
z.o(z.join(a, \",\"))" "hello"

run_test "$LANG" "list_mutation" "#alphabet<$LANG>
5 a = [1, 2, 3]
z.append(a, 4)
z.o(a[3])" "4"

run_test "$LANG" "remove_end" "#alphabet<$LANG>
5 a = [1, 2, 3]
z.remove(a, 2)
z.o(z.len(a))" "2"

run_test "$LANG" "z_insert" "#alphabet<$LANG>
5 a = [1, 3]
z.insert(a, 1, 2)
z.o(a[1])" "2"

done

###############################################################################
# PART 2: ENGLISH KEYWORD TESTS (#alphabet<en>)
###############################################################################
echo ""
echo "=== PART 2: English keyword tests ==="

run_test "en" "en_if" "#alphabet<en>
5 x = 10
if (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "en" "en_if_else" "#alphabet<en>
5 x = 3
if (x > 5) {
  z.o(\"big\")
} else {
  z.o(\"small\")
}" "small"

run_test "en" "en_loop" "#alphabet<en>
5 cnt = 0
loop (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "en" "en_while" "#alphabet<en>
5 cnt = 0
while (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "en" "en_break" "#alphabet<en>
5 cnt = 0
loop (cnt < 100) {
  cnt = cnt + 1
  if (cnt == 5) {
    break
  }
}
z.o(cnt)" "5"

run_test "en" "en_continue" "#alphabet<en>
5 sum = 0
5 cnt = 0
loop (cnt < 5) {
  cnt = cnt + 1
  if (cnt == 3) {
    continue
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "en" "en_return" "#alphabet<en>
m 5 double_it(5 x) {
  return x * 2
}
z.o(double_it(21))" "42"

run_test "en" "en_class" "#alphabet<en>
class Point {
  public 5 x
  public 5 y
  method init(5 ax, 5 ay) {
    x = ax
    y = ay
  }
  method get_x() {
    return x
  }
}
15 pt = new Point(3, 7)
z.o(pt.get_x())" "3"

run_test "en" "en_inheritance" "#alphabet<en>
class Animal {
  public 12 name
  method init(12 n) {
    name = n
  }
  method speak() {
    return name
  }
}
class Dog extends Animal {
  method speak() {
    return name + \": woof\"
  }
}
15 dog = new Dog(\"Rex\")
z.o(dog.speak())" "Rex: woof"

run_test "en" "en_interface" "#alphabet<en>
j Printable {
  m 5 print()
}
c MyVal ^ Printable {
  v 5 val
  v m init(5 v) {
    val = v
  }
  v m 5 print() {
    z.o(val)
    r val
  }
}
15 obj = n MyVal(42)
obj.print()" "42"

run_test "en" "en_static" "#alphabet<en>
class Math2 {
  static method square(5 x) {
    return x * x
  }
}
z.o(Math2.square(7))" "49"

run_test "en" "en_try_catch" "#alphabet<en>
try {
  5 x = 1 / 0
} handle (5 err) {
  z.o(\"caught\")
}" "caught"

run_test "en" "en_const" "#alphabet<en>
const x = 42
z.o(x)" "42"

run_test "en" "en_match" "#alphabet<en>
5 x = 2
match (x) {
  1: z.o(\"one\")
  2: z.o(\"two\")
  3: z.o(\"three\")
}" "two"

run_test "en" "en_import" "#alphabet<en>
5 x = 10
z.o(x)" "10"

run_test "en" "en_public_private" "#alphabet<en>
class Secret {
  private 5 hidden = 42
  public 5 visible = 100
}
15 s = new Secret()
z.o(s.visible)" "100"

run_test "en" "en_inheritance" "#alphabet<en>
c Shape {
  v m 5 area() {
    r 0
  }
}
c Circle ^ Shape {
  v 5 radius
  v m init(5 r) {
    radius = r
  }
  v m 5 area() {
    r radius * radius
  }
}
15 c = n Circle(5)
z.o(c.area())" "25"

run_test "en" "en_chained_methods" "#alphabet<en>
class Builder {
  public 12 buf
  method init() {
    buf = \"\"
  }
  method add(12 s) {
    buf = buf + s
    return buf
  }
  method build() {
    return buf
  }
}
15 b = new Builder()
b.add(\"hel\")
b.add(\"lo\")
z.o(b.build())" "hello"

run_test "en" "en_for_loop" "#alphabet<en>
5 items = [10, 20, 30]
5 sum = 0
5 idx = 0
loop (idx < z.len(items)) {
  sum = sum + items[idx]
  idx = idx + 1
}
z.o(sum)" "60"

run_test "en" "en_for_range_loop" "#alphabet<en>
5 sum = 0
5 i = 0
loop (i < 5) {
  sum = sum + i
  i = i + 1
}
z.o(sum)" "10"

###############################################################################
# PART 3: AMHARIC KEYWORD TESTS (#alphabet<am>)
###############################################################################
echo ""
echo "=== PART 3: Amharic keyword tests ==="

run_test "am" "am_if" "#alphabet<am>
5 x = 10
ከሆነ (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "am" "am_if_else" "#alphabet<am>
5 x = 3
ከሆነ (x > 5) {
  z.o(\"big\")
} አለበለዚህ {
  z.o(\"small\")
}" "small"

run_test "am" "am_loop" "#alphabet<am>
5 cnt = 0
ሉፕ (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "am" "am_while" "#alphabet<am>
5 cnt = 0
እስከሆነ (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "am" "am_break" "#alphabet<am>
5 cnt = 0
ሉፕ (cnt < 100) {
  cnt = cnt + 1
  ከሆነ (cnt == 5) {
    ስበር
  }
}
z.o(cnt)" "5"

run_test "am" "am_continue" "#alphabet<am>
5 sum = 0
5 cnt = 0
ሉፕ (cnt < 5) {
  cnt = cnt + 1
  ከሆነ (cnt == 3) {
    ቀጥል
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "am" "am_return" "#alphabet<am>
ዘዴ 5 ድርብ(5 x) {
  ተመለስ x * 2
}
z.o(ድርብ(21))" "42"

run_test "am" "am_class" "#alphabet<am>
ክፍል Point {
  ግልጽ 5 x
  ግልጽ 5 y
  ዘዴ መጀመሪያ(5 ax, 5 ay) {
    x = ax
    y = ay
  }
  ዘዴ አግኝ_x() {
    ተመለስ x
  }
}
15 pt = አዲስ Point(3, 7)
z.o(pt.አግኝ_x())" "3"

run_test "am" "am_try_catch" "#alphabet<am>
ሞክር {
  5 x = 1 / 0
} ያዟ(5 err) {
  z.o(\"caught\")
}" "caught"

run_test "am" "am_const" "#alphabet<am>
ቋሚ-እሴት x = 42
z.o(x)" "42"

###############################################################################
# PART 4: SPANISH KEYWORD TESTS (#alphabet<es>)
###############################################################################
echo ""
echo "=== PART 4: Spanish keyword tests ==="

run_test "es" "es_if" "#alphabet<es>
5 x = 10
si (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "es" "es_if_else" "#alphabet<es>
5 x = 3
si (x > 5) {
  z.o(\"big\")
} sino {
  z.o(\"small\")
}" "small"

run_test "es" "es_loop" "#alphabet<es>
5 cnt = 0
bucle (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "es" "es_while" "#alphabet<es>
5 cnt = 0
mientras (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "es" "es_break" "#alphabet<es>
5 cnt = 0
bucle (cnt < 100) {
  cnt = cnt + 1
  si (cnt == 5) {
    romper
  }
}
z.o(cnt)" "5"

run_test "es" "es_continue" "#alphabet<es>
5 sum = 0
5 cnt = 0
bucle (cnt < 5) {
  cnt = cnt + 1
  si (cnt == 3) {
    continuar
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "es" "es_return" "#alphabet<es>
metodo 5 doble(5 x) {
  retornar x * 2
}
z.o(doble(21))" "42"

run_test "es" "es_class" "#alphabet<es>
clase Point {
  publico 5 x
  publico 5 y
  metodo init(5 ax, 5 ay) {
    x = ax
    y = ay
  }
  metodo get_x() {
    retornar x
  }
}
15 pt = nuevo Point(3, 7)
z.o(pt.get_x())" "3"

run_test "es" "es_try_catch" "#alphabet<es>
intentar {
  5 x = 1 / 0
} capturar(5 err) {
  z.o(\"caught\")
}" "caught"

run_test "es" "es_const" "#alphabet<es>
constante x = 42
z.o(x)" "42"

###############################################################################
# PART 5: FRENCH KEYWORD TESTS (#alphabet<fr>)
###############################################################################
echo ""
echo "=== PART 5: French keyword tests ==="

run_test "fr" "fr_if" "#alphabet<fr>
5 x = 10
si (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "fr" "fr_if_else" "#alphabet<fr>
5 x = 3
si (x > 5) {
  z.o(\"big\")
} sinon {
  z.o(\"small\")
}" "small"

run_test "fr" "fr_loop" "#alphabet<fr>
5 cnt = 0
boucle (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "fr" "fr_while" "#alphabet<fr>
5 cnt = 0
tantque (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "fr" "fr_break" "#alphabet<fr>
5 cnt = 0
boucle (cnt < 100) {
  cnt = cnt + 1
  si (cnt == 5) {
    rompre
  }
}
z.o(cnt)" "5"

run_test "fr" "fr_continue" "#alphabet<fr>
5 sum = 0
5 cnt = 0
boucle (cnt < 5) {
  cnt = cnt + 1
  si (cnt == 3) {
    continuer
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "fr" "fr_return" "#alphabet<fr>
methode 5 doubler(5 x) {
  retour x * 2
}
z.o(doubler(21))" "42"

run_test "fr" "fr_class" "#alphabet<fr>
classe Point {
  public 5 x
  public 5 y
  methode init(5 ax, 5 ay) {
    x = ax
    y = ay
  }
  methode get_x() {
    retour x
  }
}
15 pt = nouveau Point(3, 7)
z.o(pt.get_x())" "3"

run_test "fr" "fr_try_catch" "#alphabet<fr>
essayer {
  5 x = 1 / 0
} attraper(5 err) {
  z.o(\"caught\")
}" "caught"

run_test "fr" "fr_const" "#alphabet<fr>
constante x = 42
z.o(x)" "42"

###############################################################################
# PART 6: GERMAN KEYWORD TESTS (#alphabet<de>)
###############################################################################
echo ""
echo "=== PART 6: German keyword tests ==="

run_test "de" "de_if" "#alphabet<de>
5 x = 10
wenn (x > 5) {
  z.o(\"yes\")
}" "yes"

run_test "de" "de_if_else" "#alphabet<de>
5 x = 3
wenn (x > 5) {
  z.o(\"big\")
} sonst {
  z.o(\"small\")
}" "small"

run_test "de" "de_loop" "#alphabet<de>
5 cnt = 0
schleife (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "de" "de_while" "#alphabet<de>
5 cnt = 0
solange (cnt < 3) {
  cnt = cnt + 1
}
z.o(cnt)" "3"

run_test "de" "de_break" "#alphabet<de>
5 cnt = 0
schleife (cnt < 100) {
  cnt = cnt + 1
  wenn (cnt == 5) {
    brechen
  }
}
z.o(cnt)" "5"

run_test "de" "de_continue" "#alphabet<de>
5 sum = 0
5 cnt = 0
schleife (cnt < 5) {
  cnt = cnt + 1
  wenn (cnt == 3) {
    fortsetzen
  }
  sum = sum + cnt
}
z.o(sum)" "12"

run_test "de" "de_return" "#alphabet<de>
methode 5 verdoppeln(5 x) {
  zuruck x * 2
}
z.o(verdoppeln(21))" "42"

run_test "de" "de_class" "#alphabet<de>
klasse Point {
  offentlich 5 x
  offentlich 5 y
  methode init(5 ax, 5 ay) {
    x = ax
    y = ay
  }
  methode get_x() {
    zuruck x
  }
}
15 pt = neu Point(3, 7)
z.o(pt.get_x())" "3"

run_test "de" "de_try_catch" "#alphabet<de>
versuchen {
  5 x = 1 / 0
} fangen(5 err) {
  z.o(\"caught\")
}" "caught"

run_test "de" "de_const" "#alphabet<de>
konstante x = 42
z.o(x)" "42"

###############################################################################
echo ""
echo "========================================="
echo " RESULTS: $PASS passed, $FAIL failed"
echo "========================================="

if [ $FAIL -gt 0 ]; then
    echo ""
    echo "FAILURES:"
    echo -e "$ERRORS"
fi
