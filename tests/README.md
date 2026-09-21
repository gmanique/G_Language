# Tests

```bash
cmake -B build -DGLANG_SANITIZE=ON && cmake --build build   # sanitizers catch what exit codes hide
tests/run_tests.sh ./build/glang                            # everything (about 45 s with sanitizers)
tests/run_tests.sh --help                                   # all options and conventions
```

Only problems are printed. A passing run ends with `failed=0`; `xfail` counts the checks
that are **not implemented yet** (see below), it is not a failure.

| you want to...                              | command                                                    |
| ------------------------------------------- | ---------------------------------------------------------- |
| run one area                                | `tests/run_tests.sh -k parser/err_enum ./build/glang`      |
| see every test, including the passing ones  | `tests/run_tests.sh -v ./build/glang`                      |
| list the semantic checks still to implement | `tests/run_tests.sh -v -k semantic/todo ./build/glang`     |
| skip the slow groups                        | `--no-fuzz --no-stress`                                    |
| fuzz harder                                 | `FUZZ_LEVEL=2 tests/run_tests.sh --fuzz-only ./build/glang`|
| accept a changed golden (then `git diff`!)  | `tests/run_tests.sh --update ./build/glang`                |

## Layout

```
tests/cases/lexer/      tokenisation: keywords, numbers, strings, comments, operators, exact error positions
tests/cases/parser/     grammar: precedence, associativity, declarations, statements, every syntax error
tests/cases/semantic/   valid programs that must NEVER be rejected + todo_* checks to implement
tests/cases/multi/      several files on one command line (each directory = one test)
```

Also generated at run time by `run_tests.sh` (nothing to store): **stress** (very deep nesting, 1 MB strings,
20 000 functions... must be fast and never crash), **cli** (missing files, paths with spaces, 300 files, binary
garbage...) and **fuzz** (valid programs cut at many offsets or with one byte mutated: the compiler
must answer exit 0 or 1, never crash or hang; reproducers are saved in `/tmp/glang_fuzz_failures`).

## What the name prefix means

| prefix   | expectation                                                                                     |
| -------- | ----------------------------------------------------------------------------------------------- |
| `ok_`    | the whole pipeline succeeds: exit 0, empty stderr                                               |
| `lex_`   | lexing succeeds (the file need not be a valid program)                                          |
| `parse_` | lexing and parsing succeed (analysis ignored: undeclared names are fine)                        |
| `err_`   | rejected: exit 1 **and** an explanation on stderr                                               |
| `todo_`  | should be rejected but is not implemented yet: `XFAIL` now, `XPASS` once it works, never a FAIL |

`lex_`/`parse_` exist so that grammar tests keep passing when the semantic analysis gets stricter.
When a `todo_` starts to `XPASS`, rename it to `err_` (and add an `X.stderr` pattern).

Every test starts with a `// ...` line saying what it checks and why. Companion files next to `X.gg`:

* `X.stderr`: one `grep -E` pattern per line, all must match; a line starting with `!` must **not** match.
  Error tests pin the exact `file:line:col: message`.
* `X.tokens` / `X.ast`: golden dumps. The AST golden drops the numeric token ids, so adding a token kind
  does not invalidate every file. To create one: `touch X.ast`, run with `--update`, read the result.

A directory named like a test (`multi/ok_two_independent_files/`) passes all the `*.gg` it contains, sorted.

## Checks applied to every test

No timeout, no crash, no exit code other than 0/1, no sanitizer report, no absurd line number in a
diagnostic (that is how an uninitialised token showed up), and an `err_` test must not fail silently.

## Behaviour pinned on purpose

Some tests document the **current** behaviour of a decision that is yours to make. They are marked
`CURRENT behaviour`, `DESIGN` or `DOCUMENTED AMBIGUITY` in their first line; change the test when you
change the language: `let`/`const` qualifiers, `a * b;` read as a declaration, empty statements `;`,
`--`/`++`/`+=` lexed as separate tokens, `break`/`continue`/`null`/`true` being plain identifiers.
