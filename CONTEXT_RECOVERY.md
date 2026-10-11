# Safe recovery from a stale OptChat turn

## Symptom and evidence

OptChat refuses a request if the host context does not contain the first message of the active turn:

> OptChat refused the request: this turn's messages are missing from the host context.

The original session logs confirm the refusal, but they do not identify its cause.
The regression reproduces one cause: a missed terminal `agent_end` notification leaves a stale turn active.
A terminal notification with unrelated messages also leaves that turn active, because OptChat ignores late notifications from other turns.

## Recovery boundary

`src/index.ts` now handles the host `input` event for active OptChat sessions.
If `ctx.isIdle()` returns true, OptChat saves pending messages and clears the cached turn.
The next context request waits for memory summaries, creates a fresh view, and then records the new prompt.
The new prompt stays outside that view and reaches the model unchanged.

The host emits interactive and RPC input events before prompt setup marks the session busy.
`before_agent_start` is not a safe idle check: the host already counts a prompt as active at that point.

## Safety limits

- Input during an active run does not clear the turn.
- Steering, asides, retries, and queued continuations keep the same view and raw messages.
- A context request without the active turn marker still aborts the run and sends no request.
- Recovery does not replace a missing marker with a timestamp guess or another message.
- Recovery does not delete memory, change the log format, or release the writer lock.
- Programmatic prompts and background reports without an `input` event still depend on the host lifecycle notifications.

The host compaction hook still returns `{ cancel: true }`.
The message “Auto server compaction cancelled” is separate from this refusal.
OptChat cancels host compaction to preserve the raw tool loop.

## Apply the fix

1. Stop the OMP session that uses the affected memory directory.
2. Restart OMP with this extension and the same `--optchat-dir` value.
3. Check `/optchat status` for memory and compressor errors.
4. Resend the refused prompt after the host becomes idle.

Do not delete the memory directory or start a second writer for that directory.

## Verification

The regression cases in `tests/integration.test.ts` cover:

- An idle interactive prompt after a missed terminal notification and removal of the old marker.
- An idle RPC prompt after a missed terminal notification with the old marker still present.
- Exact durable message order, with no duplicate records or lost completed messages.
- Busy input and steering with the original view and raw loop intact.
- Refusal of an active request without its turn marker, followed by continuation with the original view.

Verification commands:

```sh
bun run typecheck
bun test tests
bun run smoke
```

The full suite passed: 412 tests, zero failures.
The smoke run used the real installed OMP CLI with a synthetic provider.
It verified fresh views, raw tool loops, durable memory, writer exclusion, and restoration after restart.
The stale notification regression uses the fake host event adapter, not the original affected session.
