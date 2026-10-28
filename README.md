# Fanx Controller Character Device Driver

A Linux character device driver that controls and monitors a system fan from
userspace through `/dev/fanx`, with a small userspace application to use it.

> **Status:** early development.

## Requirements

- `make`, `gcc`, `g++`
- Kernel headers matching your running kernel (`linux-headers-$(uname -r)`)

## Build

Run `make help` to list all targets.

```bash
make help
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

GPL-2.0.