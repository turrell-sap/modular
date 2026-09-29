# UAF-SPEC-001

Source, build tooling and conformance suite for the **Unified Accelerator
Fabric (UAF)** engineering specification.

| File | Description |
|---|---|
| `build_uaf_v214_pdf.sh` | Builds **v2.1.4** (current). Writes the Markdown, runs two build gates, compiles the PDF |
| `UAF-SPEC-001-v2.1.4.md` / `.pdf` | v2.1.4, 73 pages, 192 numbered requirements |
| `build_uaf_v213_pdf.sh` | Builds **v2.1.3**, the reviewed baseline for v2.1.4 |
| `UAF-SPEC-001-v2.1.3.md` / `.pdf` | v2.1.3, 71 pages, 191 numbered requirements |
| `build_uaf_v212_pdf.sh` | Builds **v2.1.2** |
| `UAF-SPEC-001-v2.1.2.md` / `.pdf` | v2.1.2, 67 pages, 187 numbered requirements |
| `build_uaf_v211_pdf.sh` | Builds **v2.1.1** |
| `UAF-SPEC-001-v2.1.1.md` / `.pdf` | v2.1.1, 61 pages, 180 numbered requirements |
| `build_uaf_v21_pdf.sh` | Builds **v2.1** |
| `UAF-SPEC-001-v2.1.md` / `.pdf` | v2.1, 51 pages, 162 numbered requirements |
| `build_uaf_v2_pdf.sh` | Builds **v2.0**, the original reviewed baseline |
| `UAF-SPEC-001-v2.md` / `.pdf` | v2.0, 23 pages |
| `conformance/` | Executable conformance suite for v2.1.4 |

Each revision is kept so the review trail stays intact: v2.0 was reviewed twice,
then v2.1, v2.1.1, v2.1.2 and v2.1.3 once each. Every finding's disposition is
recorded in the current document's Appendices E through I, newest first.

The `.md` files are generated: each build script rewrites its Markdown from an
embedded heredoc on every run, so edits belong in the script.

## Build the specification

```bash
./build_uaf_v214_pdf.sh
```

### Build gates

The build fails rather than producing a document that contradicts itself.

**Requirement index.** Appendix C is generated from the text and gates the
build. A paragraph beginning with a bracketed identifier is that requirement's
definition; every other occurrence is a citation. The build fails if an
identifier is cited without being defined, is defined twice, or is defined
outside a numbered section. v2.1.2 shipped a conformance row citing
`[R-5.5-012]`, which nobody had written — the gate exists because that survived
a whole revision unnoticed.

**Version consistency.** The document version, the `uaf_config.h` listing and
the CMake listing must agree. Review caught a three-way mismatch in v2.1.1, and
each of the three is prose in a different place, so nothing else would notice.

Both gates have been checked by deliberately breaking them.

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

Twelve cases, `C-1` through `C-12`, mapped to requirements in Section 12.3 of
the specification. They build with `-Wall -Wextra -Wpedantic -Werror` and run under
`ctest`.

| Case | Covers |
|---|---|
| `C-abi` | Struct sizes and offsets, CQE alignment and phase position, MTU accounting |
| `C-crc32c` | RFC 3720 vectors and the golden header of Section 5.8 |
| `C-wire` | Encode/decode, big-endian order, bad magic, bad CRC, framing, atomics |
| `C-qp_state` | Legal and illegal transitions, attribute masks, the three per-path RTR columns |
| `C-rkey` | Key generation, binding at connect, bounds, authenticated-failure throttle, per-source throttle, stale keys |
| `C-ring` | Power-of-two depth, full ring, wrap inside bounds, counter wrap |
| `C-cqe_phase` | Phase-tag polling; a zeroed ring yields zero completions; the caller's full 64-bit `wr_id` survives the poll |
| `C-dst_map` | NVMe opcode map including flush, NLB 0 and 1, PRP/SGL, `cmd_id`, each NVMe status code pinned by value |
| `C-conn_wire` | Connection record round trip including the `path` byte; a host `memcpy` is not the wire form; emits the Section 5.8 vector |
| `C-udp_loopback` | End-to-end multi-segment RDMA WRITE over UDP on loopback |
| `C-cm` | CM body length per opcode; MAC input for both body sizes; simultaneous open elects exactly one active side |
| `C-rmt_cq` | The UAF-D intra-host RMT completion ring, its initial phase of 1 and its phase discipline |

The values the specification publishes as test vectors are produced by
`C-crc32c`, not written by hand.

## Status

v2.1.4 is **Draft for Approval**. The review of v2.1.3 recommended signing
Section 1.6; v2.1.4 closes the six items it would still have edited (Appendix E).
The signature block stays empty because only the eight named reviewers can fill
it, and the status line changes when they do — not before. Of the eight open issues, four are closed on the substance and
four are accepted as deliberate limits of the 2.1.x line; only OI-7, validating
the performance targets, needs hardware. Appendix D records each. Section 1.6 carries the sign-off
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
bookmarks. All six documents build with zero overfull boxes, zero missing characters and
zero LaTeX errors. No strikethrough is used, so `soul.sty` is not required.
