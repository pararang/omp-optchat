# Changelog

## Unreleased

- Add macOS storage using a compiled Node-API helper for descriptor-relative filesystem operations and a process-lifetime directory lock, retaining Linux's existing backend and the log/tree format.
- Preserve durability, private permissions, and inode-anchored writes across renamed directories and symlink aliases; refuse macOS torn-line repair if the validated file was replaced.
- Keep provider cache markers and Codex reasoning retention active when OMP 18.8 prepends its date/cwd reminder.
- Run CI on Linux and macOS, with portable replay-fsync observation and cross-process renamed-root lock coverage.
- Document macOS build prerequisites and the Bun 1.4+ requirement for installing the checked-in lockfile.

## 0.1.0 — 2026-10-05

- Add opt-in OMP OptChat sessions with a durable, append-only message log and binary contextual summary tree.
- Start fresh inference contexts from an incremental summary view while preserving active tool loops, steering, asides, and background reports.
- Provide lossless `zoom`, message dates, JSONL/text imports, and a private navigable HTML export.
- Add contextual background compression, UTF-8 budgets, fixed retries, cancellation, Linux single-writer locking, recovery fsync, and descriptor-anchored filesystem access.
- Keep native thinking and the host's private thinking tool out of memory.
- Add capability-gated provider cache markers, fail-closed host deadline handling, regression coverage, and real-OMP synthetic-provider smoke verification.

See README for the Linux requirement, the 25-second host settle limit, supported cache transports, and the stateless OpenAI launch setting.
