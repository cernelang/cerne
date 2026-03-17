# Cerne's Roadmap
Currently, Cerne is sitting at an alpha phase, this means that by no means should you consider Cerne a complete programming language/compiler yet. But there are set plans which are mapped in **versioning**.

## Versions
So, generally speaking, Cerne will be divided by 2 types of versions: Alpha (pre v1.0.0) and Stable (post 1.0.0). Currently, Cerne sits in the Alpha stage. 

You will see a little section at the end of every alpha version (**v0.x.x**) called "_requirement_", this is so that the version doesn't advance to the next "major" version until a _requirement_ is met, this keeps thing way more organized.

### v0.0.x ✅ ("the start")
Initial repo, some CLI functionality implemented, and links to the lexer began establishing

**Requirement:** Initial lexer
### v0.1.x ✅ ("the lexer update")
This version is dedicated to CLI functionality advanced and improved a lot, diagnostics improvement, lexer improvement, links to the parser and a minimal but decent working version of the parser working.

**Requirement:** Initial parser, improvement on the other components.

### v0.2.x <- [Currently here] ("the parser update")
This version is dedicated to parser improvement, SEMA implementation, project structure improvement, docs building and even the initial version of the [website](https://cerne.space) as well!

**Requirement:** Initial SEMA, improvement on other components, mainly parser.

### v0.3.x ("the sema update")
This version is dedicated to the development of the SEMA component, as well as the beginning of IR construction and structure.

**Requirement:** Initial IR, huge focus on SEMA completion.

### v0.4.x ("the IR update")
This version is strictly dedicated to building the IR, this does NOT include optimizations (that's the next version), it's simply to "translate" SEMA's "sanitized" AST into a much better IR prepared for codegen. Also one or two optimizations might be introduced, but the main focus of this version is just to get the IR going.

**Requirement:** IR completion, maybe 1 or 2 optimizations

### v0.5.x ("the optimizations")
This version is to mainly act on the IR and create as many optimizations as possible. Cerne is not aimed to be just another hobbyist project, but it's also not meant to be a revolutionizing, full fledged "production" compiler either, so I can only so implement around 13-15 main optimization passes, but that should be enough for majority of my goals. Either way, this is something I'll see later anyways.

**Requirement:** Optimizations implementation (exact number not needed for requirement to be passed)

### v0.6.x ("codegen")
Now we finally get to work with code generation, this version is STRICTLY dedicated to generating machine code, as to what it's geared towards, for this version, only linux x86_64 is going to be supported. But that's something _somewhat_ temporary.

**Requirement:** Codegen implementation (must successfuly, without bugs, generating a very good object file)

### v0.7.x ("linker")
Cerne also obviously needs a linker, but there's no need to use `ld` or any of these other linkers, so this version is going to be fully dedicated to making a linker.

Now I know that making a linker is a whole project in itself but as long as the linker does it's main job and actually links the object files without any weird errors, corruption or anything else, then it should be fine for a "component" instead of a whole new repo in of itself.

**Requirement:** Linker implementation (must link multiple object files without **__ANY__** problems)

### v0.8.x ("tools")
For cerne to be considered a more serious programming language, it needs tools. One of these tools, for example, is the LSP (language server protocol), in which it will be used by IDE's to communicate with it and be able to create things like intellisense, real-time linkage, ...

Other tools include (but are not limited to): formatter, REPL, debugger (which is a huge tool on it's own but... oh well), ...

**Requirement:** LSP fully working, and those other tools I mentioned as well

### v0.9.x ("everything possible before stable release")
Before extreme testing, we just need to implement some last things that were not yet mentioned but are a huge part in cerne, mainly speaking, the package manager, this is a huge thing that does deserve a whole version in itself, but I thought it would be better to include it inside of one instead. And another thing is obviously support for other big things, such as window and macOS, and if possible (although not required for now IMO) 32-bit support (main point with the compatibility goal is to be able to be compatible with the 3 main OSes).

Anyways this version is dedicated to clean up everything (dead code, unused headers, extremely code complex functions, super slow algorithms/methods, pre c++20-methods, ...)

And after all of that, the extreme test bombing begins, before this version, there's obviously going to be very big test suites, but we need to ensure **__100%__** of **any and every** case is fully supported and working as intended.

Everything that the other version took so long to make, the hard work, everything has to pay off in this version, and if something doesn't? It's better to be here than to be fixed during stable updates when it should be a huge change (saves a lot of embarassing patch notes)

**Requirement:** Package manager working, as well as literally everything else in every case possible to test for