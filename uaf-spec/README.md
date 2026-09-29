# UAF-SPEC-001

Source, build tooling and conformance suite for the **Unified Accelerator
Fabric (UAF)** engineering specification.

| File | Description |
|---|---|
| `build_uaf_v21_pdf.sh` | Builds **v2.1** (current). Writes the Markdown, generates the requirement index, compiles the PDF |
| `UAF-SPEC-001-v2.1.md` / `.pdf` | v2.1, 51 pages, 162 numbered requirements |
| `build_uaf_v2_pdf.sh` | Builds **v2.0**, retained as the reviewed baseline |
| `UAF-SPEC-001-v2.md` / `.pdf` | v2.0, 23 pages |
| `conformance/` | Executable conformance suite for v2.1 |

The `.md` files are generated: each build script rewrites its Markdown from an
embedded heredoc on every run, so edits belong in the script.

## Build the specification

```bash
./build_uaf_v21_pdf.sh
```

Appendix C, the requirement index, is generated from the requirement
identifiers actually present in the text, so it cannot drift from the body.

### Prerequisites

```bash
sudo apt-get install -y pandoc texlive-xetex texlive-latex-recommended \
    lmodern fonts-dejavu poppler-utils
```

If `xelatex` is absent the script emits standalone HTML and prints it with
headless Chromium, Chrome or Edge, needing only `pandoc` and a browser.

## Run the conformance suite

```bash
./conformance/run_conformance.sh
```

Ten cases, `C-1` through `C-10`, mapped to requirements in Section 12.3 of the
specification. They build with `-Wall -Wextra -Wpedantic -Werror` and run under
`ctest`.

| Case | Covers |
|---|---|
| `C-abi` | Struct sizes and offsets, CQE alignment and phase position, MTU accounting |
| `C-crc32c` | RFC 3720 vectors and the golden header of Section 5.8 |
| `C-wire` | Encode/decode, big-endian order, bad magic, bad CRC, framing, atomics |
| `C-qp_state` | Legal and illegal transitions, attribute masks, profile address vectors |
| `C-rkey` | Key generation, QP binding, bounds, brute-force limit, stale keys |
| `C-ring` | Power-of-two depth, full ring, wrap inside bounds, counter wrap |
| `C-cqe_phase` | Phase-tag polling; a zeroed ring yields zero completions |
| `C-dst_map` | NVMe opcode map including flush, NLB 0 and 1, PRP/SGL, `cmd_id` |
| `C-conn_wire` | Connection record round trip; a host `memcpy` is not the wire form |
| `C-udp_loopback` | End-to-end multi-segment RDMA WRITE over UDP on loopback |

The values the specification publishes as test vectors are produced by
`C-crc32c`, not written by hand.

## Status

v2.1 is **Draft for Approval**, not approved for implementation. It closes two
independent reviews of v2.0; the disposition of every finding is in Appendix E
and the items still open are in Appendix D. Section 1.6 carries the sign-off
block that must be completed before the status may change.

## PDF layout notes

The v2.0 Markdown had three layout defects, corrected in both scripts:

1. **No duplicate section numbers.** Headings carry no hand-written numbers,
   so `--number-sections` emits `1 Introduction` rather than
   `0.1 1. Introduction`.
2. **One table of contents.** No static TOC is present in the source, so
   `--toc` produces a single hyperlinked TOC.
3. **Full Unicode box drawing.** `mainfont` and `monofont` select the DejaVu
   families, so the diagram glyphs render with no missing-glyph warnings.

The preamble also keeps the output free of overfull lines: `fvextra` wraps long
code lines, long `snake_case` identifiers become breakable in both prose and
tables, and `pdfstringdefDisableCommands` keeps that breaking out of the PDF
bookmarks. Both documents build with zero overfull boxes, zero missing
characters and zero LaTeX errors.
