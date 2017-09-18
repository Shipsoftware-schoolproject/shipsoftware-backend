# Contributing

Fork, then clone the repo:

```
git@github.com:your-username/shipsoftware-backend.git
```

Make your change, push to your fork and [submit a pull request](https://github.com/Shipsoftware-schoolproject/shipsoftware-backend/compare).

## Coding style

Preferred coding style for the project follows the [Linux kernel coding style](https://www.kernel.org/doc/Documentation/process/coding-style.rst) with
an exception e.g. with one line `if`'s which in this project are usually
closed with braces.

## Documenting

Documentation is done with Doxygen and only `.h` files should be documented.

Doxygen commands are started with `@` (at-sign) and comment blocks uses
JavaDoc style:
```
/**
 * ... text ...
 */
```