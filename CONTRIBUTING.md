# Contributing

Thanks for your interest in this project. It contains a Linux kernel module
(`kernel/`) and a userspace application (`app/`), built from the top-level
`Makefile`.

## Getting started

```bash
git clone <repository-url>
cd <repository>
make help
```

Requirements: `make`, `gcc`, `g++`, `cmake`, the Linux kernel headers of the
target kernel, and `clang-format`.

Test kernel code in a virtual machine or on a dedicated board, not on your
main system.

## Before sending a change

```bash
make build     # the project must build
make format    # format the sources
make sa        # static analysis, no new warning
```

## Code style

- Kernel code follows the Linux kernel coding style
  (`Documentation/process/coding-style.rst`).
- Run `clang-format` through `make format`; do not reformat unrelated code.
- Every source file starts with an SPDX license line.

## Commits

- One logical change per commit, and every commit must build.
- Use a short summary in the imperative mood, prefixed by the area:

  ```
  kernel: fix missing unlock on error path
  ```

- Explain *why* the change is needed in the message body.
- Sign your commits (`git commit -s`) to certify the
  [Developer Certificate of Origin](https://developercertificate.org/).

## Pull requests

1. Create a branch from `main`.
2. Keep the change small and focused.
3. Describe what it does and how you tested it.
4. Answer the review comments by updating your commits.

## License

By contributing, you agree that your work is released under the license of
the files you modify (GPL-2.0 for the kernel module).