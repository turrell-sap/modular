# QNLP specification set

Engineering specification for quantum natural language processing (pregroup /
DisCoCat / DisCoCirc) as classical simulation in C, C++ and Mojo on CUDA and
Qualcomm NPU/GPU, under the goal "NLP on quantum theory with almost zero memory".

Start with `qnlp_bridge_for_agents.md`. It is the entry point for people and
software agents: document registry with SHA-256 pins, precedence rules,
convention and equation indexes, test registry and task procedures.
`qnlp_manifest.json` is the same index in machine-readable form.

| File | Role |
|---|---|
| qnlp_bridge_for_agents.md | Entry point and precedence rules (v1.2) |
| qnlp_manifest.json | Machine-readable index (v1.2.0) |
| qnlp_first_principles_spec.md | Baseline specification, Sections 0-10 (frozen) |
| qnlp_spec_errata_v1_1.md | Errata E-1 to E-13, governing over the baseline |
| qnlp_spec_addendum_coverage_capacity.md | Sections 11-19: grammar, capacity, hybrid, discourse |
| review_of_companion_v1_1.md | Review of the external companion document v1.1 |
| qnlp_hardware_limits_distilled.md | What QNLP has not done, and its GPU/NPU maths |
| qnlp_pure_mathematics.md | Definitions, theorems and proofs |
| qnlp_dsdm_delivery_plan.md | DSDM delivery plan |
| verify.c | fp64 reference program for the baseline worked example |

The external companion document registered in the manifest is not included
here. The manifest's absolute paths refer to the authoring session; resolve
them by file name within this directory.
