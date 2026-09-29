# UAF-SPEC-001 v2.0

Source and build tooling for the **Unified Accelerator Fabric (UAF)**
engineering specification, `UAF-SPEC-001`, version 2.0 (approved for
implementation).

| File | Description |
|---|---|
| `build_uaf_v2_pdf.sh` | Self-contained builder: writes the Markdown source, then compiles the PDF |
| `UAF-SPEC-001-v2.md` | Generated Markdown source (overwritten by the script) |
| `UAF-SPEC-001-v2.pdf` | Generated PDF, 23 pages |

## Build

```bash
./build_uaf_v2_pdf.sh
```

The script is idempotent: it rewrites `UAF-SPEC-001-v2.md` from its embedded
heredoc on every run, so edits belong in the script, not in the generated
Markdown.

### Prerequisites

Primary path — pandoc with XeLaTeX:

```bash
sudo apt-get install -y pandoc texlive-xetex texlive-latex-recommended \
    lmodern fonts-dejavu
```

`fvextra`, `etoolbox`, and `lmodern` come from `texlive-latex-recommended`
and `lmodern`; the DejaVu families come from `fonts-dejavu`.

Fallback path — if `xelatex` is absent, the script emits standalone HTML and
prints it with headless Chromium/Chrome/Edge, needing only:

```bash
sudo apt-get install -y pandoc chromium-browser
```

## Layout notes

Three issues in the raw v2.0 Markdown are corrected in this source:

1. **No duplicate section numbers.** Headings carry no hand-written numbers,
   so `--number-sections` emits `1 Introduction` rather than
   `0.1 1. Introduction`.
2. **One table of contents.** No static TOC list is present in the source, so
   `--toc` produces a single hyperlinked TOC.
3. **Full Unicode box drawing.** `mainfont`/`monofont` select the DejaVu
   families, so `─`, `│`, and `└──` in the code and state-machine diagrams
   render with no missing-glyph warnings.

The preamble additions in the YAML header also keep the output free of
overfull lines: `fvextra` wraps long code lines, and long `snake_case`
identifiers become breakable inside tables.
