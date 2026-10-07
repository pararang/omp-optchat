#!/usr/bin/env bun
import { mkdirSync, renameSync, rmSync } from "node:fs";
import { createRequire } from "node:module";
import { dirname, join } from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

// Linux retains the procfs/socket backend and needs no native build.
if (process.platform === "darwin") {
  const root = dirname(dirname(fileURLToPath(import.meta.url)));
  const require = createRequire(import.meta.url);
  const { include_dir } = require("node-api-headers");
  const output = join(root, "native", "build", "darwin.node");
  const temporary = `${output}.${process.pid}.tmp`;
  mkdirSync(dirname(output), { recursive: true });
  try {
    const result = spawnSync(process.env.CC ?? "cc", [
      "-std=c11", "-Wall", "-Wextra", "-Werror", "-O2",
      "-bundle", "-undefined", "dynamic_lookup", "-DNAPI_VERSION=8",
      "-I", include_dir, join(root, "native", "darwin.c"), "-o", temporary,
    ], { stdio: "inherit" });
    if (result.error || result.status !== 0) {
      throw new Error("Cannot build macOS storage helper. Install Xcode Command Line Tools (xcode-select --install), then run bun run build:native.", { cause: result.error });
    }
    // Never publish a partially compiled addon, even with concurrent builds.
    renameSync(temporary, output);
    console.log("Built macOS descriptor-anchored storage helper");
  } finally {
    rmSync(temporary, { force: true });
  }
}
