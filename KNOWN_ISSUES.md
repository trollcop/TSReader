# TSReader — Known Issues

Findings from a code review of v2.8.46b in preparation for public release. Severity reflects exploitability or crash likelihood, not prevalence in normal use.

These are open issues to be addressed in a future hardening pass. Most stem from legacy Win32 C patterns (`lstrcpy`, `wsprintf`, unchecked `sscanf`) and from the fact that the codebase was originally designed for trusted local LAN use.

**Highest priority before public release:** bind ControlServer to loopback by default — the TCP/1400 control server currently grants full control (start recording, change file paths, select programs) to anyone on the LAN.

## Critical

| # | File | Line(s) | Issue |
|---|------|---------|-------|
| 1 | `TSReader_TCP/TSReader_TCP.c` | ~50 | TCP source forwards `recv` data without validating 0x47 sync alignment — a truncated or byte-shifted feed corrupts the demux. Verify sync byte before forwarding. |
| 2 | `ControlServer.c` | ~2700 | `wsprintf(szTemp, "...command: %s\n", szCommandBuffer)` overflows when peer sends 1024 non-null, non-CR bytes (prefix overruns by ~40 chars). Replace with `_snprintf`. |
| 3 | `parser.c` | ~500, ~1108 | `nSectionLength > 65536` check is wrong — section_length is a 12-bit field with max 4093. Combined with later pointer arithmetic, malformed PSI sections can read past buffer end. Tighten to `> 4093` and require `<= nPacketLength` before payload deref. |
| 4 | `ControlServer.c` | ~281, ~429, ~668, ~770, ~825 | `sscanf("%d"/"%x")` from socket input with no width specifier and no result-count check; results used as array indexes (e.g. `nSelectProgramNumber`). |
| 5 | `ControlServer.c` | ~598, ~612, ~625 | `lstrcpy(v->szRecordFile, szSpace+1)` from network-supplied command — no length check. Remote unauthenticated user on port 1400 can overflow the record filename buffer. |

## Major

| # | File | Line(s) | Issue |
|---|------|---------|-------|
| 6 | `TSReader_UDP/TSReader_UDP.c`, `TSReader_TCP.c` | ~482, ~521, ~227 | `sscanf` style parses CLI/registry strings into `MAX_PATH` buffers without `%255s` width specifier. |
| 7 | `TSReader_HTTP/TSReader_HTTP.c`, `TSReader_UDP.c` | ~258, ~257, ~319, ~457 | `lstrcpy(szURL, szCommandLine)` and similar without bounds checks; relies on caller-side discipline. |
| 8 | `ControlServer.c` | ~369, ~534, ~555, ~572 | `sprintf` (not `_snprintf`) into fixed `szResponse`/`szOK` buffers using `%s` with `v->szRecordFile`. With long filenames (Windows MAX_PATH=260) plus literal text, exceeds typical 256-byte response buffers. |
| 9 | `TSReader_UDP/TSReader_UDP.c` | ~128 | Dead `nOffset/nRemaining` accounting (commented out) in `recvfrom` path — confusing refactor footgun. Remove. |
| 10 | `parser.c` | ~553–556 | `nSectionLength -= 14` after only checking `> 0`; if `nSectionLength < 14` this underflows to a large positive int and `pSectionPointer[14]` reads past intended bounds. Add `if (nSectionLength < 14) break;`. |
| 11 | `ControlServer.c` | ~2618 | `accept()` with no source-IP filtering. **TCP/1400 grants full control** (record, change file paths, select programs) to any LAN host. **Bind to 127.0.0.1 by default** with an opt-in `--allow-remote-control` flag. |
| 12 | `TSReader_TCP/TSReader_TCP.c` | ~90 | `closesocket(sock)` in `TSReader_Stop` while `recv` is in flight; subsequent `Sleep(50)` poll on `fReadThreadTerminated` is not memory-barriered. |
| 13 | `sources.h` ring buffer (256×698 packets) | — | `nTSBuffersInUse` is guarded by CS but `tsb[].nSize` is written outside the CS in some paths; consumer reads without CS in some plugin examples. Document the contract or make all reads/writes Interlocked. |
| 14 | `ControlServer.c` | ~2581 | `lstrlen(szCommandBuffer)` recomputed each iteration of write-back loop; `nOutputIndex--` then check-after-the-fact leaves `nOutputIndex == -1` written before reset. Reorder check-before-decrement. |
| 15 | `TSReader_TCP.c`, `TSReader_UDP.c` | ~111 | `WSAStartup` return value never checked; matching `WSACleanup` not present on plugin unload. |

## Minor

| # | File | Line(s) | Issue |
|---|------|---------|-------|
| 16 | `ControlServer.c` | ~746, ~864, ~896 | `wsprintf` into stack buffers using PSI-derived strings (channel names from broadcast — untrusted). Cap with `_snprintf`. |
| 17 | All plugins + core | — | `lstrcpy` (an alias for ANSI `strcpy`) used pervasively. Project-wide replace with `StringCchCopyA` from `strsafe.h`. |
| 18 | `parser.c` | — | Magic number `4096` for section_length max; should be `4093` (12-bit value minus 3 already consumed). Define `MAX_SECTION_LEN`. |
| 19 | repo root | — | Many prebuilt `.exe`/`.pdb`/`.ilk`/`.map` artifacts (TSReaderPro2.7.45*.exe, TSReaderLite*.exe). Should not ship in source. Strip and add to `.gitignore`. |
| 20 | `TSReader_UDP.c` | — | Dead `#ifdef DEBUG_FILE` and commented buffering blocks — clean up before public release. |
| 21 | All `.c` files | — | VS2026 will flag deprecated `lstrcpy`/`wsprintf` — code currently relies on `_CRT_SECURE_NO_WARNINGS` or similar. Better to migrate. |
| 22 | `TSReader_TCP/TSReader_TCP.c` | ~129–138 | Declares stack `HOSTENT phe; pHostent = &phe;` then immediately overwrites with `gethostbyname()` return. Local `phe` is dead. |
| 23 | UDP plugin | — | No `IP_MULTICAST_LOOP`/`SO_REUSEPORT` plumbing on UDP join — minor robustness issue for multi-instance use. |

## Recommended remediation order

1. **#11** — Bind ControlServer to loopback by default (one-line `bind()` change). Highest security ROI.
2. **#3, #10** — Tighten `parser.c` section_length validation. PSI parsing is the most exposed input path after network sockets.
3. **#1** — Add 0x47 sync-byte realignment in TCP plugin.
4. **#2, #5, #8, #16** — Mechanical pass: replace `sprintf`/`wsprintf`/`lstrcpy` with `StringCch*` family in `ControlServer.c` and the network source plugins.
5. **#19** — Strip prebuilt binaries from repo, add `.gitignore`.
6. Remaining minor items as time permits.

---

*Generated as part of pre-release code review. Contributions welcome.*
