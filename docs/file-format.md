# Project File Format

The first persistent format will be versioned JSON. The schema is intentionally not frozen in this milestone.

Planned top-level fields:

- `version`
- `tempo`
- `scale`
- `instruments`
- `tracks`

Each track will serialize its bank/pattern slots and each pattern will serialize its length, timing options, and full step parameter state, including explicit unset values.
