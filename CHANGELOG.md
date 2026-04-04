# v0.2.0
HUGE CHANGES
## Parser & Lexer
Each update that passes, the lexer gets closer and closer to feeling "complete". However, this major update was not because of the lexer, and instead because of the parser. The parser is now OFFICIALLY in it's first, working version. It can parse return statements, the fun, import keyword, AND variable declarations (and obviously, expressions)!

This is HUGE since now I can just focus on the fixing any parser related errors, add more features and can finally start moving towards SEMA (which will have it's own first working version for v0.3.0).

# v0.1.4
biggest update in v0.1.x
## Project Structure
I have moved the licenses for stdlib and docs to their respective folder instead of remaining in the root directory and also modified them so github could easily parse since adding "headers" to licenses makes github unable to detect what type of license it is.

## Parser & Lexer
There were some problems related to how the lexer and parser were being handled.
The `fun` blueprint has also been initialized (not finished because of the function's node class) 

## Diagnostics
I have also updated diagnostics so there's a clear difference whether the error is a **code**/**compiler** error or if it's a `general`/`CLI`/`driver` error.

# v0.1.3
Slight evolution on the parser, fixes on the lexer, and some other things to improve performance & code quality overall

# v0.1.2 
Beginning the parser construction

# v0.1.1
Some more fixes regarding the lexer

# v0.1.0
Began lexer construction and some other small fixes

# v0.0.1
Began CLI construction and project structure