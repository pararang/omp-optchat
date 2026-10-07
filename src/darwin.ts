import { createRequire } from "node:module";

interface DarwinStorage {
  openAt(directoryFd: number, name: string, flags: number, mode: number): number;
  mkdirAt(directoryFd: number, name: string): void;
  readDirectory(directoryFd: number): string[];
  /** Diagnostics only; filesystem I/O never goes through the returned path. */
  descriptorPath(fd: number): string;
  lockDirectory(fd: number): void;
}

let storage: DarwinStorage | undefined;

/** Load only when OptChat opens storage; disabled sessions need no native addon. */
export function darwinStorage(): DarwinStorage {
  if (storage) return storage;
  const require = createRequire(import.meta.url);
  try {
    storage = require("../native/build/darwin.node") as DarwinStorage;
  } catch (error) {
    throw new Error("macOS storage helper is unavailable. Run bun install and bun run build:native in the omp-optchat checkout before enabling OptChat.", { cause: error });
  }
  return storage;
}
