# UAF-SPEC-001

Source, build tooling and conformance suite for the **Unified Accelerator
Fabric (UAF)** engineering specification.

| File | Description |
|---|---|
| `build_uaf_v211_pdf.sh` | Builds **v2.1.1** (current). Writes the Markdown, generates the requirement index, compiles the PDF |
| `UAF-SPEC-001-v2.1.1.md` / `.pdf` | v2.1.1, 61 pages, 180 numbered requirements |
| `build_uaf_v21_pdf.sh` | Builds **v2.1**, retained as the reviewed baseline for v2.1.1 |
| `UAF-SPEC-001-v2.1.md` / `.pdf` | v2.1, 51 pages, 162 numbered requirements |
| `build_uaf_v2_pdf.sh` | Builds **v2.0**, the original reviewed baseline |
| `UAF-SPEC-001-v2.md` / `.pdf` | v2.0, 23 pages |
| `conformance/` | Executable conformance suite for v2.1.1 |

Each revision is kept so the review trail stays intact: v2.0 was reviewed twice,
v2.1 once, and every finding's disposition is recorded in the current document's
Appendices E and F.

The `.md` files are generated: each build script rewrites its Markdown from an
embedded heredoc on every run, so edits belong in the script.

## Build the specification

```bash
./build_uaf_v211_pdf.sh
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

Eleven cases, `C-1` through `C-11`, mapped to requirements in Section 12.3 of
the specification. They build with `-Wall -Wextra -Wpedantic -Werror` and run under
`ctest`.

| Case | Covers |
|---|---|
| `C-abi` | Struct sizes and offsets, CQE alignment and phase position, MTU accounting |
| `C-crc32c` | RFC 3720 vectors and the golden header of Section 5.8 |
| `C-wire` | Encode/decode, big-endian order, bad magic, bad CRC, framing, atomics |
| `C-qp_state` | Legal and illegal transitions, attribute masks, profile address vectors |
| `C-rkey` | Key generation, QP binding, bounds, authenticated-failure throttle, stale keys |
| `C-ring` | Power-of-two depth, full ring, wrap inside bounds, counter wrap |
| `C-cqe_phase` | Phase-tag polling; a zeroed ring yields zero completions; the caller's full 64-bit `wr_id` survives the poll |
| `C-dst_map` | NVMe opcode map including flush, NLB 0 and 1, PRP/SGL, `cmd_id`, NVMe status mapping |
| `C-conn_wire` | Connection record round trip; a host `memcpy` is not the wire form |
| `C-udp_loopback` | End-to-end multi-segment RDMA WRITE over UDP on loopback |
| `C-cm` | CM body length per opcode; simultaneous open elects exactly one active side |

The values the specification publishes as test vectors are produced by
`C-crc32c`, not written by hand.

## Status

v2.1.1 is **Draft for Approval**, not approved for implementation. It closes the
review of v2.1 (Appendix E) on top of the two reviews of v2.0 (Appendix F).
Five of v2.1's eight open issues remain; they are listed with their disposition
in Appendix D. Section 1.6 carries the sign-off
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
bookmarks. All three documents build with zero overfull boxes, zero missing characters and
zero LaTeX errors. No strikethrough is used, so `soul.sty` is not required.
