# v0.2.1
some big changes happened once again, but this time, project-wide
## Build System
this one was arguably the biggest one, I decided to move cerne's build system entirely from makefile to cmake and honestly I should've done this earlier, way better for quality-of-life and also, obviously, because of releases (they can finally be cross platform)
## Test Script
the test script has also been fixed and is now working perfectly as intended
## AST Nodes
ast nodes which previously had std::string_view properties now have std::string to fully owned strings because there were some issues involving outlived src code buffers which resulted in gibberish being stored (first error catched by the test script!)
another nice thing was a very simple example of export being added, and although it's not fully implemented yet (since, as you can see, at the time of THIS commit there aren't any export tests), it's at least not returning "nullptr"
## Workflow
a new github action was created to automatically create a release, I'm not sure if this will actually work as intended, but if not, I'll fix it in the next update

Either way, this might've not been a super focused C++ update on a lot of components but I still feel like this update was important for the whole project anyways

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