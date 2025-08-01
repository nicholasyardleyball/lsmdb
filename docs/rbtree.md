### Purpose
This Red-Black Tree serves as the in-memory mutable component of a Log-Structured Merge (LSM) tree database. It is responsible for handling writes and lookups efficiently before data is flushed to disk as immutable SSTables.

Red-Black Trees were chosen for their predictable performance characteristics and amenability to precise reasoning via structural invariants. They provide a good balance between simplicity of implementation and consistently logarithmic time operations for insert, lookup, and delete.

This implementation closely follows the algorithm presented in Cormen, Leiserson, Rivest and Stein's *Introduction to Algorithms* (*CLRS*) with reference also to the original 1972 paper by Rudolf Bayer.

### Implementation
The core algorithm follows the standard description provided in *CLRS*. A small scaffolding layer supports tree lifecycle management, basic test instrumentation, and provisional memory allocation.

One key departure from *CLRS* is the removal of the sentinel nil node. Instead, we use NULL pointers to represent absent children and the parent of the root.

### Invariants
Although removing the nil sentinel requires guarding against NULL in some locations, the overall impact on complexity is minimal. In many cases, structural invariants allow us safely to assume the existence of nodes. This is particularly valuable in the insertion and deletion fix-up procedures, where guaranteed relationships simplify control flow and produce a cleaner, more readable implementation.

*CLRS* provides a formal treatment of these invariants, which remain applicable to this version.

### Interface Summary
The following core operations are supported:
- `rb_create()`, `rb_clear(tree)`, `rb_destroy(tree)`
- `rb_insert(tree, key, value)`
- `rb_delete(tree, key)`
- `rb_find(tree, key)`

Routines for validation and printing of the tree structure are also provided for debugging.

At present, keys and values are limited to `uint16_t` and memory is allocated and freed as required using `malloc()` and `free()`. This is sufficient for prototyping and testing.

### Future work
This Red-Black Tree is currently implemented as a minimal working component to support early development and testing of an LSM tree system. Planned improvements will make it more flexible and performant. These include:
- Generalized key and value types passed as `void*`
- Caller-supplied comparison functions to handle complex or structured key types
- Support for custom memory allocation strategies
- Addition of an external, stateful iterator interface.