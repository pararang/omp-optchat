# Changelog

## Unreleased

- Add macOS storage using a compiled Node-API helper for descriptor-relative filesystem operations and a process-lifetime directory lock, retaining Linux's existing backend and the log/tree format.
- Preserve durability, private permissions, and inode-anchored writes across renamed directories and symlink aliases; refuse macOS torn-line repair if the validated file was replaced.
- Keep provider cache markers and Codex reasoning retention active when OMP 18.8 prepends its date/cwd reminder.
- Run CI on Linux and macOS, with portable replay-fsync observation and cross-process renamed-root lock coverage.
- Document macOS build prerequisites and the Bun 1.4+ requirement for installing the checked-in lockfile.
- Recover stale OptChat turns at idle input boundaries without weakening active-context refusals. See [safe context recovery](CONTEXT_RECOVERY.md).
- Integrate v0.3.0 default activation, revised memory views, and cache coordination with macOS storage; keep atomic view snapshot replacement and cleanup anchored to the held directory descriptor.

## 0.3.0 — 2026-10-11

- Automatically activate OptChat in main-agent sessions when the enabled plugin is discovered, for fresh installations and upgrades alike; `--optchat` is no longer required.
- Add `--no-optchat` as a session opt-out that takes precedence regardless of flag order, leaving ordinary context intact and creating no OptChat store or compressor work. Subagents remain inactive.
- Exercise isolated plugin linking, automatic discovery, default activation, and explicit opt-out against the real OMP host. Preserve existing memory-directory ownership and host compaction behavior; no global settings are changed.

## 0.2.0 — 2026-10-11

- Correct merge priority to measure age from each sibling pair's last message.
- Batch the main view from 128 KB to 64 KB and preserve its exact tiling and unfinished batches in atomic, fsynced `view.json` snapshots, including legacy-log migration and crash recovery.
- Give compression a separate 16–32 KB built-only context and a 512-dash ruler.
- Share the effective host system prompt and compatible captured tool schemas between turns and compactions, without executing compressor tools or bypassing destination-model capabilities.
- Use four-line Anthropic cache blocks and coordinate concurrent compressor prefix writes, with cancellation, retry, and unsupported-transport boundaries. Main requests may wait for compressor writers but are not tracked as writers because host response hooks lack request identity.
- Extend real-host smoke coverage for shared prefixes, saved-view stability, and installed-artifact verification. Preserve the existing leaf scheduler, retry policy, log vocabulary, host safety, and thinking exclusion.

## 0.1.0 — 2026-10-05

- Add opt-in OMP OptChat sessions with a durable, append-only message log and binary contextual summary tree.
- Start fresh inference contexts from an incremental summary view while preserving active tool loops, steering, asides, and background reports.
- Provide lossless `zoom`, message dates, JSONL/text imports, and a private navigable HTML export.
- Add contextual background compression, UTF-8 budgets, fixed retries, cancellation, Linux single-writer locking, recovery fsync, and descriptor-anchored filesystem access.
- Keep native thinking and the host's private thinking tool out of memory.
- Add capability-gated provider cache markers, fail-closed host deadline handling, regression coverage, and real-OMP synthetic-provider smoke verification.

See README for the Linux requirement, the 25-second host settle limit, supported cache transports, and the stateless OpenAI launch setting.
