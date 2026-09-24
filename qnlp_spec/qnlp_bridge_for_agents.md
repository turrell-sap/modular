# QNLP document set: bridging specification for software agents

Version 1.2, 2026-09-24 (1.2: registry update -- puremath and dsdm registered in
Sections 1 and 12 with SHA-256, precedence note R14, Section 2.2 support note,
UNVERIFIED groups P and Q, gap G31, reading-order steps 5a and 7a; 1.1: navigation
fixes -- composite kernel route 8.1a,
tensordot store, relative-clause line map, E[D] at q_n = 2, export offsets, RDM /
entropy procedure 8.9, LIM-id resolution, conflict rows 41-42). Plain ASCII. No Python anywhere in this programme.
Scope: the seven-document specification (plus, from 1.2, the registered
pure-mathematics companion puremath and the delivery plan dsdm) of "NLP on
quantum theory with almost
zero memory" (pregroup / DisCoCat -> tensor contractions or parameterised
circuits, DisCoCirc for text), implemented in C / C++ / Mojo as classical
simulation on CUDA and on Qualcomm Hexagon/Adreno through the engineer's
adapter. This document lets a software agent that has read nothing else treat
the seven documents as ONE specification.

Citation style used here: <key> Sec. <n> (<equation>) line <m>, with keys from
the registry of Section 1 (spec, errata, addendum, companion, review, limits,
verify_c, puremath, dsdm). Line numbers refer to the files whose SHA-256 is
given in Section 1.
The abbreviations sNNN (spec line), aNNN (addendum line), cNNN (companion line)
are those of errata line 9 and review line 8-9. Anything not established by a
CONFIRMED erratum, by normative spec/addendum text, or by a listed fp64 C
program is marked UNVERIFIED.


## 0. How to use this document (one screen)

Reading order for a new agent:

  1. This document, Sections 0-3 (registry, precedence, conventions).
  2. spec Sec. 0 (conventions C1-C13, lines 11-142) -- read once, verbatim,
     with errata E-8 open (it corrects C12 at line 113).
  3. errata, whole file (217 lines). Keep it open beside the spec for ever.
  4. spec Secs. 1-2 (model and kernel), then Sec. 8 (worked example) while
     running verify.c (gcc verify.c -lm; diff its printout against spec Sec. 8).
  5. spec Secs. 4-6 (numerics, learning, export) and Sec. 9 (tests T1-T10).
  5a. puremath (qnlp_pure_mathematics.md, registered 1.2) only when a task needs
     the proof behind a convention, bound or algorithm: find the result through
     its Appendix A.2 (engineering label -> result); its Appendix B lists what
     it does not prove (R14).
  6. addendum C14-C17 (lines 15-70), then Secs. 11, 12, 16, 18 (tests T11-T24);
     Secs. 13-15 when the task touches the hybrid generator, DisCoCirc/MPS or
     evaluation.
  7. review Secs. 1, 2, 7 (verdicts and the minimal programme P0); the
     companion only through the review, and only Secs. 1-9, 13, 16-19.
  7a. dsdm (qnlp_dsdm_delivery_plan.md, registered 1.2) for WHAT to build next:
     the PRL row (id, MoSCoW, acceptance test and expected value), its timebox
     and the gate it feeds (dsdm Secs. 4, 5.4, 5.5). It never overrides a
     technical statement (R14).
  8. limits only when a task needs one of its themes (density matrices,
     spectral readouts, Gram/contrastive, partial traces, DisCoCirc higher-order
     boxes, exact second-order training, qudits, streamed lexicon).
  9. Sections 4-12 of this document as lookup tables while working.
     Procedures (Section 8) chain: a device kernel in a non-fp32 format =
     8.1 + 8.3 + 8.7 (worked route 8.1a); a new readout on DisCoCirc wires =
     8.9; an NPU export = 8.4 (steps 1 and 1b).

Precedence in one sentence: the frozen baseline spec governs every convention
and kernel, read with the errata overlaid at the lines the errata cite; the
addendum extends it and is the document of record for Sections 11-19; the
review governs how the companion is read and the companion's requirements are
constraints only; the hardware-limits report is advisory; verify.c is the
numerical ground truth for spec Sec. 8; anything UNVERIFIED beats anything
merely asserted.
Registered in 1.2 (R14): puremath proves results but never sets an
implementation value; dsdm sets schedule, priority and acceptance sequencing
only.

Hard rules an agent must never break (each has a source):
  - Bit order little-endian, first (leftmost) factor = lowest bits (spec C1, C7).
  - Angles stored as uint16 half-angle turns, theta 4 pi-periodic; never wrap a
    controlled-rotation angle mod 2 pi (spec C5 line 39-44, Sec. 5.6 line 1081).
  - Never accumulate probabilities in fp16 (spec C12 line 113 as corrected by
    errata E-8; spec Sec. 4.5 line 850).
  - Never renormalise a state mid-circuit; carry log sums; divide once at
    readout with the Z_min / eps_D guards (spec Sec. 4.5 line 859-862, (5.1)).
  - The two-term shift rule is only for N_c and D with G^2 = I; controlled
    rotations need the four-term rule (5.4) or the adjoint (spec Sec. 5.2).
  - The adjoint runs on the UNFUSED gate list (review line 320-321).
  - Init uniform [0, 2 pi) with a fixed seed, never N(0, 0.1^2) (errata E-10).
  - Q1.15 tensordot path: pre-scale words by S_q15 = 1 - 2^-15 before the single
    narrowing (errata E-6).
  - Production parser is the chart recogniser (11.4)-(11.5); greedy shift-reduce
    is a six-shape pre-check only (errata E-12, addendum Sec. 11.1).
  - E[Z] = 2^{-2 q_n} for Haar-random words, not 1 (errata E-1).
  - Baseline stays byte-identical; a spec change is an appended erratum with the
    hash pin (Section 8.8 below).


## 1. Document registry

All hashes recomputed on disk 2026-09-24 with sha256sum; sizes from wc.
Directory D = /tmp/claude-0/-home-user-modular/58690423-9136-5dc0-b3e6-6a28285a6c82/scratchpad/
Uploads U = /root/.claude/uploads/58690423-9136-5dc0-b3e6-6a28285a6c82/

| key | file | role | status | SHA-256 | lines / bytes | sections |
|---|---|---|---|---|---|---|
| spec | D/qnlp_first_principles_spec.md | "QNLP from first principles: unified engineering specification"; baseline | frozen (hash-pinned by companion c42, errata l1, review l5) | 7408d5b7fba156b29bd5e5b88f03f4fbb1f480ed95e48d834c41691f68efe86c | 1314 / 105999 | Sec. 0 (C1-C13), Secs. 1-10, equations (0.n)-(5.n), tests T1-T10 |
| errata | D/qnlp_spec_errata_v1_1.md | "Errata and clarifications for qnlp_first_principles_spec.md -- v1.1"; 12 corrections E-1..E-4, E-6..E-13 + 1 clarification E-5; Sec. B no-change list | governing over the baseline (and over addendum lines it names) | a5ebbf57971ab65ed534e3a8c41272330e8196bdd903a59934cc71fe04904359 | 217 / 20907 | Header l1-9, A. Corrections l11-204, B. No change l206-217 |
| addendum | D/qnlp_spec_addendum_coverage_capacity.md | "QNLP specification - Addendum: grammar coverage, capacity, hybrid architecture, discourse, positioning"; Secs. 11-19, C14-C17, T11-T24 | extension; technical document of record for its topics | 6b5e4bd95e0a17c4c8471d75acdced9d50bbafb4c03846fb959cf8000386afa5 | 1928 / 179689 | C14-C17 l15-70, Secs. 11-19 l73-1928 |
| companion | U/f1469dfb-qnlp_review_synthesis_and_application_refinement.md | "QNLP review synthesis and application refinement", v1.1, 2026-09-24; requirements, scope widening (typed records, processes, temporal layer), claim-disposition register RV01-RV60 | uploaded-external; corrections superseded by errata; requirements = constraints; adoption conditional (review l31-37) | 210bd23a643293e2fdb46fa5f7f386d72b03dc20b9992960db4872d483830028 | 1082 / 104202 | Secs. 1-19 |
| review | D/review_of_companion_v1_1.md | "Review of 'QNLP review synthesis and application refinement' v1.1"; verdict per correction, companion defects, consistency with addendum, architecture assessment, minimal falsifiable programme P0 | review; governing over the companion's wording; basis of the errata | 3d2fa31226264381d73c8ddcc6806cb35405ea92d50f157f3a851866e0225f54 | 603 / 61422 | Secs. 0-9 |
| limits | D/qnlp_hardware_limits_distilled.md | what QNLP has not done, why, and its GPU/NPU maths: nine themes A1-A8 + B, Tables A/B, tier-C table, primitive cost table | advisory report; cites spec and addendum, never re-derives | 5298903da120bd73036cfd60926ae8f5ded7665ff4104d246a8d64f9737410ec | 1683 / 178827 | Secs. 1-5 |
| verify_c | D/verify.c | fp64 C reference reproducing spec Sec. 8 (word states, sigma, Z, p, losses, bent-wire, Bell cups, 8 FD gradients, shift rules, CRz decomposition, periodicity, i00/compact at W = 6) | frozen reference program (hash-pinned by companion c45, review l6; errata l8: unaffected by every item) | ecdb49e3c7baf7b91f91cc2697bbf7e504063ddd1b5df0bd302e207d963fa052 | 103 / 8528 | n/a (no sections) |
| puremath | D/qnlp_pure_mathematics.md | "Mathematics of compositional quantum-theoretic language models", v1.1: definitions, lemmas, propositions, theorems and proofs (283 labelled results in Chapters 1-7: pregroups and chart recognition, rigid categories and spiders, Hilbert-space semantics and the law of Z, tensor networks and MPS/MPO, differentiation, numerical analysis, capacity); Appendix A result <-> engineering-label correspondence; Appendix B conjectures / UNVERIFIED / open problems (37 items); Appendix C standard results | registered 1.2 (2026-09-24); proofs companion; ranks with the review for derivations, below the errata for any spec line (R14) | 0254d2b100df04a147fd4595b54d0e412690ae1b0a7a75b7ad260601087a1996 | 3776 / 324707 | Sec. 0 (conventions, notation), Chapters 1-7, Appendices A-C |
| dsdm | D/qnlp_dsdm_delivery_plan.md | "QNLP delivery plan (DSDM Agile Project Framework)", v1.0: DSDM principles and PAQ, roles and vacancies, lifecycle products, Prioritised Requirements List (138 schedulable rows, 5 NFR Musts, 9 Won'ts), increments I0-I7 in 18 two-week timeboxes (TB0-TB17) plus final deployment, gates G0-G7 with the P0 falsification gate G4, SAD / DAD summaries, risk log, traceability matrix, estimation, management approach, benefits | registered 1.2 (2026-09-24); delivery plan; no technical precedence (R14) | 3e2b0ba07867e9e527bf7372092366962a01f83fcfeb2e28b1f994ef2a7cf9ce | 1789 / 199966 | Secs. 1-12, Appendix A |

Authoritative for / NOT authoritative for:

- spec: authoritative for every kernel convention (C1-C13), the six-shape
  grammar, the fused tensordot, bent-wire circuit form, readout, simulation
  kernel, hardware mapping, number formats, learning rules, export format v1,
  memory budget, worked example, tests T1-T10. NOT authoritative at any line
  named by the errata (see Section 2), nor for grammar beyond six shapes,
  capacity, hybrid, DisCoCirc/MPS (addendum), nor for lambeq/DisCoPy
  conventions (all UNVERIFIED, spec Sec. 10 item 1).
- errata: authoritative for the replacement text at every cited spec line and
  the named addendum lines; for tests T-EZ, the E-6 endpoint case, E-7 stall
  assertions, E-10 saddle/E[D] tests, E-12 greedy-vs-chart regression, E-5
  offset test. NOT authoritative for anything it does not cite; its HVX cycle
  figures are instruction-count estimates (errata l111, UNVERIFIED).
- addendum: authoritative for grammar coverage (chart parser, lexicon, spiders,
  arbitrary cup structures, tree adjoint), capacity and scaling, hybrid angle
  generator and its export block, DisCoCirc/MPS, positioning and evaluation
  plan, extended memory table, C14-C17, T11-T24. NOT authoritative where the
  errata names its lines (a107, a130-131, a486/488, a708/a788, a721, a726,
  a1560/1566/1657), for its literature figures (Sec. 19 item 1, all UNVERIFIED),
  or for the p_min/precision numbers of Sec. 12.3 derived with superseded
  constants (Section 9 below).
- companion: authoritative only as a source of requirements/constraints (CP-*
  rules, resource categories, tractability contract, output status fields,
  export metadata list, ablations, gates) and of the deferred-scope documents
  (Secs. 10-12.8, 14). NOT authoritative for any technical correction (errata
  supersede), any number (review re-derived them), or its own adoption status
  (review l31-37).
- review: authoritative for the verdict on each companion correction (Sec. 2),
  companion defects (Sec. 3), addendum consistency actions (Sec. 4),
  architecture assessment (Sec. 5), companion edits (Sec. 6), programme P0
  (Sec. 7), sources check (Sec. 8), and the derivations/programs behind the
  errata. NOT authoritative over the errata's final wording (errata carry the
  second-pass fixes: E-1 sd, E-5 status, E-9 registers, E-10 Rz ratio, E-11
  numbers, E-12 pattern); its statement that the limits report is absent
  (l6-7, l384) is stale; its floor and amplification statements at l81-83
  ("Sec. 6.2 floors D >= 2^-6 / 2^-4 sit at the untrained median for (2,1)",
  "A ~ 2^{q_n+2}") predate E-11 and say "median" where the mean is meant
  (superseded: single floor 2^-6, factor 2^{q_n}; Section 9).
- limits: authoritative for the kernels, tests and working-set figures of its
  own themes (A1-A8, B), which no other document derives. NOT authoritative
  where it overlaps the addendum (density matrices 13.2(c), DisCoCirc 14.1, MPS
  14.3, readout 14.6), for any convention (it defines none; its C1..C27 are
  candidate ids), for literature (from memory), nor for its operand-only
  working-set count at l1301 and hardware-only SPSA iteration count at l1257.
- verify_c: authoritative for every number it prints (= spec Sec. 8 and the
  expected values of T1, T3 (CRz part), T4-T6, T7 FD/shift parts). NOT
  authoritative for implementation conventions (it uses interleaved double
  complex, eps = 0), and it does not cover T2, T3 CRx (its line 91 comment
  promises a CRx check that is not coded), T7 adjoint/Sim15 sign, T8-T10.

- puremath: authoritative for the STATEMENTS AND PROOFS of its results
  (Chapters 1-7; 283 labelled definitions, lemmas, propositions, theorems,
  corollaries, remarks, examples and conjectures) and for its correspondence
  tables (Appendix A.1 result -> engineering labels; A.2 label -> result).
  NOT authoritative for any convention (its Sec. 0.1 restates spec C1-C17 and
  defers to them), any implementation value (kernel choice, number format,
  store rule, tolerance, test expected value, memory figure, hardware fact:
  the engineering documents govern, R8, R14), anything in its Appendix B
  (conjectures, UNVERIFIED statements, open problems: R7), or literature
  citation details (App. B items 6, 7, 17, 36). Its engineering line
  citations refer to the file versions it was checked against. It corrects
  the baseline only through a new erratum (Section 8.8).
- dsdm: authoritative for SCHEDULE (week offsets, timeboxes TB0-TB17, final
  deployment Weeks 37-38), PRIORITY (MoSCoW of every PRL row) and ACCEPTANCE
  SEQUENCING (which test closes which requirement in which timebox; gates
  G0-G7, including the P0 falsification gate G4 and its decision table), and
  for roles, vacancies, risk ownership and the definition of done. NOT
  authoritative for any technical statement: every number, convention, bound
  and test value it contains is quoted from a registered document, which
  governs; it inherits R1-R13 and cannot alter precedence; its estimates and
  velocity are UNVERIFIED planning assumptions (Section 10 group Q).

Verification-program folders (all plain C, fp64, build `cc -O2 -lm`; paths
relative to D; none are normative, all are ground truth for the numbers they
print). Sizes/hashes not pinned by any document (UNVERIFIED as frozen):

| folder / file | produced for | what it establishes |
|---|---|---|
| verify.c | spec Sec. 8 | see registry row |
| addendum/t18.c, t18b.c, mix.c | addendum Sec. 18 | T12-T14, T18-T20 values; mix32/FNV vectors |
| critic/adjtree.c, critic/chk.c | addendum 11.7, 14.3 | T24 tree adjoint (<= 1.5e-10 vs FD); capped MPS counts |
| derivH/jsvd.c, mpstest.c; verH/mps_nan.c, jsort.c, mps_*.c | addendum 14.3 | T15 Jacobi SVD, T21 MPS vs exact, rank guard, sort necessity |
| verE/frob.c, parse.c; refE/chkE.c, genE.c, cyc3.c | addendum 11.1-11.4 | chart parser vs exhaustive search; CFG statistics (cycle 25-27 %, greedy 46-58 %) |
| derivF/sym.c, vfy.c, zdense.c, zmc.c | addendum 12 | dof/rank checks, E[Z] MC (400 triples), order sensitivity |
| second/ez_mc.c, beta.c, bp_mc.c, num.c, parse.c | errata (second pass) | independent re-run of E-1, E-6, E-7, E-10, E-11, E-12 numbers |
| verEZ/ez.c; consist/ez.c, spsa.c, cyk_steps.c; scope/ez.c, exp0.c | review 2.1, 2.2, 2.10, 7 | E[Z] MC 1e5; SPSA; chart step count; P0 sizing |
| spsa_var.c, fp16_absorb.c, fp16_c12.c, errbound_check.c, errata_numbers.c, lipschitz_check.c, verify_export_bytes.c | review 2.2-2.9 | SPSA variance, fp16 stall k = 2049, C12 counterexample, 4x looseness, replacement numbers, L_k gains, 300,032 B |
| verQ15/q15_tensordot.c, q15_circuit.c, q15_sat.c | review 2.4 | Q1.15 endpoint/pre-scale, worked example 3.11e-5 |
| verLane/lane_coef.c | review 2.6 | 0 of 8 gates lane-uniform |
| amp72/amp72.c; bp/bp.c, bp2.c; parseK/parsechk.c; adj_dz/adj_dz.c; fit/fit.c; audit/audit.c | review 2.7, 2.8, 2.10, 2.11, 5, 3 | amplitude table; barren-plateau MC; chart vs exhaustive; D = Z, adjoint, sign_k; permutation invariance; companion audit |
| nonunit/nu.c; spec/spec_test.c; jacobi.c; gram/gram.c; ptrace/ptrace.c, grad.c, ptrace2.c; discocirc_ho/ho2.c; exact_train/et.c; wirerep/qd.c; lexicon/lex_test.c, fid_test.c; parsestruct/ps.c; critic2/extra.c, recompute.c | limits Themes A1-A8, B | all limits test vectors (A1-T1..A8, T-SF1..5, T-W2..W4, A5 A-H, A6, A7 A-C, B) |
| chk.c, chk2.c, gchk.c, memchk.c, ps.c, jacobi.c (top level); chk_dir/, refF/, verG/, report_parts/, lambeq_src/ | earlier verifier scratch | not cited by any document; lambeq_src/ and *.py were READ for provenance only, never executed (limits l1680) |


## 2. Precedence and conflict resolution

### 2.1 Decision procedure (apply in order; stop at the first rule that decides)

R1  Errata override the baseline at the cited lines. If the question touches a
    spec line named by an errata item (sNNN), the errata's replacement text is
    the specification; the spec file is never edited (errata l3-8). E-5 is a
    clarification (spec not wrong); E-1..E-4, E-6..E-13 are corrections.
    Errata Sec. B (l206-217) lists companion claims that need NO change.

R2  Baseline conventions C1-C13 govern every document unless an erratum says
    otherwise. The only erratum touching Sec. 0 is E-8 (C12 absorption
    criterion). No other document may redefine a convention; the addendum
    obeys them verbatim (addendum l3-4); the limits report cites them (l3).

R3  Addendum C14-C17 and Secs. 11-19 extend and never override. Where the
    addendum restates a baseline constant with another value (2^{q_n+1} at
    a708/a788) the errata's value governs; where the errata names an addendum
    line (a107, a130-131, a486/488, a708/a788, a721, a726, a1560/1566/1657,
    Sec. 19 SPSA) read the errata over it. Elsewhere the addendum is the
    technical document of record for grammar coverage, capacity, hybrid,
    discourse/MPS, positioning, extended memory table, export extension C15.

R4  Review verdicts govern over the companion's wording. "Correct" in the
    companion register RV01-RV60 corrects the third-party discussion's
    wording, never the baseline (review l340-342). The companion is adopted as
    governing only after review conditions (i)-(v) (review l31-37); on disk only
    (i) (the errata table) is discharged. Until then the companion is a
    validated corrections memo plus constraints.

R5  The companion's requirements are constraints; its technical corrections
    are superseded by the errata. A companion requirement binds an implementer
    only when it maps to a document of record that supplies the procedure or
    numbers (then that document is the source) or when it is recorded as a gap
    (Section 11). Requirements attached to companion Secs. 10-12.8 and 14 (ids
    R4, R5, R8; RV16-24, RV34-41, RV43-60) are deferred and bind nothing before
    P0 reports (review l35, l136-137, l172-175).

R6  The hardware-limits report is advisory and cites the addendum where they
    overlap. Its labels (NU.n), (SF.n), (G.n), (T.n), (HO.n), (L.n) and local
    conventions (LIM-*) apply only inside kernels imported from it. A limits
    "replaces" statement ((G.9) vs (11.27'); linear seed vs (5.8)) means
    "replaces for the new objective it introduces". Its Haar-expectation
    figures (4^{-q} per cup) never replace exact conventions (C8, (1.4)).

R7  UNVERIFIED beats asserted. A statement tagged UNVERIFIED in any document,
    or not backed by a CONFIRMED erratum, normative spec/addendum text, or a
    listed fp64 program, is treated as unverified; a later document cannot
    upgrade it by restating it without a program or measurement. Nothing in
    the set is MEASURED on a device, dataset or compiler; register residency is
    a ptxas-verified TARGET (addendum C14 l22-24).

R8  Numeric precedence, per claim at the cited line:
    verify_c (for the Sec. 8 numbers it prints) > errata > review > addendum >
    limits > spec > companion. Where a higher document gives a formula or factor
    but does not recompute a downstream number, recompute by that formula and
    label the result UNVERIFIED (Section 9 lists these). Rounding differences
    (0.144 vs 0.1443; 0.0033 vs 0.00328) are not conflicts: quote the errata's
    form, cite the review for the derivation. Review self-corrections resolve
    to the later value (Rz suppression ~400x, p_min 0.00328, 28-29 registers).

R9  Unresolved conflict: record it (Section 2.2 format: topic, a, b,
    resolution, rule), prefer the more recent derivation that has a program
    behind it, apply the rules above, and flag it to the engineer with the
    proposed erratum text; never resolve it silently in code. If it changes the
    baseline, it becomes a new erratum (Section 8.8).

R10 Symbol namespacing. A symbol of spec C9 / Sec. 0 (D, S, N, W, u, Ng,
    p_surv, m, K, M_post, b(c)) keeps its meaning in every kernel formula; other
    uses are qualified (D_V, D_s, depth D, S_q15, S(rho), S_k, N_nouns,
    m_phys/m_arg). Canonical spellings d_n = 2^{q_n}, d_s = 2^{q_s} (spec 1.6
    l343). Identifiers Cn (1..17), Tn (1..24), (S.n), (11.n)-(19.n) are reserved
    for spec/addendum/errata; same-shaped ids elsewhere are cited with a
    document prefix ("limits candidate C7", "limits A1-T3", "companion RV09").

R11 Form scoping. Circuit-form rules (C11 transpose, sign_k = -1 for a
    transposed Ry, shift rules (5.2)-(5.4), W_hw, TRANSPOSED/POST*/CUP_BELL
    opcodes, unfused adjoint list) apply only to the bent-wire circuit (spec
    1.4/5.3, addendum 11.5). Tree-form rules (addendum 11.7, sign_k = +1, node
    ops C16) apply to Sec. 11 sentences on the simulator. DisCoCirc rules (C17)
    apply to Sec. 14 text registers. Doubled-register rules (limits LIM-7/8,
    NU.5) apply only to density-matrix kernels. Every emitted gate carries its
    form.

R12 Statistical vs exact. Exact conventions (C8 Bell factor 2^{-q} per cup;
    (1.4) P_post = Z / 2^{sum q_c}; D = Z in bent-wire form) are never replaced
    by Haar expectations (E[Z] = 2^{-2 q_n}, E[P_post] = 2^{-4 q_n}, A = 2^{q_n});
    the latter are used only for floors, p_min and format selection.

R13 Units and identity. KB = 1024 B, MB = 1024 KB (addendum C14 l17); companion
    KiB/MiB are the same. Where a document gives exact bytes, the byte figure
    governs (so spec "100 KB" model row = 100,000 B). Documents are identified
    by SHA-256, never by path (review l369).

R14 Registered companions (added in 1.2). puremath is authoritative for proofs
    and for the statements of its results; it ranks with the review for
    derivations and below the errata for any spec line (Section 12 step 2).
    The engineering documents (spec as overlaid by the errata, addendum,
    limits for its own themes, verify.c for what it prints) remain
    authoritative for every implementation value; where puremath states a
    number, R8 still decides, a disagreement is recorded under R9, and a
    baseline change still needs an erratum (Section 8.8). Items puremath
    labels Conjecture or UNVERIFIED (its Appendix B) carry no authority (R7).
    dsdm is authoritative for schedule, priority (MoSCoW) and acceptance
    sequencing only; it has no technical precedence, inherits R1-R13, and
    where it quotes a technical value the cited document governs.

### 2.2 Known conflicts and their resolutions

Each row: topic | side a | side b | resolution (governing text) | rule.

1. fp16 absorption criterion (C12) | spec l113 "below 2^-11 of the running
   sum" | errata E-8 l95-98, review l165-167 | absorbed when below half an ulp
   of the sum, guaranteed for term <= 2^-12 x sum; design rule unchanged | R1.
2. E[Z] for Haar words | spec l218 "E[Z] = 1" | errata E-1 l13-28 (value l14,
   Beta law l15, |K| = 2 q_n l23, "(2,1) untrained mean equals the fp16 floor"
   l27); addendum a705-712; companion c384-396; review l55-84 | E[Z] = E[D] =
   2^{-2 q_n}; spec T6 (l1272, Z = 0.25) and s967 already agree; test T-EZ;
   numbers at q_n = 2 in Section 9 "Statistics of Z and D" | R1, R8.
3. Constant in (4.7)/(4.9) | spec l856, l926 factor 4; companion c406 "order
   of e/||sigma||" | errata E-11 l149-165; review l253-276 (exactly 4x loose)
   | first order e/sqrt(p_surv); all orders 2e/(2 sqrt(p_surv) - e);
   certificate 2e/(2 sqrt(D) - e) iff sqrt(D) > e | R1, R5.
4. Amplification factor A | spec l871 A = 4/sqrt(p_surv) (12.65 at 0.1),
   untrained 2^{q_n+2}; addendum a708, a787-788 2^{q_n+1} | errata E-11 l158, l165 |
   A = 1/sqrt(p_surv) (3.16 at 0.1); Haar-init factor 2^{q_n} | R1, R3.
5. Inference D floors | spec l1104-1108 two floors 2^-6 (Q1.15) / 2^-4 (fp16-
   store), s950 "0.078 FAILS"; review l81-83 (pre-E-11, "median" meaning
   mean); addendum a708 (2^{q_n+1}) | errata E-11 l160-163; E-10 l145; E-1 l27
   | single floor D >= 2^-6 on both paths (0.0197 passes; tight 2^-8.6); fp32
   fallback default for |K| >= 6 (q_n >= 3 for N TV N) unless training raises
   D. Link to E[Z]: at q_n = 2 the untrained mean E[D] = 2^-4 equals the
   superseded fp16-store floor (errata l27), so that floor would have rejected
   ~59 % of Haar sentences (Section 9) | R1, R8.
6. Q1.15 tensordot narrowing | spec l930 narrowed once, no scale factor;
   companion c412 (path unnamed) | errata E-6 l77-87; review l131-154 |
   pre-scale words by S_q15 = 1 - 2^-15; circuit path (0x7FFF init) unchanged;
   without pre-scale constants become sqrt(2n+3) 2^-16 | R1, R5.
7. HVX sentence-per-lane coefficients | spec l709-713 scalar Rt.h operands,
   12 spare registers; addendum a721 WS <= 13; companion c480-484 "recalculate"
   | errata E-9 l100-124; review l170-200 | lane-varying Q2.14 vectors, 3
   registers per rotation gate, peak 27-29 of 32, no spill; WS <= 11; vgather
   v65+; V6_vmpyh_acc is v65 | R1, R3.
8. Initialisation | spec l1072 uniform OR N(0, 0.1^2) | errata E-10 l126-147;
   review l221-251 | uniform [0, 2 pi) with fixed seed only; >= 3 restarts;
   theta = 0 is an IQP saddle; addendum (13.1) generator needs non-zero bias |
   R1.
9. Production parser | spec l162-163 "a stack suffices", l176-179 DFS |
   errata E-12 l167-192; addendum 11.1 a80-140; review l278-307 | chart
   (11.4)-(11.5) at (m-1)(m+1)(m+3)/24 partner tests; greedy = six-shape
   pre-check only; no DFS; addendum a107 m^3/12 -> m^3/24 | R1, R3.
10. Slot rule and layout labels | spec l258 (sufficient), l1163 "Layouts B/C"
    | errata E-13 l194-204; review l214-219 | one 8 B complex accumulator
    suffices; "(Layout C only) ... 0 (Layouts A/B)" | R1.
11. Barren-plateau exponent | spec l1073 Var ~ 2^-W | errata E-10 l133-136 |
    projector variance 2^{-2W}; normalised readout flat Var[dP_c/dtheta] ~ 0.06;
    real hazards 1/D tail, saddle, local minima | R1.
12. SPSA variance | spec l1051-1052 (P-1) mean g^2 / c^2 | errata E-2 l30-39;
    companion c432; review l86-107 | sum_{j!=i} g_j^2 + sigma^2/(2c^2); c ~
    eps_mach^{1/3}; O(P) more steps stands | R1.
13. Model memory row and "no weight table" | spec l1148, l1172-1178, l1137-
    1139; addendum a1560/1566/1657 | errata E-3, E-4, E-5; companion c376-380;
    review l109-129 | angles 100,000 B (weights); records + header 200,032 B in
    the file only; resident 100-140 KB; claim = materialised objects 0 B +
    working set 40-400 B | R1, R3.
14. Training footprint 16 x P_total | spec l1165 96 KB in a V = 1e4 table |
    errata E-3 l47 | 96 KB at P_total = 6000 (V = 1e3); 800 KB at 50,000 | R1.
15. fp16 stall in T8 | spec l1281-1282 "sum of 2^11 terms stalls" | errata
    E-7 l89-93; companion c914; review l156-164 | exact to 2048 (sum = 1.0),
    first stall at term 2049; 2^-12 series stalls at 0.5 | R1.
16. Symbol D (three meanings) | spec C9 l81 D = postselection denominator |
    errata E-1 l15 / review l57 D = 2^{Q_V}; limits l526 D = 2^{q_s}; addendum
    l186 nesting depth; (12.17) discourse word | C9 governs bare D; others
    qualified D_V, D_s, depth D, word D | R10.
17. Symbol S | spec l297-300, l846 surviving s field / index set | errata E-6
    S = 1 - 2^-15; limits S(rho), S superoperator, S sliced wires; addendum
    (14.10) singular values | spec governs; S_q15, S(rho), S_k, S_sliced | R10.
18. Symbol N | spec C9 l84 N = 2^W | addendum C17 l58 N = noun wires | C9 in
    Sec. 2-4 cost formulas; C17 in Sec. 14 / limits A4-A5; write N_nouns when
    both appear | R10.
19. d vs d_n | addendum l574, limits l15 d = 2^{q_n} | spec 1.6 l343 d_n |
    canonical d_n, d_s; d accepted only when quoting addendum/limits | R10.
20. Hermitian packing of rho | spec 1.6 l347 upper triangle | limits LIM-11
    l376 lower triangle; addendum (13.10), (14.19) full row-major | spec
    packing for stored objects; full rho[a][a'] (ket row, bra column) agreed
    everywhere; convert at kernel boundary | R2, R6.
21. Ids C1..C27 | spec/addendum conventions | limits Sec. 3 l1481-1536 tier-C
    candidates | bare Cn = convention; "limits candidate Cn" otherwise | R10.
22. Ids T1..T5 and T-series extent | spec T1-T10, addendum T11-T24 (T24 at
    a1880 although the programme summary says T11-T23) | limits A1 tests T1-T5b
    | canonical T1-T24 + T-EZ + errata-added checks; "limits A1-Tn" | R10.
23. Relative pronoun type | spec l176 n^r n s^l n (unlabelled) | addendum
    a169-170 (subject n^r n s^l n, "Alice who loves Bob sleeps", cups
    (0,1)(4,5)(7,8)(3,6)(2,9), s at 10) and a171-172 (object n^r n n^{ll} s^l,
    "Alice whom Bob loves sleeps", cups (0,1)(5,6)(4,7)(3,8)(2,9), s at 10);
    review l292-293. (a173-175 are the possessive and wh-question rows, not
    relative pronouns.) | addendum governs; E-12 l186 fixes the test | R3.
24. sign_k and "the training circuit" | spec l268, l1029, l1213 sign_k = -1 for
    transposed Ry | addendum a428-450, a537-538 sign_k = +1 in the tree; limits
    (NU.5) l291 | scoped: circuit form -1, tree form +1, doubled register per
    NU.5; each gate tagged with its form | R11.
25. Root cotangent seed | spec (5.8) l1019 = addendum (11.27') l521 | limits
    (G.9) l580-582 "REPLACES"; A1 l286-287 linear seed; A6 l1055 e_c; A8 l1335
    (L.4) | (5.8)/(11.27') for CE/MSE on P_c; limits seeds for their own
    objectives | R6.
26. Local 2q index m | spec C6 l48-49 m = bit_lo + 2 bit_hi (physical order);
    addendum C17 l58-64 pi permutation for dense forms | limits (HO.1) l849
    argument order | m_phys (C6) reference; m_arg (HO.1) equals it iff w[] is
    ascending; passing w = (subj, obj) = applying C17's pi | R2, R6.
27. Export type signature extension | companion c498 "do not force new types
    into six-atom type_sig" | addendum C15 l26-36 | C15 governs for the
    linguistic profile (and_vp has exactly 6 factors); companion constraint
    survives for typed-record profiles; missing fields r, chi, readout, grammar
    version, checksum (review l402) | R3, R5.
28. Meaning of "Correct" | companion c944 | review l340-342, l542 | corrects the
    third-party review's wording; baseline corrections are E-1..E-13 only | R4.
29. Per-cup postselection factor | spec C8 l79, (1.4) l215, l1218 Z/4 |
    limits l1123, l1563 4^{-q} per cup | exact: 2^{-q} per cup times Z; limits'
    figure is the Haar EXPECTATION (2^{-q} Bell x 2^{-q} share of E[Z]) | R12.
30. Storage/eps in verify.c | spec C3, (1.7) eps = 1e-9 | verify.c l7
    interleaved double complex, l39-40 eps = 0 | spec governs implementations;
    verify.c is reference only | R7-numeric, R8.
31. Label "IQP" for Q = 1 nouns | spec C10 l89-90 Rx Rz Rx list | verify.c l24
    comment / iqp() | C10 governs naming | R2.
32. Companion governing status and scope | companion c64, c132, c1014, Secs.
    10-12.8, 14 | review l31-37, l136-137, l148-175 | companion = corrections
    memo + constraints; committed programme = P0; Secs. 10-12.8, 14 deferred |
    R4, R5.
33. Open decisions (companion 19.2, 14 items) | no defaults | review l166-170 |
    seven answered by the baseline (pregroup + greedy/chart; IQP ansatz 0
    uniform init; (1,1) L = 2 then (1,3); CFG data; CUDA Layout A fp32 train /
    HVX Q1.15 infer; certificate acceptance; no first extension); seven gate
    nothing before companion stage R4 | R4.
34. Expressivity ceiling (companion RV09 vs addendum 12.2) | c956 "unsupported"
    | a666-670 "independent of q_n, q_s, L" | review l79-83: addendum correct
    but qualified to d << |V|; edit owed to the addendum | R3, R4.
35. Lorenz et al. SPSA iterations | limits l1257 "100 iterations" | addendum
    a1460-1462 500 (simulation), 100/130 (ibmq) verified against qnlp.txt |
    addendum governs | R6.
36. Fused-tensordot working set | limits l1301 12 / 40 amps (operands only) |
    spec l254, addendum (12.11) 16 / 48 full, 10 / 36 in place | budget with
    WS_full 16 / 48 (128 / 384 B fp32) and WS_inplace 10 / 36 | R6. NOTE a
    third count: addendum a241 and Sec. 16 a1683 give N TV N and the relative
    clause as 14 / 44 (= 2^{2q_n+q_s} + 2^{q_n} + 2^{q_n+q_s}, no second
    + 2^{q_n}); the extra 2^{q_n} of WS_full is the second noun held live.
    Quote 14 / 44 when citing a1683 / T12 / T22 for a relative clause and
    16 / 48 when budgeting the spec's WS_full; state which.
37. Matched-memory baselines | addendum a1637-1640 (V = 1e4 / 1000) | review
    l557-558 (V = 64) | both valid for their vocabulary; P0 uses the review's;
    state V beside every byte count | R8.
38. Units of the 100 KB model row | spec l1148, addendum C14 l17 "Sec. 7
    already follows KB = 1024" | errata/companion/review 100,000 B | byte
    figure governs; "100 KB" is shorthand for 100,000 B | R13.
39. Addendum precision numbers derived with the superseded factor (a708-709
    1.6e-2, 0.25, 2.5e-2, p_min 1.6e-3; T23) | errata E-11 changes factor but
    recomputes none | recomputed halves (8e-3, 0.125, 1.25e-2) and p_min
    (0.01 + 0.00025)^2 = 1.05e-4 -- UNVERIFIED; T23's 2.5e-2 stays a loose
    ceiling | R8.
40. Hardware-limits report existence | review l6-7, l384 "absent" | file exists
    (registry) | review statement stale; limits obeys E-1..E-13 (l488) | R7.
41. Entropy log base and zero-eigenvalue guard (inside limits) | limits A2
    (SF.4), kernel l409: natural log, clamp lam > 1e-7f (T-SF1/T-SF2 report
    nat) | limits A4 (3)(d) l683-684: log2, guard lam <= 1e-15 (T-W4 / T-SF5
    report bits) | fp32 kernels use the A2 clamp 1e-7 (normative text limits
    Sec. 2.2 (8)(iii) l473-474; the T-W4 pass criteria l775-777 cite it); the
    1e-15 guard is for fp64 references only (T-W4's two zero eigenvalues come
    out ~1e-8 or slightly negative in fp32 and would pass a 1e-15 guard into
    log, giving NaN or a spurious term); report units
    (bit or nat) explicitly; S_bit = S_nat / ln 2 | R6, R9.
42. 4x4 Jacobi cost | limits A4 table l700 "entropy 4x4 ~2000 flops, 32 fp32"
    | limits A2 (4) l413-418: 1,152 real flops per sweep without V (1,728 with
    V) x 3 sweeps at d = 4 = ~3,456 (5,184 with V), plus 6 atan2/sincos per
    sweep on the scalar core | A2 governs (derivation shown); A4's ~2000 is
    an under-estimate by ~1.7x (recomputed here, UNVERIFIED as a measured
    count) | R6, R8.



Registered documents and the rows above (1.2, Section 12 step 4). puremath
supports, with proofs, the resolutions of row 2 (Z ~ Beta(d_s, d_V - d_s),
E[Z] = 2^{-2 q_n}; its Sec. 3.4), rows 3-4 (tight amplification constant 1,
not 4; Theorem 6.27, Sec. 6.6), row 8 (saddle at the origin of IQP models,
mixed derivative -1/4; Sec. 5.11), row 9 (chart recognition exact, stack
reducer incomplete; exact cost (m-1)(m+1)(m+3)/24 partner tests for odd m,
Theorem 1.11, which adds the even-m form m(m+1)(m+2)/24), row 12 (SPSA
variance sum_{j != i} g_j^2, independent of c; Sec. 5.9) and row 15 (fp16
partial sums exact to 2048 terms of 2^-11, half-ulp tie at the 2049th;
Sec. 6.7). It contradicts no resolution, so no row is opened under R9. Its
C8 statement leaves the cup pairing of the reference implementation
UNVERIFIED, consistent with spec C8 and gap G5. dsdm takes no technical side.


## 3. Convention registry

### 3.1 C1-C17 (one line each; defining document and line)

| id | statement | defined at | corrected by |
|---|---|---|---|
| C1 | Bit order little-endian: qubit j = bit j of index i, bit_j(i) = (i >> j) & 1; qubit 0 is LSB and fastest-varying; ket written b_{W-1} ... b_0 | spec l13 | -- |
| C2 | Kronecker order: LEFT factor = HIGH bits, (A (x) B)[rA*dimB + rB][cA*dimB + cB]; Kronecker notation avoided for words (linguistic order is the opposite), explicit index maps (C7) used instead | spec l17 | -- |
| C3 | Storage split planar re[]/im[] of one scalar type (fp32, fp16, int16 Q1.15); real ansatz re[] only; CUDA may use interleaved float2 per thread; matrices row-major, column vectors, psi' = U psi | spec l22 | -- (verify.c deviates by design) |
| C4 | Operator products read right-to-left; GATE LISTS read left-to-right (first listed = first applied); every ansatz is a gate list | spec l28 | -- |
| C5 | R_P(theta) = exp(-i theta P/2); Rx (0.1), Ry (0.2), Rz (0.3), H (0.4); angles stored as uint16 half-angle turns k, phi = 2 pi k/65536, theta = 4 pi k/65536, |d_phi| <= pi/65536 = 4.8e-5 rad; 4 pi-periodic in theta; CUDA sincospif(k/32768.0f); lambeq theta = 2 pi phase UNVERIFIED | spec l32-45 | -- |
| C6 | 2q gates: qubit numbers lo < hi, local index m = bit_lo(i) + 2 bit_hi(i), dense U4[m_out][m_in]; controlled gates DEFINED by bit tests (0.5)-(0.8); CRz symmetric phases e^{+-i theta/2}; diag(1,1,1,e^{i theta}) is a DIFFERENT gate | spec l48-60 | -- |
| C7 | Word factors t_1..t_m left to right; factor i at local offset fo_w[i] = sum_{i'<i} q(t_{i'}) (0.9); FIRST factor = LOWEST bits; V[a,s,b] = phi[a + 2^{q_n} s + 2^{q_n+q_s} b]; off_w, start(p), width(p) | spec l62-71 | -- |
| C8 | Cups = UNNORMALISED Bell effect sum_k <k|<k| paired component-wise (bit r with bit r); nested pairing is a different valid model (flags.bit1); circuit: CNOT(a->b), H(a), postselect 00 = contraction x 2^{-q/2} per cup | spec l73-80 | -- |
| C9 | Symbols: Ng gate count; D(theta) = postselection denominator, never a gate count; u = unit roundoff (fp32 2^-24, fp16 2^-11, bf16 2^-8, Q1.15 store 2^-16 abs); p_surv; W, N = 2^W; FMA-class op; complex MAC = 4 | spec l81-84 | -- |
| C10 | Ansatz ids 0 IQP no closing H, 1 IQP + closing H, 2 Sim14, 3 Sim15, 4 Ry-only (real); Q = 1 words: Rx(t0) Rz(t1) Rx(t2), P_w = 3 (Ry-only: 1); IQP layer H all + CRz(j->j+1), P_w = L(Q-1); Sim15 P_w = 2QL; Sim14 P_w = 4QL; Ry-only P_w = Q; ring orders UNVERIFIED vs lambeq; ansatz 0 L = 1 has |amp| = 2^{-Q/2} | spec l86-102 | addendum a1201-1203 PROPOSES id 5 (undefined); C15 adds 0xFE spider marker |
| C11 | U_w^T = reversed gate list with each gate transposed; Rx, Rz, H, CNOT, CZ, CRz, CRx symmetric; Ry(theta)^T = Ry(-theta) | spec l104-108 | -- (limits LIM-8 l225 gives the complementary conj rule) |
| C12 | fp16 facts: u = 2^-11, max 65504, min normal 2^-14; |psi|^2 ~ 2^-W subnormal from W = 15; probabilities NEVER accumulated in fp16 | spec l110-113 | errata E-8 l95-98 (absorption = below half an ulp; term <= 2^-12 x sum) |
| C13 | Paulis (0.10); generators X/Y/Z (G^2 = I); CRz generator |1><1|_c (x) Z_t, CRx |1><1|_c (x) X_t with eigenvalues {0,+1,-1}; dense 4x4 forms (0.11); lift_lo/lift_hi (0.12); dense W-qubit reference M for T2 | spec l115-142 | -- |
| C14 | KB = 1024 B, MB = 1024 KB; tiers REGISTER (CUDA thread ~1 KB / HVX file 4 KB), VTCM (8 KB slice unless stated; 8 MB on v73+), SMEM (CUDA 48/99-227 KB; Adreno local 32 KB), HBM; "fits in registers" = ptxas-verified target (0 spills), not a fact | addendum l17-24 | -- |
| C15 | Basic types B = {N, S, I, Q}, q(I) = q_i @20, q(Q) = q_q @21 (0 = same as q_s); type_sig base codes 0 n, 1 s, 2 i, 3 q; SPIDER word ansatz_id = 0xFE (P_w = 0); accept(p, target); flags.bit3 = generator block; param_offset 0xFFFFFFFF = generate; version += 1 | addendum l26-36 | review l402 lists missing fields (r, chi, readout, grammar version, checksum) |
| C16 | struct Cup {u8 i, j}; struct Node {u16 word; u8 nchild; u8 child[4]; u8 cmask[4]; u8 pmask; u8 kind} = 14 B; kind 0 unitary, 1 spider, 2 CHAIN; nchild <= 4; tree[] post-order, root last; one node-op instantiation per (type string, cmask, pmask); device gets cups[] + tree[] | addendum l38-56 | -- |
| C17 | DisCoCirc: noun wire j = field [q_n j, q_n(j+1)) of W = q_n N_nouns; local qubits 0..q_n-1 SUBJECT, q_n..2q_n-1 OBJECT; wire_map(j); dense form permuted by pi(m) when i_subj > i_obj; gate emission order (adj/det, verb, adverb); head-noun rule; readout = RDM (14.19) or final effect word | addendum l58-70 | -- |

Companion-level rules CP-1..CP-36 (ids assigned by the inventory, not by the
companion) are constraints, not conventions; see Section 8 procedures and
Section 11 gaps for the ones that bind. Limits LIM-1..LIM-28 are ids
assigned by the inventory behind this bridge, NOT labels in the limits file
(grep "LIM-" on the limits file finds 0 hits); they apply only inside the
limits report's kernels (R6). Resolve each by the limits line quoted beside
it: LIM-8 l225 (conjugation rule); LIM-10 l356 (F_root vs F_sq); LIM-11 l376
(lower-triangle packing); LIM-12 = Sec. 2.2 (8)(iii) l473-474 (fp32 entropy
clamp lam <= 1e-7, kernel l409); LIM-13 l526 (D_s); LIM-19 l1132 (mixed
radix); LIM-24 l1308 (verb-resident lexicon). LIM-7, -9, -14, -15, -21, -22
have no line recorded here (UNVERIFIED mapping): search the limits text for
the named object instead.

### 3.2 Angle encoding (single source of truth)

- Stored form: uint16 k = half-angle turns; phi = theta/2 = 2 pi k / 65536;
  theta = 4 pi k / 65536 (spec C5 l39-41).
- Export quantisation: k = round(theta / (4 pi) x 65536) mod 65536 from the
  UNWRAPPED fp32 theta (spec 6.3 l1135); d_phi <= 4.8e-5 rad.
- Codec check values: theta = 0.8 -> k = 4172 (decode 0.399985 rad, error
  1.45e-5) (spec l1234, T10 l1285); z = -1.0 -> 60321; 13.0 -> 2261; NaN -> 0
  (addendum (13.2) l894-897, T19 l1867). Integer path k = (uint16)(((int64) acc
  x Cc) >> 32), Cc = llround(s_E s_W 65536/(4 pi) x 2^32) (addendum l896).
- Periodicity: single-qubit rotations 2 pi in theta; controlled rotations only
  4 pi; the uint16 encoding is 4 pi-periodic and therefore safe; wrap only at
  export and only mod 4 pi (spec 5.6 l1076-1085; verify.c l95-98: p0(t + 2 pi)
  = 0.87639280 != p0 = 0.58263882 = p0(t + 4 pi)).
- Alternative table form: flags.bit0 = 1 stores Q15 (cos phi, sin phi) pairs,
  round(cos phi x 32767) (spec l1132); on-device cos/sin from k by octant
  reduction and degree-7/8 Taylor, or a 257-entry Q2.14 table (514 B) (spec 4.4
  l830-843); degree 5/6 Taylor fails 2^-15.
- fp16 angle storage is forbidden (ulp near pi = 2e-3 rad) (spec l792).

### 3.3 Ansatz ids

| id | name | Q = 1 word list | Q >= 2 layer | P_w | real? | transpose (C11) |
|---|---|---|---|---|---|---|
| 0 | IQP, no closing H | Rx Rz Rx | H all; CRz(theta)(j->j+1), j = 0..Q-2 | 3 (Q=1); L(Q-1) | no | reversed order, same angles |
| 1 | IQP with closing H | Rx Rz Rx | as 0 then H all | 3; L(Q-1) | no | reversed, same angles |
| 2 | Sim14 | Rx Rz Rx | Ry all, CRx(j->(j-1) mod Q), Ry all, CRx(j->(j+1) mod Q) | 3; 4QL | no | reversed, Ry negated |
| 3 | Sim15 | Rx Rz Rx | Ry all, CNOT(j->(j-1) mod Q), Ry all, CNOT(j->(j+1) mod Q) | 3; 2QL | yes | reversed, Ry negated |
| 4 | Ry-only | Ry | Ry each, CNOT ladder (0->1),(1->2),... | 1; Q | yes | reversed, Ry negated |
| 0xFE | SPIDER (per-word marker) | -- | tensor by index equality (11.6)-(11.13') | 0 | -- | -- |
| 0xFF | per-word "use header default" | -- | -- | -- | -- | spec l1128 |
| 5 | PROPOSED full U(4) KAK/Cartan, >= 15 angles | undefined | undefined | undefined | -- | UNDEFINED (addendum l1201-1203; gap G3) |

Source: spec C10 l86-102; addendum C15 l29; spec 6.3 l1116, l1128.
Sim14/Sim15 ring direction and IQP closing-H vs lambeq are UNVERIFIED (spec
l87, l99); the chosen order is the one written above and must be stated in the
export header (no field exists for it: gap G2).

### 3.4 Export format version fields

Header (32 B, little-endian; spec 6.3 l1113-1116; addendum C15 l26-36):
  @0 magic "QNLP" (u32 0x504C4E51); @4 version u16 (baseline value NOT stated
  in spec l1115 -- UNVERIFIED; this bridge assumes 1; C15 bumps by 1 for I/Q
  types, spider marker, generator block); @6 flags u16: bit0 angle
  encoding (0 uint16 turns, 1 Q15 cos/sin pairs), bit1 cup pairing (0
  component-wise, 1 nested), bit2 real-only, bit3 generator block present
  (C15); @8 V u32; @12 P_total u32; @16 q_n u8; @17 q_s u8; @18 L u8; @19
  ansatz_id u8; @20 q_i u8 (C15); @21 q_q u8 (C15); remainder reserved.
Word record (20 B, packed, V records from @32; spec l1118-1129):
  @0  word_id u32 (index into an external string table);
  @4  type_sig u8[6] (byte = base | adjoint code << 2; base 0 n / 1 s (C15
      adds 2 i, 3 q); adjoint codes 0 none, 1 .l, 2 .r, 3 .ll, 4 .rr; 0xFF
      end; byte 0 = leftmost atom = lowest qubit field (C7); > 6 atoms
      unsupported; spec l1120-1127);
  @10 n_qubits u8;
  @11 ansatz_id u8 (per-word override; 0xFF = header default; 0xFE = spider,
      C15);
  @12 P_w u16;
  @14 param_offset u32 (entry index into the angle table; 0xFFFFFFFF =
      generate, C15);
  @18 reserved u16.
Angle table @ 32 + 20 V (spec l1130-1132): P_total x uint16 k (2 B, flags.bit0
  = 0, default) or P_total x (int16 round(cos phi x 32767), int16 round(sin
  phi x 32767)) (4 B, flags.bit0 = 1).
Generator block (addendum 13.7 l1123-1163): 16 B header magic "QNLG" (m,
  B_log2, nclass, ctx, feat, r_or_mh, s_E); 32 B class records (P_c, m_in,
  off_W, off_b, s_W, Cc, type_sig[6], ansatz_id, L); type classifier; E;
  context; blobs; loader checks: exact Cc equality, T19 codec vectors,
  mix32(0) = 0x92CA2F0E.
Sizes: file = 32 + 20 V + 2 P_total (turns) or + 4 P_total (pairs).
  V = 1000 (600 N, 250 TV, 100 ADJ, 50 IV; spec l1062) at (1,1), L = 1:
  Sim14 6000 params 32,032 B (pairs 44,032 B), IQP 2450 params 24,932 B
  (pairs 29,832 B) (spec l1134; verify_export_bytes.c); IQP L = 2 (the
  baseline (1,1) choice, conflict row 33) 3100 params 26,232 B (pairs
  32,432 B) -- recomputed here by the formula, UNVERIFIED. Resident on device
  = angle table + 0 or 4 B/word offset lookup (errata E-5 l70-73); records
  are not loaded.
  V = 1e4, P_w = 5: 300,032 B (turns) / 400,032 B (pairs) (review l111,
  errata E-3); generator (1,1) IQP L = 2 m = 32: 808 B; (5,2) Sim14 L = 2
  m = 384: 120,052 B (addendum l1150).
T10: type_sig of n^r s n^l == 08 01 04 FF FF FF; record stride 20.


## 4. Symbol and notation index

Format: symbol | meaning | defining location | notes on collisions.

- psi[i], W, N | state array of 2^W complex amplitudes; W = qubits; N = 2^W |
  spec C1 l13, C9 l84 | N = noun-wire count in addendum C17 l58 and limits A4/A5
  (write N_nouns, W = q_n N_nouns).
- bit_j(i) | (i >> j) & 1 | spec l14.
- re[], im[] | split-planar planes | spec C3 l22.
- c, s (per gate) | cos(theta/2), sin(theta/2) | spec l32.
- k (uint16), phi | half-angle turns; phi = 2 pi k/65536 | spec l40 | k also the
  readout index in (1.6) l331 and the C1 sentence-wire index in limits LIM-14.
- m (local 2q index) | bit_lo + 2 bit_hi | spec C6 l49 | m = flip mask (spec 2.4
  l467); m = feature dimension (addendum Sec. 13 l878); m = type-string length
  (addendum 11.1, errata E-12); limits HO.1 argument-ordered m_arg.
- lo, hi, c, t | qubit numbers of a 2q gate; control, target | spec l48-50.
- q_n, q_s, q(t), q_i, q_q | qubits per noun/sentence/infinitive/question type |
  spec C7 l65; addendum C15 l26.
- d_n, d_s, d | 2^{q_n}, 2^{q_s}; addendum/limits write d = d_n | spec 1.6 l343;
  addendum l574; limits l15.
- Q_w, Q, fo_w[i], off_w | qubits of word w; Q = 2 q_n + q_s for a TV; factor
  offset; word start | spec l65-71; addendum l574.
- start(p), width(p) | field of global type position p | spec l71; addendum l139.
- phi_w, V[a,s,b], IV[a,s] | word state; verb view; intransitive view | spec l67,
  (1.5) l221; addendum l216.
- (b, z) | simple type: basic b, adjoint index z (^l = z-1, ^r = z+1) | spec l148;
  addendum (11.1) l84.
- cups (A_c, B_c), S (surviving position), shape | parser output | spec l162,
  l181, l205 | S also = index set {i : (i & M_post) == 0} (spec l846) and the
  s field in b(c) = c << S; S_q15 = 1 - 2^-15 (errata E-6); S(rho) entropy
  (limits A2); S superoperator (HO.3); S_k singular values (addendum (14.10)).
- sigma[s], Psi, idx | unnormalised sentence state; product state (never
  formed); gather address | spec (1.3) l205-206 | sigma^2 = SPSA evaluation
  noise variance in errata E-2.
- Z, Z_min, sigma_hat, P_post | norm sum |sigma|^2 (= D = p_surv in bent-wire
  form); guard 1e-12; normalised sentence; Bell postselection probability
  Z/2^{sum q_c} | spec (1.4) l215-217.
- T, restT, cupT, R | fused-tensordot open tensor, its non-cup fields, cup
  fields, result | spec 1.3 l231 | T = trace of a non-unitary update (limits A1
  l205); T = InfoNCE temperature (limits G.4); T-series = tests.
- effect word / state word, W_hw, W_train | bent-wire classification; minimum
  circuit width; uncompacted training width | spec l270-275; addendum (11.23)
  l416, (11.23') l434.
- wire_map[j] (effect-word array) | physical qubit that effect-word factor j
  cups to | spec 1.4 l291 | DIFFERENT object from addendum C17 wire_map(j)
  (function, l60-62) and limits A5 slot_of_wire[] (l846). Suggested names:
  effect_wire_map[], text_wire_map(), slot_of_wire[].
- K, M_post, b(c) | postselected qubit set; mask; class index c << S | spec l297
  | |K| = number of postselected qubits (errata E-10 l136); K = Kraus operator
  (limits A1); K = branch count (addendum 14.5); K = classes.
- p[k], p_K[k], eps | readout probabilities; K-class readout with eps = 1e-9 |
  spec (1.6)-(1.7) l331-334.
- Fid, rho, F | fidelity |<sigma_1|sigma_2>|^2/(Z_1 Z_2); reduced density
  matrix; Uhlmann F (squared convention) | spec 1.5 l335 | limits LIM-10 l356:
  F_root vs F_sq; spec 1.5 and addendum 14.6 are both F_sq.
- v_w, d_w, rho_w | vector-model word; dimension; density-matrix word | spec 1.6
  l343.
- i0(j,k), i1, t, i00, idx[m], compact(i), i11 | index helpers | spec (2.2) l362,
  (2.3) l391, (2.4) l393, (2.6) l426.
- W_live | qubits online in the early-cup algorithm | spec l455.
- ca, cb, k, fa, fb, ia, ib, ci | matmul-form ranks and indices | spec (2.8) l476.
- PROGRAM, ONLINE/G1/G2/TRANSPOSED/CUP_BELL/POST00/POST0/CONTRACT/READOUT,
  kind, slot, n_angles, bucket, B | compiled per-bucket program | spec 2.6
  l486-509 | extended by G2F/G1F (addendum l1221) and ctrl field (limits l895).
- Layout A / B / B-blocked / C | CUDA layouts | spec 3.1 l535 | E-13: only C
  uses shared memory.
- VTCM, HVX V0..V31, Q0..Q3, vdelta, vror, vmux, vasr, vmpy, vdmpy, vmpye/vmpyo,
  vgather, Rt.h, DV | Hexagon resources and ops | spec 3.2 l633-704; errata E-9.
- Cs[l], Co[l] | lane-dependent butterfly coefficients | spec l696.
- SENTENCE-PER-LANE / AMPLITUDE-PER-LANE, WS | HVX layouts; working-set vector
  registers (WS <= 11 per E-9) | spec l709; addendum a721.
- Psi_mat, U_m | state as 2^{W-m} x 2^m matrix; fused m-qubit block | spec l724.
- u, u16, u32 | unit roundoff | spec C9 l82 | u = noun-contracted MPO
  intermediate in addendum (12.14); u = M^H tau in limits (G.6).
- gamma_n, c (per-gate constant) | Higham constant; error constant (4.2) |
  spec l775-779 | c = SPSA step (spec 5.4, errata E-2); c = class index.
- Q1.15, Q2.14, Q3.29, Q1.31, Q2.30, Q3.61 | fixed-point formats | spec 4.3 l801.
- g_Q15(W), g_typ(W), g_Q31(W), g | per-gate fixed-point errors | spec (4.6) l815.
- oct, r, x, T[j] | octant reduction; sine table | spec l832 | r = MPO rank
  (addendum (12.13)); r = Jacobi rank guard count (addendum (14.10)).
- N_c, D, P_c | class weight |sigma[c]|^2; denominator sum_c N_c = p_surv;
  probability | spec (5.1) l966; C9 l81 | D_V = 2^{Q_V} verb Hilbert dimension
  (errata E-1 l15, review l57); D_s = 2^{q_s} batch matrix width (limits LIM-13
  l526); depth D = cup-forest nesting (addendum l186); word D = discourse word
  s^r s s^l (addendum (12.17) l825).
- p_min, E, A, Ng_max, D_max | flag threshold; output error; amplification;
  gate budget | spec l863-871; corrected by errata E-11.
- O_c, Pi_K, y_c, g_c, eps_D | class projector; label; dL/dP_c; guard 1e-7 |
  spec l964-968.
- d1, d2 | four-term shift coefficients 0.4267767, -0.0732233 | spec (5.4) l995.
- psi_k, lambda_k, O_eff, gbar, w_c, grad_local[k], sign_k | adjoint objects;
  sign_k = -1 for a transposed Ry (circuit form), +1 in the tree form | spec
  l1011-1029; addendum (11.30) l536-538.
- A, B, C (adjoint vectors) | psi, lambda, scratch | spec l1021 | A, B, C = MPO
  cores (addendum (12.13)); A = Sinkhorn matrix (limits Theme B).
- Delta_i, ghat, a_k, c_k | SPSA | spec l1046; errata E-2.
- word_param_base[j], param_offset[w], P_total, V | parameter indexing | spec
  l1056 | V also = verb tensor; V = vocabulary.
- m, v, b1, b2, lr, t (Adam) | (5.9) | spec l1065.
- magic, version, flags, type_sig, n_qubits | export fields | spec l1114.
- Alice, Bob, loves, sleeps, dreams | worked-example words: (0.3, 0.7, 1.1),
  (0.5, -0.4, 0.9), (0.8, -0.6), 0.8, -0.6 | spec l1191; addendum l1820-1824.
- E[Z], sd(Z), Beta(d_s, D_V - d_s) | Haar statistics of Z | errata E-1 l14-16.
- e, L_k, eta_k | error norm ||delta||; per-step operator-norm gain; additive
  error | errata E-11 l151, l163 | L = layers (C10); L_sat (addendum (12.3));
  L = loss.
- RNE, ulp | round-to-nearest-even; unit in last place | errata E-7 l90-91.
- REGISTER/VTCM/SMEM/HBM | tiers | addendum C14 l18; limits l13.
- struct Cup, struct Node, tree[], node op, R[v], OUT/SUM, cw(x), wrapper/chain
  | Sec. 11 objects | addendum l39-50, l321, l334, l394, l257.
- delta, mu, Delta, eps, eta (Frobenius) | spider, merge, copy, counit, unit |
  addendum l192-197 | eps also readout guard (spec (1.7)); eps^2 = MPS
  discarded weight (addendum (14.12)).
- who, whom, and_n, and_vp, and_s | spider tensors | addendum (11.10)-(11.13').
- lambda_x, out(j), f_k(j), M_train | Wirtinger cotangent; field maps; tree
  training memory | addendum (11.27)-(11.31) l510-545; limits LIM-15.
- L_sat, r_w, omega, V_s, WS_full, WS_inplace | capacity objects | addendum
  l585-676.
- A, B, C cores, r, q_r, u[r1], w[r2] | MPO verb | addendum (12.13)-(12.15).
- stride[j], v_j, d[j], s[j] | mixed radix | addendum (12.16') l807; limits
  LIM-19 l1132.
- f_c, W_c, b_c, z, theta, Cc, g(.), W_t, G_w, x_w, E, h, mix32, c_w, h_t |
  hybrid generator | addendum Sec. 13 l885-983.
- Psi_0, G_k, G_F, G_V, v, e[s], G2F/G1F, Ng_text | DisCoCirc | addendum
  l1176-1250.
- A_i[alpha][s][beta], chi_i, Theta, M, S_k, r, S_tol, eps^2, log Z_text,
  S_cut, I(a:b), chi_exact, G_i, S_i, struct Mention, rho_q, rho_qq', L/X/Y |
  MPS and readout | addendum l1272-1415.
- A_w, k_hyp, B1/B2/B3 | audit matrix; hyponymy score; baselines | addendum
  l1502, l1630, l1637.
- cL/sL/aL, c<n>/s<n>/a<n> | line prefixes | review l8-9; errata l9.
- RVnn, S1-S17, CP-n, R0-R9 | companion register, sources, inventory-assigned
  rule ids, roadmap stages | companion l948, l1048, l863.
- Tier A/B/C, A1..A8, B1..B20, C1..C27 (candidates), hw vote, four primitives
  | limits classification | limits l29-37, l1477, l160.
- rho[a][a'], vec(rho)[a + d a'], Wgt, T, dec, neg, K, q_e, F_root/F_sq, P
  (d x E), F (Frechet), F_eff, G_ab, F_ab, Td_ab, ins(), p[x], z[S], Lambda,
  frame F, superoperator S, slot_of_wire[], psi_v, D_k, J_c[j], F_FS, F_cl, H,
  dim g, Ry_pq, P_q, xi, NOUN/VERB tables, sig_k, Z_k, q_k, Sw, ov | limits
  objects | limits l183-1387 (see limits definitions list).

Symbols that differ across documents (summary; details Section 2.2 rows
16-20, 26): D, S, N, d/d_n, F/F_root, m, wire_map, T, K, L, A/B/C, u, c, r, k.



Registered in 1.2: puremath carries its own global notation table (its Sec.
0.2) and restates C1-C17 in its Sec. 0.1. Collisions are resolved by R10 as
above; notably puremath writes d_V for the verb Hilbert dimension (= D_V here,
errata E-1's D), d for d_n, and reserves bare D for the C9 denominator and bare
u for a unit roundoff. dsdm introduces no symbols; its ids (PRL rows FND-, REF-,
TST-, ERR-, EXP-, P0-, CUD-, QC-, DEP-, GRM-, CAP-, DSC-, NFR-, WNT-; gates
G0-G7 and falsifiers F1-F4; benefits BN-; risks UV-, PM-, PR-) are
plan-scoped and must be cited with the prefix "dsdm".


## 5. Equation index

Grouped by topic; label | location | one phrase.

Gates and indices (spec Sec. 0, 2):
  (0.1)-(0.4) l34-37 Rx, Ry, Rz, H | (0.5) l52 CNOT by bit test | (0.6) l53 CZ |
  (0.7) l54 CRz symmetric phases | (0.8) l55 CRx | (0.9) l65 field offsets |
  (0.10) l117 Paulis | (0.11) l131 dense 4x4 forms both orientations | (0.12)
  l134 lift_lo/lift_hi | (2.1) l361 butterfly | (2.2) l362 pair enumeration
  i0(j,k) | (2.3) l391 quad enumeration | (2.4) l393 compact(i) | (2.5) l418
  postselect 00 and drop | (2.6) l426 Bell cup gather | (2.5') l435 single-qubit
  postselect | (2.7) l455 Kronecker-in high bits | (2.8) l479 contraction as
  matmul | (3.1) l550 Layout B warp butterfly.
Grammar and model (spec Sec. 1; addendum Sec. 11):
  (1.1) spec l152 pregroup rewrite | (1.2) l200 word state | (1.3) l206
  monolithic contraction | (1.4) l215 Z, sigma_hat, P_post | (1.5) l221 N TV N
  form, no conjugation | (1.6) l331 probabilities | (1.7) l332 K-class readout
  and CE loss | (11.1)-(11.3) addendum l84-86 adjoints, contraction, expansion |
  (11.4)-(11.5) l97-99 chart recurrence and accept | (11.6)-(11.9) l194-197
  spider delta, mu, Delta, eps/eta | (11.10)-(11.13') l208-213 who, whom, and_n,
  and_vp, and_s | (11.12') l247 sum semantics | (11.14)-(11.17') l219-224
  relative clause, coordination formulas | (11.14') l376 THAT orientation |
  (11.18') l271 peak recurrence (no plain 11.18 exists) | (11.18'') l388 cycle
  chain bound | (11.19) l274 macs | (11.20) l281 crude bound | (11.21) l395
  streaming cut width | (11.22) l412 circuit 2-colouring | (11.23) l416 W_hw |
  (11.23') l434, (11.23'') l438 training width and growth law | (11.25) l473
  parse superposition | (11.26) l474 parse mixture (no 11.24 exists) |
  (11.27) l511 Wirtinger cotangent | (11.27') l521 root cotangent = (5.8) |
  (11.28) l522 word cotangent | (11.29) l524 child cotangent | (11.30) l536
  per-word gradients, sign_k = +1 | (11.31) l545 M_train.
Numerics (spec Sec. 4; errata):
  (4.1) l767 4-term unit dot products | (4.2) l779 per-gate constants c | (4.3)
  l784 Ng-gate bound | (4.4) l785 probability perturbation | (4.5) l810-811 Q1.15
  gate-path scheme and store (>> 14) | (4.6) l815 g_Q15(W), g_typ | (4.7) l856 amplification
  -- READ WITH errata E-11 l151 (2e/(2 sqrt p - e); first order e/sqrt p) | (4.8)
  l912 tensordot range ||T|| <= 1 | (unlabelled) spec l918 TENSORDOT Q1.15
  STORE n' = sat_int16((acc + 2^14) >> 15) (Q1.15 x Q1.15 -> Q2.30; HVX vasr
  Rt8 = 15) -- distinct from the GATE-path store (4.5) l810 sat_int16((acc +
  2^13) >> 14) (Q1.15 x Q2.14 -> Q3.29; vasr Rt8 = 14, l658) | (4.9) l926
  tensordot error -- READ WITH
  E-6 l78 (pre-scale S_q15) and E-11 l155 | E-1 E[Z], Beta law, sd, covariance
  errata l14-22 | E-2 SPSA variance l32-34 | E-6 pre-scale and unscaled constant
  l78-84 | E-7 fp16 sum l89 | E-8 absorption l96 | E-9 register counts
  l108-110, coefficient-generation estimate l112-113, WS <= 11 l121-123 | E-10 saddle, projector scaling, tail bound l130-139 | E-11
  corrected (4.7), certificate, sharp forms, corrected (4.9), p_min, recurrence
  e_{k+1} <= L_k e_k + eta_k l151-163 | E-12 chart cost l180 | E-13 hazard rule
  l199 | Sec. B Bell P_post = Z 2^{-2 q_n} l208.
Learning (spec Sec. 5; addendum 13.4; limits A6):
  (5.1) l966 N_c, D, P_c, losses | (5.2) l978 two-term shift | (5.3) l984
  quotient rule | (5.4) l995 four-term shift | (5.5) l1002 CRz/CRx
  decompositions | (5.6) l1014 adjoint gradient | (5.7) l1015 recurrences |
  (5.8) l1019 O_eff and lambda seed | (5.9) l1066 Adam | (13.12) addendum l1052
  hybrid gradients | limits A6 unlabelled: Jacobian, F_FS, F_cl, Hessian, LM/QNG
  l998-1032.
Capacity and scaling (addendum Sec. 12):
  (12.1)-(12.2) l582 dof | (12.3) l592 P_w and L_sat | (12.4) l616 IQP phase
  form | (12.5) l622 realised rank | (12.6)-(12.8) l637-639 trigonometric
  polynomial, rational p_c | (12.9) l647 multilinearity | (12.10) l658 order
  sensitivity | (12.11), (12.11') l676 (label on l677), l736 working set, flops | (12.12) l744
  lexicon bytes | (12.13)-(12.16') l760-814 MPO verb, streamed contraction,
  isometric, norm constraint, odometer | (12.17)-(12.20) l827-847 two-layer
  composition, memory max, reprepare, capacity verdict.
Hybrid (addendum Sec. 13):
  (13.1), (13.1') l886-888 generator | (13.2) l894 turn codec | (13.3) l915
  bytes(f) | (13.4), (13.4') l922-933 FNV-1a features, mix32 | (13.5)-(13.7)
  l957-959 DisCoCirc updating, CPTP form | (13.8)-(13.9) l986-987 context |
  (13.10), (13.10') l997-1009 density composition, mixture | (13.11) l1024
  re-uploading.
Discourse and MPS (addendum Sec. 14; limits A4, A5):
  (14.1)-(14.2) l1176-1177 Psi_0 and text-order product | (14.3) l1185
  Frobenius bending | (14.4) l1197 unitary bending | (14.5) l1251 Ng_text |
  (14.6)-(14.7) l1275 MPS and memory | (14.8)-(14.12) l1284-1290 two-site update,
  rank guard S_tol = 1e-6, discarded weight | (14.13)-(14.14) l1317-1322 Jacobi
  SVD and mandatory sort | (14.15)-(14.17) l1360-1368 Schmidt, MI, chi_exact |
  (14.18) l1391 ambiguous reference channel | (14.19), (14.19') l1402-1415
  RDMs | (15.1) l1502 affine A_w | limits (T.1)-(T.5) l636-724 partial trace,
  ins(), WHT, marginals, seed | (HO.1)-(HO.10) l849-865 dense controlled gate,
  frames, sandwich, retained s wire, slicing, slot reuse, assertion, sampler.
Limits themes (advisory):
  (NU.1)-(NU.7) l189-294 fuzz, phaser, Mult, Schur update, sign table, Wgt and
  Kraus gradients | (SF.1)-(SF.7) l379-445 rho = P P^H, 2x2 closed form, Jacobi,
  spectral functionals, MI, Frechet cotangent | (G.1)-(G.9) l530-580 Gram,
  fidelities, InfoNCE, slot scoring, cotangents, normalisation cotangent |
  (L.1)-(L.4) l1242-1291 amplitude nouns, low-rank init, streamed tensordot,
  cross-lingual fidelity.
verify.c (inventory labels VE-1..VE-24, mapped to spec labels above; no labels
  in the file).
Companion: no labelled equations; unlabelled forms at c108 (8.76 Wh), c314 (MPS
  O(m d chi^2)), c372 (widths, 1,120 amps), c389-392 (Haar covariance, E[Z] =
  1/d_n^2), c402 (e_next <= L e + eta), c432 (SPSA), c550-560 (Line/Journal,
  residual), c828 (streaming memory), c914 (binary16 boundary).



puremath (registered 1.2): its numbered results are not duplicated here; use
its Appendix A.1 (result -> spec / addendum / errata / review / limits labels)
and A.2 (engineering label -> result proving it). dsdm has no equations.


## 6. Concept cross-reference map

Format: concept | defined in | used in | tested by | corrected by | hardware
mapping in.

- Cups (Bell effect, component-wise) | spec C8 l73-80, (1.4) l215 | spec 1.3,
  1.4, 2.3 (2.5)-(2.6); addendum 11.3 l199-203, (11.22) l413; verify.c l70-73 |
  T5, T6(d), T12, T13, T18 | errata Sec. B l208 (optional P_post = Z 2^{-2 q_n});
  none | spec 2.3 (live statevector), 3.2 (HVX), limits l1123 (expectation only).
- Spiders (Frobenius delta/mu/eps/eta; who, whom, and_*) | addendum 11.3
  (11.6)-(11.13') l190-252; C15 0xFE | addendum 11.4 spider loop l334, 11.5,
  14.1 (14.3); limits (NU.3) Mult, C1 candidate | T13, T12, T18 | -- | addendum
  l199-203 (CNOT ladder + postselect; H + postselect for eps); limits Sec. 3.
- Fused tensordot (recommended inference kernel) | spec 1.3 l225-267 | spec 4.7,
  6.2, 7; addendum 11.4 eval_tree, (12.11); limits (L.3) | T6(b), T7 (vs
  bent-wire), T12, T22 | errata E-6 (pre-scale), E-11 (bounds), E-13 (slot rule
  8 B accumulator) | spec 3.1 Layout A (thread-per-sentence), 3.2 sentence-per-
  lane, 3.4 (MANDATORY on Adreno l754).
- Bent-wire circuit form | spec 1.4 l268-328 (wire-map table l300-306) | spec
  2.6 PROGRAM, 5.3 adjoint, 8; addendum 11.5 (11.22)-(11.23''); verify.c l60-66 |
  T5, T6(c), T7 (all six shapes), T18, T24(e) | -- (scoped by addendum 11.5:
  hardware form, not the training path for Sec. 11 sentences) | spec 3.1-3.4,
  6.2; addendum 11.5 W_hw.
- Adjoint differentiation | spec 5.3 (5.6)-(5.8) l1009-1043 | spec 6.1;
  addendum 11.7 (11.30); limits A6, A1 (6), A4 (T.5), A3 (G.7)-(G.9) | T7, T24,
  limits A6-et, A4-grad, A1-T5a | review l320-321 (unfused list); errata Sec. B
  l210-211; sign_k scoping (Section 2.2 row 24) | spec 3.1 training layout 24 x
  2^W B l625; 6.1.
- Parameter shift and quotient rule | spec 5.2 (5.2)-(5.5) l972-1008 | spec 5.4
  SPSA; limits A7 (qudit generators) l1195 | T3, T7 (VT-16, VT-17 in verify.c) |
  -- | spec 6.2 (forward-only devices).
- Four-term rule for controlled rotations | spec (5.4) l995, C13 l121 | limits
  A7 l1197 | T7 (loves t0 = -0.2073400; two-term -0.0684908 wrong) | -- | --.
- E[Z] and the Z distribution | errata E-1 l13-28 (spec l218 corrected) |
  spec 4.5-4.6 floors; addendum 12.3, 19 item 7; review 2.1; limits Sec. 4
  l1563, A8 l1286 (data-dependent with pretrained init) | T-EZ, T23, T6 all-zero
  (Z = 0.25) | errata E-1 | floors: spec 6.2 as corrected by E-11.
- Q1.15 path (int16 state, Q2.14 gates, int32 accumulate) | spec 4.3 (4.5)-(4.6)
  l799-829, 4.7 l902-956 | spec 3.2 HVX integer path, 6.2; addendum 12.3
  residency; review 2.4 | T8 (+ E-6 endpoint case), T23 | errata E-6 (pre-scale
  S_q15), E-11 (numbers l159-161) | spec 3.2 l656-700 (vmpy/vasr, Rt8 = 14),
  errata E-9 (lane-varying vectors).
- Sentence-per-lane (HVX (1,1)) | spec 3.2 l709-713 | addendum 12.3 a721; review
  2.6; P0 review l152 | T9 + E-9 extension (65536-code sincos sweep, ramp) |
  errata E-9 | spec 3.2; errata E-9 l104-115 (vector multiplies, vgather v65+).
- Chart parser (recogniser (11.4)-(11.5)) | addendum 11.1 l80-140 | addendum
  11.6 interface l486-493, 15.4; errata E-12; review 2.10 | T11, E-12
  greedy-vs-chart regression | errata E-12 (a107 m^3/24; a130-131; a486/488) |
  host CPU / DSP scalar core only (C16 l52).
- Greedy shift-reduce (six shapes) | spec 1.1 l146-194 | P0 (review l149) | T5,
  T6 parse steps; E-12 counterexamples must FAIL | errata E-12 (demoted to
  pre-check) | host.
- MPO verb (tensor train) | addendum 12.5 (12.13)-(12.16') l755-822 | limits A8
  (L.2), LIM-22 cores; review l402 (export field r missing) | T16, T17 | -- |
  addendum l768 streamed contraction; limits LIM-24 verb-resident.
- DisCoCirc (nouns as wires, sentences as gates) | addendum C17 l58-70, 14.1
  l1174-1243 | addendum 13.2(a), 14.2-14.7; limits A4, A5, A1 | T20 (argument
  order only, no program); limits T-W3 (q_n = 1, ptrace/ptrace2.c, l740-750),
  T-W4 (q_n = 2, critic2/extra.c, l752-777), A5 A-H | errata E-1 consequences for postselection across texts
  (addendum l968-972) | addendum 14.1 table l1229 (residency), G2F/G1F opcodes
  l1221; limits A5 l885 register limits.
- MPS truncation (two-site update, Jacobi SVD, rank guard) | addendum 14.3
  (14.6)-(14.14) l1270-1355 | addendum 14.4-14.6; limits A6 caveat (tangent
  inexact) | T15, T21 | -- | addendum l1328 scratch 2 (d chi)^2; l1345 capped
  counts; chi <= 4 under 8 KB slice.
- Hybrid angle generator | addendum 13.1 (13.1)-(13.4') l881-949, 13.7 export
  | addendum 13.3-13.6; errata E-10 l147 (non-zero bias) | T19 | errata E-10 |
  addendum 13.5 per-token pipeline l1070-1091.
- Density matrices (word meanings, composition, readout) | spec 1.6 l341-351;
  addendum 13.2(c) (13.10)-(13.10') l993-1017, 14.6 (14.19) | limits A1, A2, A4
  | limits A1-T1..T5b, A2-T-SF1..5, A4-T-W2..W4 | LIM-10 fidelity reconciliation
  (clarification); LIM-11 packing differs from spec l347 (Section 2.2 row 20) |
  spec 1.6 l349 (32x32 fp16 HMX tile); limits A1 (4) l251-270 tier ceilings.
- Reduced density matrix / partial trace | addendum C17 l68-69 (readout =
  RDM), (14.19) l1402-1403 (d^2 complex accumulator: 32 B at q_n = 1, 128 B at
  q_n = 2), (14.19') l1415 (two wires from the MPS) | limits A4 Sec. 2.4
  (T.1)-(T.5) l636-726, (3)(c) l675-678 all-wires single pass; A2 (SF.1)
  l379-380; (HO.7) | limits T-W3 (q_n = 1, ptrace/ptrace2.c, l740-750), T-W4
  (q_n = 2, critic2/extra.c, l752-777), T-SF5 l463-467 (= T-W4 spectrum);
  addendum T20 l1870 (wire-order invariance only, no program) | -- | limits
  A4 (4) l691-705 (< 400 flops at (1,1); 1,920 FMA and 228 B for all 3 RDMs
  at (2,1), l703-705), (5) l708-720. Procedure: Section 8.9.
- Spectral functionals (entropy, Renyi, purity, Uhlmann, trace distance,
  k-hyponymy) | limits A2 Sec. 2.2 (SF.1)-(SF.7) l332-479 | addendum 15.4
  l1629 k_hyp | T-SF1..T-SF4 l452-463 (spec/spec_test.c, jacobi.c); T-SF5 =
  T-W4 l463-467 / l752-777 (critic2/extra.c: eig 0.97766824 / 0.02233176 /
  0 / 0, S = 0.15433977 bit, purity 0.95633390, I = 0.30867954 bit) | fp32
  clamp lam <= 1e-7 (limits Sec. 2.2 (8)(iii) l473-474; kernel l409); the A4
  (3)(d) l683-684 guard 1e-15 is fp64-only (Section 2.2 row 41) | limits A2
  (4) l413-425 (Jacobi sweeps 3/4/5 at d = 4/8/16; 1,152 flops/sweep at
  d = 4, row 42). Procedure: Section 8.9.
- Gram matrices, InfoNCE, slot scoring | limits A3 (G.1)-(G.9) l481-624 |
  addendum 15.3(e) retrieval | limits A3-gram, A3-slot | (G.9) scoped extension of
  (11.27') | limits l552-564.
- Higher-order boxes, slicing, slot reuse | limits A5 (HO.1)-(HO.10) l794-951 |
  addendum 14.1 (U)/(F) | limits A5 A-H | -- | limits l873-892.
- Exact second-order training (Jacobian, F_FS, Hessian, DLA) | limits A6
  l953-1099 | errata E-2 (SPSA disappears) | limits A6-et, A6-steps, A6-tied,
  A6-MC-DLA | -- | limits l1038-1050.
- Qudits / mixed radix | addendum (12.16') l807-818; limits A7 l1101-1224 |
  limits LIM-19..21 | T16; limits A7 A-C | -- | limits l1171-1187 (16x16 = one
  32x32 fp16 tile).
- Streamed lexicon (amplitude-encoded nouns) | limits A8 (L.1)-(L.4) l1226-1367
  | spec 1.6 (vector model); addendum 12.4 | limits A8 tests | E-1 scoped (Z
  data-dependent) | limits LIM-24 l1308.
- Parse structure as object (plausibility, soft forest, switch, Sinkhorn) |
  limits Theme B l1369-1466 | addendum (11.25)-(11.26) | limits B-parse,
  B-switch, B-sinkhorn | -- (tier B: not hardware) | limits l1417-1428.
- Postselection amplification, p_min, D floors | spec 4.5 (4.7) l844-868, 4.6,
  6.2 | addendum 12.3 a709, 13.2(a) l968; limits LIM-9 | T8, T23 | errata E-11
  (all numbers), E-10 (|K| >= 6 default fp32) | spec 6.2 l1100-1110.
- Number-format decision table | spec 4.6 l869-901 | addendum 14.2 clause table
  l1257-1263 | T8 | errata E-11 (right-hand columns ~4x, not recomputed) | --.
- cos/sin generation from k | spec 4.4 l830-843 | errata E-9 l112 (Q1.31 lanes
  via vmpye/vmpyo) | T9 + E-9 sweep | -- | spec 3.2; CUDA sincospif (C5).
- Export format | spec 6.3 l1111-1140; addendum C15, 13.7 | errata E-3, E-5;
  companion 9.3; review l402 | T10, T19, E-5 offset test | errata E-5 | loader
  on device: addendum l1158-1162.
- Memory budget / "almost zero memory" | spec 7 l1143-1180 | addendum 16, 17;
  companion 7.1-7.3; review 2.3, 2.7, 5(d) | T22 | errata E-3, E-4, E-5, E-13 |
  addendum 16 tier column.
- Initialisation, barren plateaus, saddle | spec 5.5 l1072-1074 | addendum 13.1
  | E-10 tests (saddle -0.25, E[D] = 2^{-|K|}, dN_0/dtheta = 0) | errata E-10 |
  --.
- SPSA | spec 5.4 l1044-1053 | companion 8.2; review 2.2 | E-2 variance test |
  errata E-2 | spec 6.2 (on-device fine-tuning).
- Evaluation plan / P0 | addendum 15.4 l1612-1649; review 7 l547-576 | companion
  8.4, 13 | P0 predeclared outcomes (review l561-563) | -- | P0 residency
  (review l152-153).
- Worked example "Alice loves Bob" | spec 8 l1182-1237; verify.c | every
  document (Section 2.2 row 30; overlaps) | T4-T7, T24(a), limits A2/A3/A4/A6 |
  errata E-10 l146 (two exact-zero derivatives) | spec l1235 memory (112 B fp32).


## 7. Test registry

Tolerances (spec l1258; addendum l1820): fp32 1e-6, fp16-store 3e-3, Q1.15
2e-3 against the fp64 reference, unless a test states its own. Reference
programs are ground truth for what they print; none asserts except where
noted; run them and diff.

| id | what | expected | reference program | source |
|---|---|---|---|---|
| T1 | index helpers at W = 6, all lo < hi | i0 bijective onto {bit_k = 0}; i00(j) >= j; compact(i00(j)) == j | verify.c l99-101 (prints FAIL on error) | spec l1259 |
| T2 | apply_1q/apply_2q vs dense reference M (C13) at W = 6, both control orientations, lift_lo/lift_hi | match M psi | NONE on disk (gap) | spec l1260 |
| T3 | gate identities | CRz decomposition (5.5) residual < 1e-15 (verify.c: 1.68e-16); CRx = Rx CZ Rx CZ; CNOT sandwich = I (negative); U^T by reversed list == transposed dense for IQP and Sim15 | verify.c l91-94 (CRz only; CRx NOT coded despite comment) | spec l1263 |
| T4 | word states | Alice [0.71847188-0.31582980i, -0.13353070-0.60516052i]; Bob [0.74959627+0.19470917i, 0.03946950-0.63137622i]; loves |V[k]| = 0.35355339 with phases 0, -0.4, +0.3, +0.7, 0, -0.4, -0.3, +0.1 | verify.c l47-50 | spec l1265 |
| T5 | W = 2 "Alice sleeps" | IV = [0.5, 0.46053050-0.19470917i, 0.5, 0.46053050+0.19470917i]; sigma = [0.17991068-0.41061012i, 0.41557129-0.46260942i]; Z = 0.58767549; p = [0.34197193, 0.65802807]; zero angles: sigma = [0.5, 0.5], Z = 0.5 | verify.c l51-54 | spec l1266 |
| T6 | W = 3 "Alice loves Bob" by (a) (1.5), (b) tensordot, (c) bent-wire, (d) monolithic W = 5 | sigma = [-0.02640234-0.28465252i, 0.08196341-0.22764750i]; Z = 0.14026553; p = [0.58263882, 0.41736118]; Loss(y=0) 0.54018780, Loss(y=1) 0.87380330; (c) psi[0], psi[2] = sigma to 1e-16, D = Z; (d) psi = sigma/2, P_post = Z/4 = 0.03506638; zero angles Z = 0.25 | verify.c l55-73 | spec l1270 |
| T7 | gradients and wire maps | dp_0/dtheta = [0.0701761, 0.0457067, -0.0415143, 0.1819065, -0.1080537, 0.0905236, -0.2073400, -0.5529685]; dLoss(y=0)/dtheta = [-0.1204452, -0.0784477, 0.0712521, -0.3122115, 0.1854557, -0.1553682, 0.3558636, 0.9490759]; two-term + quotient on Alice t0 = 0.0701761 (N_0' 0.0549342, D' 0.0773908; naive shift -0.0151382 wrong); four-term loves t0 = -0.2073400 (N_0' -0.0484303; two-term -0.0684908 wrong); p_0(loves t0 + 2 pi) = 0.87639280, + 4 pi = 0.58263882; adjoint == FD to 1e-6; Sim15 effect word sign_k = -1; six wire-map rows == tensordot to 1e-12 | verify.c l74-98 (FD, shifts, periodicity); bp/bp.c, adj_dz/adj_dz.c (adjoint, sign_k, shared noun) | spec l1273 |
| T8 | numerics | ||phi_w|| = 1 +- 2 c Ng u; Q15 rounds half-up and saturates on acc = 2^29 sqrt2 - 1 and -2^29; fp16 stall (E-7 form: sum(2048) == sum(2049) == sum(4096) == 1.0; 2^-12 series 0.5); Q15 vs fp32 sentence error <= g_Q15(W) Ng; P_c error <= 2e/(2 sqrt D - e) (E-11); E-6 endpoint case: narrowing a component in (1 - 2^-16, 1] with pre-scale stays within the 2^-15 word bound (endpoint-case narrowing error 1.53e-5 <= 2^-15 with pre-scale vs 3.41e-5 without); worked example |delta p0| 3.11e-5 unscaled / 3.47e-5 pre-scaled against the 2e-3 tolerance -- verQ15/q15_tensordot.c; Q1.15 stores: gate path (4.5) l810 >> 14, tensordot path spec l918 >> 15 | fp16_absorb.c, fp16_c12.c, verQ15/q15_tensordot.c, q15_sat.c, second/num.c | spec l1280; errata l86, l91, l163 |
| T9 | HVX bring-up | vmpy/vasr ramp round trip is identity; vdelta xor controls k = 0..5 vs lane-id vector; fl(a*b) rounding and subnormals recorded; E-9 extension: vector-form coefficient ramp and 65536-code sincos sweep within Q2.14 budget (review l195: 3.074e-5 vs 3.052e-5 + 3.1e-7; tolerance UNVERIFIED, not fixed by any document) | none (needs target) | spec l1283; errata l121 |
| T10 | export round trip | magic; stride 20; type_sig n^r s n^l == 08 01 04 FF FF FF; decode error <= 4.8e-5; decode(4172) = 0.399985 +- 1e-6; 0.8 + 2 pi -> different k; E-5: computed per-type-class offsets reproduce param_offset | verify_export_bytes.c (sizes only) | spec l1285; errata l74 |
| T-EZ | mean Z over 1e5 Haar triples | 0.250 +- 0.002 at (1,1); 0.0625 +- 0.0005 at (2,1) | verEZ/ez.c, second/ez_mc.c | errata l27 |
| E-2 test | SPSA variance on a linear surrogate as c varies | flat in c (0.466300 at P = 8, c = 0.01/0.1/1) | spsa_var.c | errata l39; review l96 |
| E-10 tests | saddle and init | d2P_0/dt0 dt6 at theta = 0 == -0.25; E[D] at uniform init == 2^{-|K|} (0.2500 at |K| = 2); dN_0/dtheta == 0 exactly for Bob t2 and loves t1; log clip rate (~52-54 %) and D histogram (~1.1 % below 2^-6) | bp/bp.c, bp2.c, second/bp_mc.c, exact_train/et.c (J[0][7] = 0) | errata l137, l146-147 |
| E-12 test | greedy vs chart | "Alice loves man in park" and "Alice loves the man in park": greedy FAIL, chart unique parse (0,1)(4,5)(3,6)(7,8) s at 2; "Alice whom Bob loves sleeps" passes with n^r n n^ll s^l, fails with n^r n s^l n; chart count == (m-1)(m+1)(m+3)/24 for m = 5..197 | second/parse.c, parseK/parsechk.c | errata l186-188 |
| T11 | chart parser | each 11.2 row: listed cups, survivor, 1 parse; "Alice sleeps and dreams" -> (2,3)(1,4)(0,5)(8,9)(7,10), s at 6, greedy FAILs; two-parse string kth(0) != kth(1); accept_prefix on "Alice loves Bob the"; 0 false accepts/rejects on 1e5 strings | verE/parse.c, refE/chkE.c | addendum l1826 |
| T12 | relative clause (1,1) | t1 = [0.27897688-0.15438512i, 0.19683432-0.25083682i, 0.18020560-0.07329415i, 0.19452243+0.00266698i]; t2 = [0.25800180-0.30860466i, -0.20244073-0.20369536i]; sigma = [-0.00389059-0.20869319i, 0.07543213-0.28752732i]; Z = 0.13192995; p = [0.33023573, 0.66976427]; Loss(y=0) = 1.10794854; brute force 7.9e-17; circuit amplitudes sigma x 2^{-1/2}, p_surv = Z/2 = 0.06596498; zero angles sigma = [0.35355339, 0.35355339], Z = 0.25; whom: sigma = [-0.00216678-0.20792288i, 0.09629179-0.32243941i], Z = 0.15647590, p = [0.27631489, 0.72368511]; peak 14 (10 slot rule) | addendum/t18.c | addendum l1830-1839 (subject a1830-1836, whom a1837-1838, peak assertion a1839); (1,1) ONLY -- no (2,1) sigma/Z vector exists for any relative clause (G10) |
| T13 | spider q = 1 | mu(Alice, Bob) = [0.60005880-0.09685177i, -0.38735435+0.06042272i]; eps(Alice) = 0.58494118-0.92099031i; eps(Bob) = 0.78906577-0.43666705i; "Alice and Bob sleep": sigma = [0.13340576+0.05482207i, 0.10987605-0.09602082i], Z = 0.04209530, p = [0.49417764, 0.50582236]; "Alice sleeps and dreams": sigma = [0.13129826-0.22615905i, 0.16150588-0.23282446i], Z = 0.14867853, p = [0.45996654, 0.54003346]; CNOT + postselect = mu exactly; H + postselect = eps x 2^{-1/2} | addendum/t18.c | addendum l1840 |
| T14 | THAT orientation (11.14') | t_that[c] = sum_c' phi_that[c + 2c'] t_sleeps[c'] to 1e-15; transposed order differs > 0.1 | addendum/t18.c | addendum l1846 |
| T15 | 4x4 Jacobi SVD | M = I + 2P: S = [3, 2.2360679775, 2.2360679775, 1], 5 sweeps, residuals <= 2e-15 / 1e-15, ||M||_F^2 = 20, chi = 2 keeps 14 discards 6; J_4(2): S = [2.8512117716, 2.4329222531, 1.8371875468, 1.2554770653], prod 16, sum sq 19; sort ran; rank guard S = [1,0,0,0] -> chi_i = 1, no NaN | derivH/jsvd.c, verH/jsort.c, verH/mps_nan.c | addendum l1848 |
| T16 | mixed radix d = 3 | (3,3,3): (2,1,0) -> 5; (1,2,2) -> 25; (0,0,2) -> 18; 17 -> (2,2,1); (3,2,3): 13 -> (1,0,2); (2,1,2) -> 17; odometer 0..17 in order | -- (spec pseudocode) | addendum l1857 |
| T17 | MPO verb integer case | V = [21, 0, 36, 3, 1, -4, 6, -3]; u = (7,0), w = (6,2), sigma = (70, 126) dense and streamed; B read once | -- | addendum l1860 |
| T18 | circuit colouring (11.22) | "Alice who loves Bob sleeps": states {loves, sleeps}, effects {Alice, Bob}, W_hw = 3; "Alice and Bob sleep": W_hw = 3; triangle: exactly one Bell cup | addendum/t18b.c | addendum l1864 |
| T19 | codec, mix32, FNV | -1.0 -> 60321; 0.8 -> 4172; 13.0 -> 2261; NaN -> 0; integer path acc = 1000 -> 4172 +- 1; mix32(0) = 0x92CA2F0E, mix32(1) = 0x96A0F96B, mix32(0x12345678) = 0x047660EA; FNV-1a("<the>") = 0xFCF0F1C0; |G_w| for ell = 1,2,3,7 = 1,3,6,22 | addendum/mix.c | addendum l1867 |
| T20 | DisCoCirc argument order (C17) | wires (0,1) and (1,0) give identical rho_Alice to 1e-12; U_big before U_loves; real Sim15 verb FAILs controlled-phase reproduction, Sim14 passes | -- | addendum l1870 |
| T21 | MPS vs exact | chi = 8, N = 6, 30 U(4) gates: ||Psi_mps - Psi_exact||^2 <= 1e-10 (4.5e-12); ./mpstest 2 11: error == discarded weight 1.379e-2, norm^2 0.986212 | derivH/mpstest.c | addendum l1874 |
| T22 | scheduler memory vs (11.18') | two relative clauses 16 / 48 (12 / 40 in place); believes sentence 14 / 44; streaming 20 / 80 | -- | addendum l1876 |
| T23 | precision (12.3) | mean Z 0.25, 0.0625, 0.0039 +- 10 % over 400 triples; Q1.15 (4,2) vs fp32 <= 2.5e-2 in p (loose ceiling; E-11 tight value 1.25e-2 UNVERIFIED); p_min flag fires when Z < 1.6e-3 (predates E-11; UNVERIFIED) | derivF/zmc.c | addendum l1878 |
| T24 | tree adjoint | (a) 8 gradients == Sec. 8 dLoss to 1e-7 (4.8e-11 fp64); (b) T12 sentence dL/dtheta = [-0.15106071, -0.14060757, 0.17728899 | 0.04493042, -0.01820026, 0.02841004 | 0.03290603, -0.10412078 | 0.61601074], M_train = 50; (c) [-0.03859959, -0.02433568, 0.02115842, 0.43442175, 0.43442175]; (d) shared-angle and_s: [-0.02308977, 0.00129679, -0.02036071, -0.08223878, 0.01134526, -0.06765975, 1.06704471, 1.84830706], sigma = [-0.07322709+0.01232450i, -0.05891819-0.04440411i], L = 0.68668626, omitting scatter-add FAILs; (e) circuit adjoint agrees 1e-7; W_train = 5, 768 B vs 400 B | critic/adjtree.c | addendum l1880 (exists although the programme summary says T11-T23) |

Limits per-theme tests (advisory kernels; cite as "limits <id>"; all fp64 in
the listed programs; pass = fp32 within 1e-6 of every printed value, fp16
lexicon tables 2e-3):
  A1-T1..T5b (nu.c, l300-317): fuzz/phaser/BrhoB/Proj/Mult/neg/merge on
  |+><+| with sigma = diag(1, 1/4) -> fuzz diag(0.5, 0.125), T = 0.625; phaser
  [[0.5,0.25],[0.25,0.125]]; Bell two-wire <00|rho|11> = 0.4 (phaser) vs 0
  (fuzz); Stinespring damping [[0.68,0.4],[0.4,0.32]]; adjoint P = 0.8520079519,
  dP/dtheta = (-0.0678584686, -0.0428192659, -0.1541439754) to 7e-11; 16 Kraus
  derivatives to 1.1e-10.
  A2-T-SF1..5 (spec/spec_test.c, jacobi.c, critic2/extra.c, l452-467; T-SF5
  = the q_n = 2 case, l463-467, values from T-W4): Sec. 8
  rho_s = [[0.5, 0.40523146-0.17132911i],[c.c., 0.5]], lam 0.93996159 /
  0.06003841, S = 0.22707320 nat, purity 0.88713240, F_sq 0.93359620, T
  0.09984988; two-wire d = 2 F_root 0.97417081, k_max 0.31988774; A = |0><0|,
  B = I/2 analytic; Jacobi sweeps 3/4/5; T-W4 eig (0.97766824, 0.02233176, 0, 0),
  S = 0.15433977 bit, purity 0.95633390, I = 0.30867954 bit.
  A3-gram, A3-slot (gram/gram.c, l591-611): Z_0 = 0.14026553, Z_1 = 0.16045040,
  Z_2 = 0.26537358; G_01 = 0.98618261-0.01327170i; F_01 = 0.97273229, F_02 =
  0.79843230, F_12 = 0.77399490; InfoNCE L = 0.53396114; slot fid(Bob) = F_10.
  A4-T-W2/W3/W4/Sec8, A4-grad (l737-780; T-W2/T-W3 and T-Sec8 from
  ptrace/ptrace2.c, T-W4 (q_n = 2, l752-777) from critic2/extra.c): Bell
  S_0 = 1 bit; W = 3 p[x], WHT, S_0 = S_1 = 0.22649829, I(0:1) = 0.45299658;
  dS_0/dtheta_CRz = 0.42460500; Sec. 8 p(q0=q2=0) = 0.14026553.
  A5 Tests A-H (discocirc_ho/ho2.c, l912-937): PsiA[0..7]; controlled frame
  p(Claire = 1) = 0.221700; sandwich 7.9e-17; slice = trace 1.1e-16; slot reuse
  2.8e-17; hedge Z = 0.526433; assertion p = 0.5; H = T-W4 both paths.
  A6-et, A6-steps, A6-tied, A6-MC-DLA (exact_train/et.c, l1067-1080): J[0][7]
  = 0 exactly; F_cl eigenvalue 1.687955; H eigenvalues -0.906866 ... 1.683910;
  GD 0.420273, QNG 0.401693, GN 0.020774, LM mu = 3.0 0.124290; tied L =
  0.6930406; E[D] = 0.2500; DLA dim 13 of 63.
  A7 A/B/C (wirerep/qd.c, l1202-1215): d_n = 3 tree Z = 0.15403278, p =
  [0.76853091, 0.23146909], Loss 0.26327449; real field Z = 0.01416068; U(3)
  unitarity 6.8e-16; qudit shift gradient -0.00337194.
  A8 (1,1), rank1, (2,1) (lexicon/lex_test.c, fid_test.c, l1338-1354): sigma =
  (0.71773045, 0.47166541), Z = 0.73760526; F = 0.80357345, L_F = 0.19642655;
  rank-1 Z = 1, P = (0.8, 0.2); (2,1) Z = 0.80203295, F = 0.91910731.
  B-parse, B-switch, B-sinkhorn (parsestruct/ps.c, l1450-1459): parse 0 Z =
  0.07915003, parse 1 Z = 0.05651013; q = [0.58344344, 0.41655656]; p(+) =
  0.66580695; Sinkhorn Z = 0.13269939, L = 0.64209732.

Review-only checks (P0 and derivations; review Sec. 9 l593-603): errbound
_check.c max ratio 0.2500; lipschitz_check.c L_k 0.54-0.85; verLane 0 of 8
lane-uniform; scope/exp0.c P0 sizes (1,190,592 sentences; Model A 176 angles /
352 B, peak 16 amps; Model B 320 angles, 52 amps); P0 predeclared outcomes IID
>= 97 %, role-swap >= 90 %, triple-holdout >= 90 %, working set <= 128 / 416 B.

Coverage note: no assertion-based one-command harness exists (companion c897;
gap G10). T2, T16, T17, T20, T22 have no program on disk.



Registered in 1.2: puremath adds no test to this registry (Section 12 step 3:
none of its statements is reproduced by a new program in D/). dsdm defines
plan-level acceptance checks (CI-NOPY, CI-ASCII, CI-HASH, P0-SIZE ... P0-ALL,
PTXAS, HVX-REG, ADAPT-CONF, TRAIN-EQ, REPRO, device variants of T4/T6/T7/T24;
dsdm Sec. 7.1) and assigns every registry test above to a requirement and a
timebox (dsdm Sec. 9 coverage table); those checks are sequencing, not new
expected values.


## 8. Task procedures

Each procedure: read, apply, run, update.

### 8.1 Implement a kernel (butterfly, 2q gate, postselect, tensordot, node op)

1. Read spec Sec. 0 whole (C1-C13) with errata E-8; spec 2.1-2.6 for the
   kernel family; spec 1.3 if it is the tensordot (with errata E-6, E-13); spec
   2.3 for cups on a live statevector; addendum C16 + 11.4 pseudocode if it is a
   node op; addendum C17 + 14.1 (G2F/G1F) if it is a DisCoCirc gate.
   If the kernel targets a device or a non-fp32 format, ALSO apply 8.3
   (format) and 8.7 (device) -- in particular errata E-9 (HVX register
   budget, lane-varying coefficients), E-11 (bounds, single D floor 2^-6)
   and spec 3.2, 4.3, 4.7 (tensordot store l918). Worked composite route:
   8.1a.
2. Apply: C1 bit order, C3 split planar, C4 gate-list order, C6 bit-test
   controlled gates (never permuted matrices), C7 field layout, C8 cups; the
   slot rule as tightened by E-13 (one 8 B accumulator); flip masks only for X
   (spec 2.4 l470: CNOT is NOT a flip mask); remove postselected qubits in
   DESCENDING order (spec l437); tag each gate with its form (R11).
3. Run: T1 (verify.c), T2 (write it: dense M via mask test, both orientations,
   W = 6 -- no program exists), T3, T5, T6 (a)-(d), T12/T13/T22 for node ops,
   T20 / limits T-W4 for DisCoCirc gates; compare with fp64 to 1e-6 (fp32).
4. Update: nothing in the documents; record register/spill counts (ptxas -v)
   as a target measurement in the implementation notes (C14).

### 8.1a Worked route: HVX sentence-per-lane Q1.15 kernel for the (1,1) fused tensordot

(Composite of 8.1 + 8.3 + 8.7; use it as the template for any device kernel
in a non-fp32 format.)
1. Read: spec 1.3 l225-267 (tensordot) with errata E-6 l77-87 (pre-scale)
   and E-13 l194-204 (one 8 B accumulator); spec 4.3 l799-829 and 4.7
   l902-956 with errata E-11 l149-165; spec 3.2 l633-720 with l709-713
   REPLACED by errata E-9 l100-124; addendum a721 read as WS <= 11 (E-9
   l121-123); review 2.4 (l131-154) and 2.6 (l170-200) for the derivations.
2. Apply:
   - Register budget (authoritative = errata E-9 l108-110, target per C14):
     tensordot 20 + 4 accumulators = 24; CRz 27; Rx butterfly 28-29 (31 if
     both int32 outputs held) of 32; (1,1) WS = 10 <= 11. Coefficients are
     lane-varying Q2.14 VECTORS (3 registers per rotation gate), never scalar
     Rt.h (which spec l709-713 assumed); vector x vector vmpy is v60,
     scalar-accumulate V6_vmpyh_acc v65, vgather v65+.
   - Coefficient generation ~36 DV per 64 angles per gate (errata l112-113,
     ESTIMATE, UNVERIFIED); cheaper: exported cos/sin pairs (flags.bit0 = 1)
     gathered per lane.
   - Words pre-scaled by S_q15 = 1 - 2^-15 before the single narrowing (E-6).
   - Products Q1.15 x Q1.15 -> Q2.30, int32 accumulate (|acc| <= 2^30, spec
     l914-917); STORE n' = sat_int16((acc + 2^14) >> 15), i.e. vasr Rt8 = 15
     (spec l918). NOT the gate-path (4.5) l810 >> 14 / Rt8 = 14 of l658.
   - Format choice for the tensordot: Q1.15 over fp16-store at (1,1) and
     (2,1) on accuracy (spec l949-950 as corrected by E-11: 1.96e-4 vs
     2.4e-3; E-9: "prefer Q1.15 for the Sec. 4.3 error bound, not for
     registers"). The Sec. 4.6 W-threshold rule is for the circuit form only.
   - Readout: N_c, D in int32/fp32; single floor D >= 2^-6 with fp32
     fallback (E-11 l160-163); report D with certificate 2e/(2 sqrt D - e).
3. Run: T1, T2 (write it), T3, T5, T6(b) in Q1.15 at the 2e-3 tolerance, T8
   (incl. E-6 endpoint case; verQ15/q15_tensordot.c sweep: max ||delta
   sigma|| 1.087e-4 <= 1.96e-4, no int32 beyond 2^30), T9 + E-9 extension
   (device), T23 mean-Z part.
4. Update: record the register/spill report and measured cycles as a C14
   target measurement (Section 10 B); nothing normative changes.

### 8.2 Add a grammar construction (new word class or type string)

1. Read addendum 11.1 (chart, accept, kth), 11.2 (lexicon table), 11.3
   (spiders if the word is parameter-free), 11.4 (build_tree, cycles/chains),
   11.5 (circuit colouring if hardware form needed), 11.7 (tree adjoint);
   errata E-12 (parser rules, counterexamples); spec 1.1 only for the six-shape
   pre-check; C15 for type codes; C16 for Node limits (nchild <= 4, 6 atoms).
   Line map for relative clauses (ready-made material): types a169-172
   (subject a169-170, object a171-172); who (11.10) a208, whom (11.11) a209;
   sentence formulas (11.14)-(11.15) a219-220; contraction order and peak
   a239-244; spider node-op reductions a346-347; W_hw example a418-425;
   W_train (11.23') a433; tree-adjoint spider rules a528-532 and M_train
   (11.31) a545-548; Sec. 16 rows a1682-1683 (tensordot bytes), a1684-1685
   (circuit, circuit training), a1689 (who/whom tensors 0 B), a1701 (held
   sibling); Sec. 17 a1756-1757; tests T11 a1826, T12 a1830-1839, T13 a1840,
   T18 a1864, T22 a1876, T24(b)/(e) a1883-1886, a1894-1895.
2. Apply: express the word as a type string over {N, S, I, Q}; verify with the
   chart recogniser that every example row has exactly the listed cups and
   survivor; assign ansatz per C10 or 0xFE spider; check peak by (11.18') and
   compare with the Sec. 16 tier; if a cycle appears, mark the wrapper and
   CHAIN (11.18''); for the circuit form compute W_hw by (11.22)-(11.23) and
   W_train by (11.23'). Relative pronouns: SUBJECT = n^r n s^l n (a169; the
   unlabelled spec l176 type, gap G17), OBJECT = n^r n n^{ll} s^l (a171);
   both are C15 spiders (ansatz_id 0xFE, P_w = 0): who = delta[a,b,d] eps[c]
   in factor order [n^r, n, s^l, n], whom = delta[a,b,d] eps[c] in order
   [n^r, n, n^{ll}, s^l], evaluated by index equality, zero bytes; the node
   op is R[a] = Alice[a] sum_c R_loves[a + 2^{q_n} c] (a346). Contraction
   order (11.14): t1[a,c] = sum_b V[a,c,b] Bob[b]; t2[a] = sum_c t1[a,c];
   t2 *= Alice; sigma[s] = sum_a t2[a] IV[a,s]; peak 14 (10) at (1,1), 44
   (36) at (2,1) (a239-241; counting convention: Section 2.2 row 36).
3. Run: T11 (plus the new row), T12-T14 analogues (brute force over the product
   state), T18 colouring, T22 memory accounting, T24-style FD check of the tree
   adjoint, E-12 regression. T12 exists at (1,1) ONLY; T22 (44 at (2,1)) has
   no program (G10): for (2,1) generate the brute-force fp64 reference
   yourself (extend addendum/t18.c) and add it to addendum Sec. 18.
4. Update: addendum 11.2 table (new row with example, cups, survivor, peak),
   Sec. 16 row if a new object appears, Sec. 18 test with fp64 values; if the
   construction needs a header field, bump C15 version and record it in
   Section 12 of this bridge.

### 8.3 Change a number format (fp32 / fp16-store / Q1.15 / Q1.31)

1. Read spec Sec. 4 whole; errata E-6, E-7, E-8, E-11 (and E-9 for HVX lanes);
   spec 6.2 (floors) as corrected; addendum 12.3 (residency, precision column
   with the E-11 factor 2^{q_n}); limits Sec. 2.2 (8)(iii) l473-474 for
   entropies (fp32 clamp 1e-7; Section 8.9).
2. Apply: no scale factor beyond S_q15 = 1 - 2^-15 on the Q1.15 tensordot
   path; round-half-up saturating store: (4.5) l810 (>> 14, vasr Rt8 = 14)
   on the gate path, spec l918 (>> 15, vasr Rt8 = 15) on the tensordot
   path; accumulate probabilities in
   fp32/int32 only; never renormalise; D floor 2^-6 on Q1.15 and fp16-store
   with fp32 fallback; report D with the certificate 2e/(2 sqrt D - e); FTZ
   hazard check on the device (spec l794); Q1.15 wins W <= 9, fp16-store/fp32-
   compute W >= 10, pure fp16/bf16 never (spec 4.6 reading, circuit form only).
   Tensordot path: Q1.15 is preferred over fp16-store at (1,1) and (2,1) on
   accuracy (spec l949-950 as corrected by E-11: 1.96e-4 vs 2.4e-3; E-9:
   prefer Q1.15 for the error bound, not for registers); the Sec. 4.6
   W-threshold rule does not apply to it. For an HVX kernel follow 8.1a.
3. Run: T8 (all parts incl. E-6 endpoint and E-7 stall), T23, T9 on HVX,
   verQ15/q15_tensordot.c sweep (bound never exceeded, no int32 beyond 2^30).
4. Update: if a bound changes, append an erratum (8.8); Sec. 4.6 right-hand
   columns remain unrecomputed (Section 9) -- compute and label UNVERIFIED.

### 8.4 Export a model

1. Read spec 6.3 with errata E-3, E-5; addendum C15 and 13.7 (if I/Q types,
   spiders or a generator are present); spec C5 and 5.6 (wrap only at export,
   mod 4 pi, from unwrapped fp32 theta); T10, T19. For an NPU target also
   read spec 6.2 l1100-1110 with errata E-11 l160-163 (single floor D >=
   2^-6 on both paths; certificate 2e/(2 sqrt D - e)) and spec 4.4 l830-843
   (on-device cos/sin from k), or choose flags.bit0 = 1 (Q15 pairs, no trig
   on device, 4 B/param).
1b. Companion vs spec (which is authoritative): spec 6.3 + errata govern the
   format (R1); addendum C15 governs the type_sig / version extension
   (Section 2.2 row 27, R3); companion 9.3 c486-498 is a constraint list
   only (R4, R5); its unmet fields (MPO rank r, MPS chi, readout, grammar
   version, checksum) are gap G19 (review l402). Never change a field
   layout because the companion asks; propose a C15 version bump instead.
2. Apply: header fields of Section 3.4; flags bit0/1/2/3; per-word ansatz_id
   override 0xFF/0xFE; type_sig base codes 0-3 with adjoint code << 2, 0xFF
   terminator, <= 6 atoms; param_offset 0xFFFFFFFF = generate; k = round(theta
   /(4 pi) 65536) mod 65536 (spec l1135); Q15 pairs round(cos phi x 32767)
   (l1132); word-record byte offsets exactly as Section 3.4 (spec
   l1118-1129: @0 word_id, @4 type_sig[6], @10 n_qubits, @11 ansatz_id, @12
   P_w, @14 param_offset, @18 reserved); loader checks
   (Cc equality, T19 vectors, mix32(0)); record the chosen ring order / closing
   H in accompanying notes (no header field exists: gap G2).
3. Run: T10 round trip, E-5 offset reproduction, T19, verify_export_bytes.c
   sizes (32 + 20 V + 2 P_total or 4 P_total); for an NPU target also T6(b)
   in the device format and the D-floor rejection path.
4. Update: if new fields are needed (r, chi, readout, grammar version,
   checksum; review l402) propose the C15 version bump as an addendum change
   and log it in Section 12.

### 8.5 Train a model

1. Read spec 5.1-5.6 with errata E-2, E-10; spec 6.1; spec 5.3 for the circuit
   adjoint (six shapes, hardware form) or addendum 11.7 for the tree adjoint
   (any Sec. 11 sentence); addendum 13.4 (segmented reduction, hybrid
   gradients) if a generator is trained; review Sec. 7 for P0 protocol; limits
   A6 only for second-order methods.
2. Apply: uniform [0, 2 pi) init, fixed seed, >= 3 restarts; flat parameter
   vector in occurrence order (spec 5.5; T7 order Alice, Bob, loves); sign_k
   = -1 for transposed Ry in the circuit form, +1 in the tree; adjoint on the
   unfused gate list; (5.8) seed with eps_D = 1e-7, clip gradient norm 1; Adam
   (5.9) 16 B/param, theta unwrapped fp32; log D histogram and clip rate;
   fp32 throughout (24 x 2^W B per sentence, or M_train (11.31)); SPSA only
   when no adjoint exists (c ~ 0.08 rad, E-2 variance).
3. Run: T7 (adjoint == FD 1e-6 on the 8 parameters; Sim15 sign check), T24
   (a)-(e), E-10 tests (saddle -0.25, E[D] = 2^{-|K|}, exact zeros), T-EZ, P0
   predeclared thresholds with 5 seeds.
4. Update: P0 result record (review Sec. 7 falsifiers); addendum 15.4 item 5
   outcomes ("report all of it"); negative results (companion 16.3).

### 8.6 Answer a numeric question

1. Identify the quantity and its document of record by Section 9; check
   Section 2.2 for a conflict row; check the errata for the spec line.
2. Apply precedence R8: verify.c number if it prints it; else errata; else
   review; else addendum; else limits (own themes); else spec; companion last.
   State the form (worst bound / typical estimate / measured-once / Haar
   expectation) and the counting convention (WS_full vs in place; fused vs
   unfused Ng; first-order vs certificate).
3. Run: the reference program named in Section 7 or the review's program list
   if the number is reproducible; cite "recomputed here" otherwise.
4. Label UNVERIFIED if the number was derived from a superseded constant and
   not retabulated (Section 9 list), or if it is a hardware figure without a
   device measurement.

### 8.7 Extend to a new hardware target

1. Read spec Sec. 3 (facts, layouts, HVX/HMX/Adreno mappings) and Sec. 10
   items 2-6 (UNVERIFIED hardware facts); errata E-9, E-13; addendum C14
   (tiers), 12.3 residency table, 14.1 residency rule, Sec. 16 tier column;
   limits Sec. 4 primitive-cost table; companion CP-25 (named device/toolchain
   evidence required).
2. Apply: pick the layout (thread-per-sentence for the tensordot; warp/block
   per sentence for the circuit; sentence-per-lane (1,1) / amplitude-per-lane
   (2,1) on HVX); fused tensordot is MANDATORY on Adreno (spec l754); matrix
   units only for fused m-qubit blocks with 2^m >= 32 or vector/density models
   (spec 3.3); never accumulate probabilities on tensor cores / fp16 (spec
   l738, C12); batch by (shape, W) or tree signature; lane-varying
   coefficients on HVX (E-9); memory tiers of C14.
3. Run: T9 bring-up (ramp, vdelta controls, fl(a*b) rounding, subnormals),
   E-9 sincos sweep, T6/T12 in the device format against fp64 at the spec
   tolerances, ptxas -v or equivalent register/spill report, measured
   state bytes; record every spec Sec. 10 item the device settles.
4. Update: add a "measured on <device, toolchain>" record; move the settled
   UNVERIFIED items from Section 10 to a dated measurement note; propose
   errata for any spec Sec. 3 fact found wrong.

### 8.8 Propose a spec change (errata process)

1. Read errata header l1-9 (format: section, line, original, replacement,
   reason, consequences) and Sec. B (what needs no change); review Sec. 2 for
   the derivation standard (a plain-C fp64 program, a second independent pass).
2. Apply: the baseline file stays byte-identical (hash 7408d5b7...efe86c
   pinned at errata l1); write E-14, E-15, ... in the same format, keyed to
   sNNN (and aNNN if the addendum is affected); state whether the item is a
   correction or a clarification; state that verify.c is or is not affected;
   name the program that establishes the numbers and where it lives in D/;
   propagate consequences to every dependent line (spec, addendum, this bridge
   Sections 2.2, 7, 9, 10); never edit a governing document in place -- append.
3. Run: the new program plus every test the change touches; a second
   independent pass in a fresh folder (second/ convention).
4. Update: errata file (append), this bridge (Section 2.2 conflict row, Section
   9 value, Section 10 UNVERIFIED register), the manifest qnlp_manifest.json
   (conflicts, numeric_reference, unverified; bump version; recompute the
   errata SHA-256 in documents[]). The addendum may be edited (it is an
   extension, not frozen) but its hash must then be re-pinned in Section 1 and
   the manifest, and every aNNN citation in errata/review re-checked.

### 8.9 Add an RDM / spectral readout (DisCoCirc noun wire; entropy, purity, MI)

1. Read: addendum C17 l58-70 (wire fields, readout = RDM l68-69), (14.19)
   l1402-1403 (single wire from the exact state; d^2 complex accumulator,
   128 B at q_n = 2), (14.19') l1405-1415 (from the MPS); limits A2 Sec. 2.2
   (SF.1) l379-380, (SF.3) Jacobi, (SF.4) functionals, (4) l413-425 kernel
   shape and working set, (8)(iii) l473-474 clamp; limits A4 Sec. 2.4 (3)(c)
   l675-678 (all wires in one pass), (4) table l691-705, (5) l708-720. State
   memory at q_n = 2: addendum l1229-1236 (Section 9).
2. Apply: accumulate rho in fp32 (C12; never fp16); rho[a][a'] ket row, bra
   column (Section 2.2 row 20 for packing); divide by t = Tr rho (the state
   may be unnormalised after truncation or postselection, limits l410-411);
   d = 2 closed form, d = 4 cyclic Jacobi, 3 sweeps (A2 governs the cost,
   row 42); clamp lam <= 1e-7 in fp32 before the log (row 41); state the log
   base (bits or nats) in the output record.
3. Run: T-W4 (cc -O2 critic2/extra.c -lm; fp32 kernel within 1e-6 of eig
   0.97766824 / 0.02233176 / 0 / 0, S = 0.15433977 bit, purity 0.95633390,
   I(Alice:Bob) = 0.30867954 bit; dense 16x16 verb path == gate-by-gate path
   to 1e-6; limits l752-777); T-W3 at q_n = 1 (ptrace/ptrace2.c, l740-750);
   T-SF1..T-SF4 (spec/spec_test.c, jacobi.c; l452-463); T20 (write it: no
   program exists).
4. Update: nothing normative (limits is advisory, R6); record the kernel's
   working set against limits l419-421 (<= 400 B per wire) as a C14 target.


## 9. Numeric quick-reference (current authoritative values)

Each value with its governing source line. "UNV" = UNVERIFIED (recomputed
here from a governing formula, tabulated in no document).

Statistics of Z and D (errata E-1, E-10, E-11):
  E[Z] Haar verb, any unit nouns = 2^{-2 q_n}: 0.25 (1,1), 0.0625 (2,1), 2^-8
    (4,2) -- errata l14. Z ~ Beta(d_s, D_V - d_s); sd 0.144 / 0.042 / 0.0019 --
    l15-16. MC 1e5: 0.2497 / 0.0624 / 0.0039 -- l18.
  Ansatz 0 L = 1 uniform angles: mean Z 0.2505 (1,1); 0.041 (2,1) with 53 %
    below 2^-6 -- errata l19.
  E[D] = E[Z] = 2^{-|K|} = 2^{-2 q_n} (|K| = 2 q_n postselected qubits for
    N TV N, errata l23; D = Z = p_surv in bent-wire form, spec s285): 0.25 at
    q_n = 1, 0.0625 at q_n = 2, 2^-8 at q_n = 4 -- errata l13-24, l147, spec
    l967. This is the C9 postselection denominator D, NOT the Bell-circuit
    P_post = Z / 2^{sum q_c} (spec (1.4) l215), whose Haar expectation is
    2^{-4 q_n} (R12; Section 2.2 row 29). At q_n = 2 the untrained mean equals
    the superseded fp16-store floor 2^-4 (errata l27); Haar Beta(2, 30) puts
    ~59 % of sentences below 2^-4 and ~8 % below 2^-6 (UNV: recomputed here
    from the Beta CDF 1 - (1-x)^31 - 31 x (1-x)^30); IQP ansatz 0 L = 1 puts
    53 % below 2^-6 (errata l20). Governing floor: single D >= 2^-6 on both
    paths (errata l160-163); fp32 default for |K| >= 6, i.e. q_n >= 3 (errata
    l145).
  Untrained sentences below the OLD p_min thresholds of spec 4.5 rule (3)
    (s863-864; superseded by E-11: 0.31 -> 0.020, 0.05 -> 0.00328; these are
    p_min values, not the Sec. 6.2 D floors): 69 % of (1,1) below 0.31; 47 %
    of (2,1) below 0.05 -- errata l25-26.
  1/D tail: 1.1 % of sentences D < 2^-6; CE gradient norm > 1 for 52-54 % --
    errata l137. Sec. 8 Z = 0.1403 is a typical draw -- errata l18.
Error constants (spec Sec. 4 unless noted):
  u: fp32 2^-24 = 6.0e-8; fp16 2^-11 = 4.9e-4; bf16 2^-8; Q1.15 store 2^-16 --
    spec l82.
  Per-gate c (4.2): pure fp16 2x2 10, 4x4 25, fp16-generated entries ~24,
    fp16-store/fp32-compute 1, fp16 store + fp16 gate vectors 2.4 -- spec l779.
  g_Q15(W) = 2^{W/2-15.5} + 8.6e-5: W = 8 4.3e-4, 10 7.8e-4, 12 1.5e-3, 16
    5.6e-3; g_typ 2.1e-4, 4.05e-4, 8.0e-4, 3.2e-3 -- spec l815.
  g_Q31(W) = 2^{W/2-31.5} + 1.3e-9: W = 16 8.5e-8, 24 1.4e-6 -- spec l826.
  Angle quantisation 4.8e-5 rad per gate -- spec l41. fp16 angle ulp near pi
    2e-3 rad (forbidden) -- l792. FTZ loss up to 2^{W/2-11.5} per gate (2.8e-3
    at W = 10) -- l794.
  Amplification A = 1/sqrt(p_surv) first order (3.16 at 0.1; was 12.65) --
    errata l158; Haar-init factor 2^{q_n} (was 2^{q_n+2} spec, 2^{q_n+1}
    addendum) -- errata l165.
  Bound forms: |delta P_c| <= e/sqrt(p_surv) first order; <= 2e/(2 sqrt(p_surv)
    - e) all orders; certificate 2e/(2 sqrt(D) - e) iff sqrt(D) > e -- errata
    l151-153. Spec constant 4 valid but 4x loose (MC max ratio 0.2500) --
    errata l155, review l259.
  p_min = (e/tol + e/2)^2: 0.020 (pure fp16 Ng = 50, tol 0.05), 0.00328 (Q1.15
    W = 10) -- errata l157. Addendum a709 / T23 1.6e-3 predates E-11; E-11 form
    with e = 5e-4 gives 1.05e-4 (UNV).
  Tensordot Q1.15 worst ||delta sigma||: 1.96e-4 (1,1), 3.0e-4 (2,1) with the
    E-6 pre-scale -- spec l933-937; without pre-scale 2.38e-4 / 3.33e-4 --
    errata l82. |delta P_c|: 5.2e-4 at Z = 0.1403, 1.57e-3 at 2^-6 (1,1); 2.4e-3
    at 2^-6 (2,1); typical 4.2e-4; circuit form 2.8e-3 -- errata l159. Measured
    worked example 3.11e-5 -- errata l87.
  Q1.15 stores: gate path (4.5) n' = sat_int16((acc + 2^13) >> 14) (Q3.29,
    vasr Rt8 = 14) -- spec l810, l658; tensordot path n' = sat_int16((acc +
    2^14) >> 15) (Q2.30, vasr Rt8 = 15) -- spec l918. Tensordot format:
    Q1.15 preferred over fp16-store at (1,1)/(2,1) (1.96e-4 vs 2.4e-3, spec
    l949-950 with E-11; E-9 l115-116).
  Tensordot fp16-store: 5 u16 = 2.4e-3 worst -- spec l947; at D = 2^-6
    |delta P_c| <= 0.0197 (passes 0.05); tight floor 2^-8.6; typical 0.0051 --
    errata l160. Single inference floor D >= 2^-6 both paths -- errata l161.
  Sec. 4.6 Ng_max at E <= 0.01, A = 2 (worst/typ): fp32 8333/>1e6; fp16-store
    10/318; fp16-store + hf vectors 4/69; pure fp16 1/26; bf16 0/0; Q1.15 W = 8
    11/567, W = 10 6/152, W = 12 3/39, W = 16 0/2; Q1.31 W = 16 58823/>1e6 --
    spec l875-888. Right-hand columns (E <= 0.05, p >= 0.1) grow ~4x under
    E-11 and are NOT retabulated (UNV).
  Sincos: Taylor deg 7 sin <= 3.2e-7, deg 8 cos <= 4e-8; deg 5/6 fails 2^-15;
    table 257 Q2.14 entries 514 B -- spec l836. E-9 sweep Q2.14 error 3.074e-5
    vs 3.052e-5 + 3.1e-7 -- review l195 (tolerance not fixed: UNV).
  fp16 accumulation: exact to 2048 terms of 2^-11 (sum = 1.0), stall from 2049;
    2^-12 series stalls at 0.5 -- errata l89-91.
  MPS rank guard S_tol = 1e-6 (fp32) -- addendum l1287; entropy clamp lam <=
    1e-7 in fp32 -- limits Sec. 2.2 (8)(iii) l473-474 (rule), l409 (kernel);
    A4's 1e-15 guard (l684) is fp64-only (Section 2.2 row 41).
  Guards: Z_min = 1e-12 (spec l217); eps readout 1e-9 (l334); eps_D 1e-7,
    gradient clip 1 (l968).
Memory tiers and working sets (KB = 1024 B):
  Tiers: REGISTER ~1 KB (CUDA thread) / 4 KB (HVX file); VTCM 8 KB slice
    (8 MB on v73+; 256 KB - 4 MB v66-v69 UNV); SMEM 48 KB static, 99-163-227 KB
    dynamic, Adreno local 32 KB; HBM -- addendum l17-21, spec l518-526, l637.
  Fused tensordot (1,1): WS_full 16 amps = 128 B fp32 / 64 B fp16-Q15;
    in place 10 = 80 / 40 B; (2,1): 48 = 384 / 192 B; in place 36 = 288 / 144 B
    -- spec l254, l1154, addendum (12.11) l676. General 2^{2q_n+q_s} + 2^{q_n}
    + 2^{q_n+q_s} (+ 2^{q_n}). COUNTING CONVENTION: this WS_full (16 / 48)
    includes the second noun; the addendum's N TV N / relative-clause figure
    14 / 44 omits it -- Section 2.2 row 36; quote which one you use.
  (4,2): 1,120 amps = 8.75 KiB fp32 (1,040 in place = 8,320 B) -- addendum
    l690, companion c372, review l205. (8,2) dense verb 2 MiB -- c372.
  Bent-wire state (1,1) 64 / 32 B; (2,1) 256 / 128 B -- spec l1156. Naive
    monolithic N TV N 32 / 512 amps; ADJ N TV N 128 / 8192 (64 KB fp32) -- l1159.
  Early-cup monolithic peak N TV N 24 / 160 amps -- spec l459.
  Relative clause (subject or object), tensordot (11.14): peak 2^{2q_n+q_s}
    + 2^{q_n} + 2^{q_n+q_s} = 14 (10 in place) at (1,1) = 112 (80) B fp32 /
    56 (40) B fp16-Q15; 44 (36) at (2,1) = 352 (288) B / 176 (144) B --
    addendum a239-241, Sec. 16 a1682-1683, Sec. 17 a1756-1757. Order: t1[a,c]
    = sum_b V[a,c,b] Bob[b]; t2[a] = sum_c t1[a,c]; t2 *= Alice; sigma[s] =
    sum_a t2[a] IV[a,s]. NOTE the addendum counts N TV N as 14 / 44 (no
    trailing + 2^{q_n}); spec l1154 WS_full is 16 / 48 -- row 36. who/whom
    tensors 0 B (a1689). Circuit form W_hw 8 / 32 amps (a1684); circuit
    training 768 B / 6 KB (a1685).
  Coordination 6 / 12; two relative clauses (held sibling) 16 / 48 (12 / 40)
    (a1701); 4-cycle 24 / 80; sentential complement 14 / 24; streaming 20 /
    80 -- addendum l242-243, l277, l387, l398, l1757-1758.
  Tree adjoint M_train: 400 B (T12 sentence, 50 amps); 1.7 KB (20 words at
    (2,1), 220 amps; mean 2^{q_out} = 4 UNV) -- addendum l545-548.
  Circuit-form training 24 x 2^W B: W = 3 192 B, W = 5 768 B, W = 11 48 KB --
    spec l1040; uncompacted Sec. 11 sentences 768 B / 6 KB (relative clause),
    1.5 / 12 KB (complement), 0.75-24 GB (20 words at (2,1)) -- addendum l433-440.
  S(W) fp32 / fp16: W = 6 512 / 256 B; 8 2 / 1 KB; 10 8 / 4 KB; 11 16 / 8 KB;
    12 32 / 16 KB; 14 128 / 64 KB -- spec l1168.
  Chart parser 1.375 m^2 B: m = 256 90,112 B; m = 92 11,638 B (host) --
    addendum l105. Device input 14 B/word + 2 B/cup -- l47.
  Scratch rows (E-13): HVX lane-coefficient vectors 3 KB / 2.5 KB (optional);
    vdelta controls 640 B (32-bit lanes), 768 B (16-bit); per-gate coefficient
    vectors 3 x 128 B; butterfly temporaries 16 / 32 B; Layout C reduction 128 B
    (+~2 KB training); word ids 6-12 B; bucket program bytes UNQUANTIFIED --
    errata l195-197, spec l694.
  DisCoCirc exact text 2^{N q_n} complex: N = 8, q_n = 1: 2 KB / 1 KB; N = 12:
    32 / 16 KB; N = 8, q_n = 2: 512 / 256 KB -- addendum l964, l1229.
    q_n = 2 small texts (addendum l1229-1236): N = 2 16 amps 128 / 64 B
    (registers); N = 3 64 amps 512 / 256 B (registers, W = 6); N = 4 2 / 1
    KB; N = 5 8 / 4 KB (VTCM slice fp16, W = 10). Per-wire RDM at q_n = 2:
    d^2 = 16 complex accumulator = 128 B (addendum l1403); fp32 packed 64 B,
    <= 400 B with Jacobi in place (limits l418-421); all 3 RDMs at (2,1)
    1,920 FMA, 228 B on top of 512 B state (limits l703-705). Ceilings
    N q_n <= 9 fp32 / 10 fp16 under 8 KB; 12 / 13 Adreno; 13-14 / 15 SMEM;
    19 / 20 on 8 MB VTCM -- l975. MPS break-even N >= 9 / 11 / 13 / 15 for chi
    = 4 / 8 / 16 / 32; 32 nouns at chi = 16 in 101-128 KB + 16 KB scratch --
    l1340-1345.
  Doubled register (density texts) 8 (4^W + d^2) B: W = 4 2.2 KB, W = 7 128 KB;
    ceilings CUDA thread W = 3, HVX/VTCM W = 4, Adreno W = 5, SMEM W = 7 --
    limits l258-262 (advisory).
Register budgets (targets, not facts; C14):
  CUDA Layout A (1,1): 16 complex = 32 regs + ~20 temporaries (~52; 1024
    threads/block ok); (2,1): 96 regs + temps ~110-128 -> <= 512 threads/block
    -- spec l536-540; ~110 fp32 complex -> Q <= 6; (2,2) largest (136) --
    addendum l718-720. B-blocked 2R = 2^{W-4} fp32 (W = 7..11: 8..128) -- spec
    l575. Training smem 24 x 2^W: W = 10 24 KB, W = 11 48 KB -- l625.
  HVX sentence-per-lane Q1.15 (1,1): tensordot 24, CRz 27, Rx butterfly 28-29
    (31 worst) of 32; fp16 25-29, no spill (l115); WS <= 11 ((1,1) WS = 10)
    -- errata l108-110 (counts), l121-123 (WS <= 11). Coefficient generation
    ~36 DV cycles per 64 angles per gate (~288 vs ~128 DV per 64 sentences)
    -- errata l112-113 (ESTIMATE, UNV). Full route: Section 8.1a.
  HVX throughput floor 2^{W-4} cycles per 1q gate with n_mpy = 2 (UNV) --
    spec l670. Adreno ~16 fp32 regs per fibre (derived, UNV) -- l746.
Parameter counts:
  Q = 1 word 3 (Ry-only 1); IQP L(Q-1); Sim15 2QL; Sim14 4QL; Ry-only Q --
    spec l90-99. V = 1000 (600 N, 250 TV, 100 ADJ, 50 IV), L = 1: Sim14 6000,
    IQP 2450 -- l1062; IQP L = 2: 3100 (UNV, recomputed). (1,1) IQP L = 2
    per class N 3, ADJ 2, IV 2, TV 4, ADV 2, sum 13 -- addendum a914 (13.3).
    (1, q_s = 3) command-grammar profile, IQP L = 2: N 3, ADJ 2, IV 6, TV 8,
    ADV 10 -- addendum a1553-1555 (15.3(a)). (5,2) Sim14 L = 2 sum 304 -- l1096. P0: Model A 176 angles
    (352 B), Model B 320 (640 B) -- review l551-552. Worked example 8 angles.
  L_sat (Sim14/15; IQP) Q = 2..6: 1, 2, 2, 4, 6; 6, 7, 10, 16, 26 -- addendum
    l594-597. dof 2^{Q+1} - 2 -- (12.1).
Export and model sizes:
  Header 32 B; record 20 B; angle 2 B (turns) / 4 B (pairs) -- spec l1113.
  V = 1000 at (1,1), L = 1: Sim14 6000 p 32,032 B (pairs 44,032), IQP 2450 p
    24,932 B (pairs 29,832) -- spec l1134, verify_export_bytes.c; IQP L = 2
    (baseline (1,1) choice, row 33) 3100 p 26,232 B (pairs 32,432) --
    recomputed, UNV; resident on device = angle table + 0 or 4 B/word (E-5
    l70-73), records not loaded. Word-record offsets: Section 3.4. V = 1e4,
    P_w = 5: angles 100,000 / 200,000 B; records + header 200,032 B; file
    300,032 / 400,032 B; resident 100-140 KB; string table ~9 B/word -- errata
    l42-43, l55, l70; review l111.
  Training footprint 16 x P_total: 96 KB at 6000; 800 KB at 50,000 -- errata
    l47. Lexicon (12.12): 2.4 MB at V = 5e4, P_w = 24; training 19.2 MB --
    addendum l744.
  Generator block: 808 B ((1,1) IQP L = 2, m = 32); 3,440 B with recurrent
    context; 120,052 B ((5,2) Sim14 L = 2, m = 384) -- addendum l1150. Head 5 KB
    (1,1) / 117 KB (5,2) over a frozen encoder 22-110 MB (UNV) -- l1095-1098.
  Per-sentence: angles 16 B (1,1) / 12 B (2,1) for N TV N L = 1; outputs 12 B
    -- spec l1149, l1166; working set 40-400 B -- errata l57.
Worked example (verify.c; spec Sec. 8):
  sigma = [-0.02640234-0.28465252i, 0.08196341-0.22764750i]; Z = 0.14026553;
  p = [0.58263882, 0.41736118]; Loss(y=0) 0.54018780, Loss(y=1) 0.87380330;
  P_post(Bell W = 5) = 0.03506638; "Bob loves Alice" Z = 0.16045040, p_0 =
  0.42051694 (addendum l664); Ng = 11 unfused, 7 fused; K = {0, 2}, M_post =
  0b101, S = {1}; k(0.8) = 4172; memory 16 B in, 112 B fp32 / 56 B Q15 working
  set, 12 B out -- spec l1209-1235.
Recomputed-here values (all UNV): addendum a708-709 precision consequences
  under E-11 factor 2^{q_n}: fp16-store 8e-3 at q_n = 4, 0.125 at q_n = 8;
  Q1.15 (4,2) at W = 10 1.25e-2; p_min (E-11 form, e = 5e-4, tol 0.05) 1.05e-4.


## 10. UNVERIFIED register

Grouped; each item names the test or action that would close it. Items are
those the documents mark UNVERIFIED plus recomputations flagged in this
bridge. Sources: spec Sec. 10 l1291-1314, addendum Sec. 19 l1899-1928, errata
l111-204, review Sec. 3/8, limits per-theme (8) caveats and Sec. 5.

A. lambeq / DisCoPy conventions (spec C5 l45, C8 l76, C10 l87, l99, Sec. 10
   item 1 l1293; addendum Sec. 19 item 9 l1927): theta = 2 pi phase; CRz
   symmetric vs diag(1,1,1,e^{i theta}); IQP closing H; Sim14/15 ring order;
   nested vs component-wise cups for q >= 2; symbol order in the parameter
   vector. None affects training from scratch. Close by: export one Rz, one
   CRz, one IQP word and one Sim15 word from the exact library version and
   compare numerically with T4/T5 before importing weights.
B. HVX hardware facts (spec Sec. 3.2 l636-705, Sec. 10 item 2 l1297; errata
   l111, l121; addendum Sec. 19 item 4 l1914): multiply/permute unit counts
   (n_mpy = 2), DV occupancy, vdelta control-byte conventions, widening
   multiply lane order, shuffles 1 or 2 ops, fp16 rounding/subnormals, which
   SoCs ship IEEE fp ops, contexts per cDSP, VTCM per SoC and retention across
   power collapse, coefficient-generation cycles (~36 DV), (4,1)/(3,3)
   amplitude-per-lane residency, integer divide cost, mixer op counts. Close
   by: T9 bring-up + E-9 extension on a named part and toolchain; record
   measured fl(a*b), permutation controls, cycle counts.
C. HMX / Cloud AI 100 (spec 3.3 l731-739, Sec. 10 items 3-4; addendum l1080;
   limits l328, l1186, l1223): fp16 tile generation (v73+?), accumulation
   width, ISA reachability from the adapter on v68-v79, tile layouts, vector
   unit width/registers, int16/fp32 support, tensor-core accumulation
   semantics. Close by: one 32x32 fp16 tile multiply against fp64 on the
   device; never use for probability sums until then.
D. Adreno (spec 3.4 l743-747, Sec. 10 item 5; addendum l728): wave 64 vs 128,
   ~16 fp32 regs per fibre, cl_khr_fp16 rate, native_sin/cos accuracy,
   local-memory bandwidth. Close by: occupancy/register report from the
   compiler and a T6 run in fp16 storage.
E. CUDA (spec Sec. 10 item 6 l1307; C14): cc 10.x/12.0 shared-memory maxima,
   shuffle throughput, tensor-core accumulation, every register count (ptxas
   targets). Close by: ptxas -v (0 spills) on the P0 kernels; a T6 run with
   1024 threads/block at (1,1).
F. Error-model status (spec Sec. 10 item 7 l1309; errata l134-141; review
   l227-245): "typical" columns are random-walk estimates; projector variance
   0.4-0.6 x 2^{-2W} and Var[dP_c/dtheta] = 0.06 measured once; 5 % local
   minima measured once; FTZ term device-dependent; E-9 T9 sweep tolerance not
   fixed; Sec. 4.6 right-hand columns and addendum a708-709 / T23 numbers not
   retabulated under E-11 (Section 9). Close by: re-run bp.c / second/bp_mc.c
   with new seeds; retabulate with errata_numbers.c; fix the T9 tolerance by an
   erratum.
G. Design choices left open (spec Sec. 10 item 8 l1311; addendum l1201-1203):
   L (>= 2 for ansatz 0); real-only model; nested cups (flags.bit1); wider
   lexicon (now addendum Sec. 11); ansatz_id 5 definition; which L saturates
   SO(d^2)/SU(d^2) (addendum l1203); sufficiency of L_sat beyond Q <= 3
   (addendum l586); Sim14/15 reaching density-matrix eigenvectors (l1017).
   Close by: rank checks (derivF/sym.c) at Q = 4, 5; define ansatz 5 with a T
   test before any use.
H. Grammar / front-end statistics (addendum Sec. 19 item 2 l1907, item 5
   l1918; errata l188): m ~ 2.3 x words; cycle 25-27 % and greedy failure
   46-58 % are CFG-generator figures, not natural text; W_train 25-30 for 20
   words; mean 2^{q_out} = 4; tagger 0.5-4 MB accuracy; Hobbs-style coreference
   accuracy; string-equality coreference sufficiency; "inner step" = one
   partner test. Close by: run refE/genE.c statistics on a natural-text sample
   with a tagger; measure tagger and coreference accuracy on a public set.
I. Hybrid front end (addendum Sec. 19 item 6 l1920; l928, l939, l948, l1038,
   l1096): B = 2^15 hash-bucket accuracy; mixer ~250 HVX ops; raw-byte floor
   usefulness; q_n = 4..6 heuristic; encoder sizes 22-110 MB. Close by: the
   15.4 evaluation plan with the head over a frozen encoder vs a linear probe.
J. Capacity / positioning literature (addendum Sec. 19 items 1, 3, 8 l1901-
   1925; Sec. 15 l1450-1548; review Sec. 8; limits Sec. 5 l1602-1675): every
   figure from Lorenz 2021 (except those checked against local qnlp.txt),
   Meichanetzidis 2020, Duneau 2024 (108 -> 20 qubits, readout primitive),
   Laakkonen 2024 (BQP-complete vs hard), COGS/SCAN numbers, bert-tiny, QGT
   paraphrase, S1-S17 URLs (all blocked), Bechet citation, Reck/Clements order,
   fuzz/phaser/Proj/neg exact forms, Acharya id. Close by: fetch and cite when
   network access exists; until then never quote as fact.
K. Training-dependent Z (addendum Sec. 19 item 7 l1922; limits l1286): whether
   training drives Z above 2^{-2 q_n} (would re-enable fp16/Q1.15 at q_n >= 5);
   E[Z] with pretrained/low-rank init; qudit E[Z] = 1/d_n^2 not re-run. Close
   by: log D histograms per epoch in P0 and in a (4,2) run; run ez_mc.c with
   the qudit generator.
L. Evaluation outcomes (addendum l1643, l1658, Sec. 19 item 8; review P0
   thresholds l561-563): all expected results are hypotheses until run;
   retrieval saturation at 64-256 dims; q_n = 2 threshold on role-swap. Close
   by: P0 with 5 seeds; report falsifiers.
M. Limits-report specifics (limits (8) caveats l319-330, l469-479, l613-624,
   l782-792, l939-951, l1082-1099, l1217-1224, l1356-1367, l1461-1466):
   Stinespring gradient path not run; HMX tiles >= 16x16; Gauss 3M precision;
   MPS tangent inexactness at chi = 8-16; Ragone variance prediction; JL/PCA
   signal retention; coherent parse superposition semantics; quantum-switch
   gate sets. Close by: the named programs extended per caveat; each is
   advisory until then.
N. Companion-level open items (companion l8, l77, l104, l476, l533, l855,
   l897, l1027-1040; review l363-372): no implementation/accuracy/energy
   result; verify.c covers Sec. 8 only; ratings subjective; SAP API
   availability; "Liquid DisCoCat" novelty; one-command reproduction absent;
   fourteen open decisions (seven answered by the baseline, review l166-170).
   Close by: P0 report; the rest deferred with Secs. 10-12.8 and 14.
O. Document-set hygiene (this bridge): the addendum edits owed by the errata
   and review are not applied on disk (a107, a130-131, a486/488, a708/a788,
   a721, a726, a1560/1566/1653-1657, Sec. 19 SPSA, 12.2 d << |V|); the exact
   addendum line the errata means by "a1657" (a1653 on disk also carries the
   wording) is UNV; the review's "limits absent" is stale; verify.c residual
   digits (5.55e-17, 1.14e-16, 1.68e-16) are gcc -O0 x86-64 values; M_PI needs
   _GNU_SOURCE under strict -std=c99. Close by: apply the edits as an addendum
   revision with a new hash pin (Section 12).



P. Pure-mathematics open items (puremath Appendix B, items 1-37; registered
   1.2): Conjectures 1.26, 1.30, 2.34, 7.13; #P-hardness of crossing-cup
   counting; backtracking lower bound; m ~ 2.3 x words; cup pairing of the
   reference implementation; DisCoCirc gate set of the literature; qudit
   analogues; Sim14/15 ring order and external IQP closing H; Reck/Clements
   details; the IQP mean Z = 0.041 at (2,1); training raising Z above
   2^{-2 q_n}; ratio-estimator bias; one back edge per wrapper; treewidth of
   general word graphs; inexact SVD tangents; per-site sampling constant;
   W_train growth law; global best product approximation; equality of the last
   two gradients of "Alice sleeps and dreams"; absence of barren plateaus for
   the normalised readout; saddle at the origin beyond (1,1)/(2,1); the
   Lie-algebraic variance prediction; the sharper table bound 6.03e-5; FTZ
   behaviour; general L_k bounds; multi-sentence amplification; accuracy of
   numerically verified algorithms; numerical ranks as theorems; the q_n = 4..6
   heuristic; attributions; addendum 15.3 generalisation figures. Close by: a
   proof appended to puremath, or a program in D/ plus the owning engineering
   document; dsdm risk log PM-* rows name the timebox and test for the ones
   that affect delivery.
Q. Delivery-plan assumptions (dsdm Secs. 2.4, 10; registered 1.2): capacity of
   10 ideal days per timebox (engineer 6.0, agents 4.0); size-class estimates
   (160.5 ideal days in total, 89.0 Must); 38-week nominal duration and the
   46-week judgement range; 3-5 person scaling durations; the operational
   definition of "matches" in P0 falsifier F4 and the working-set measurement
   protocol for F3 (both to be fixed at dsdm gate G0). Close by: measured
   velocity in dsdm Timebox Review Records TB1-TB2 (FND-08 calibration);
   G0 record for the two protocol decisions.


## 11. Gaps: what no document defines, and deferred items

G1  Cross-document symbol table: none exists; Section 4 of this bridge is the
    only one (D, S, N, K, L, m, c, T, E, P, k/s/c, d/d_n, wire_map).
G2  Header field for the chosen Sim14/Sim15 ring order and IQP closing-H
    beyond ansatz_id (spec C10 l99 says "state it in the header"; no field).
G3  ansatz_id 5 (full U(4), >= 15 angles): no gate list, layout, P_w,
    transpose rule or test (addendum l1201-1203).
G4  Binary encoding, byte order and versioning of cups[] / tree[] (C16 gives
    C structs only), of G2F/G1F opcodes and of the limits ctrl / dense-K fields.
G5  No test vector for nested cup pairing (flags.bit1 = 1) at q >= 2; every
    T-series value is at q = 1 where pairings coincide.
G6  Two objects named wire_map (spec 1.4 array; addendum C17 function; limits
    slot table): names must be assigned by the implementer.
G7  One packed-Hermitian layout for density matrices (spec upper triangle vs
    limits lower triangle vs full row-major) and the conjugation rule at the
    boundary.
G8  Mixing real-only and complex words in one sentence: flags.bit2 is
    whole-model; limits LIM-21 is advisory; no normative plane layout / MAC rule.
G9  Acceptance tolerance for the E-9 T9 extension (sincos sweep, ramp).
G10 Assertion-based one-command test harness: verify.c prints and returns 0;
    T2, T16, T17, T20, T22 have no program; the CRx check of T3 is not coded;
    no (2,1) numeric vector (sigma, Z, gradients) exists for any relative
    clause (T12 and T24(b) are (1,1) only; T22's 44 is a memory count).
G11 eps for fidelity-based readouts (addendum 14.6 l1425; RP n-wire readout
    l1626): (1.7)'s 1e-9 was derived for |sigma|^2 in fp32.
G12 Canonical flat parameter order for tree-form / DisCoCirc sentences (spec
    5.5 defines it for the PROGRAM; addendum 11.7 only exemplifies).
G13 Canonical qubit numbering for the Sec. 11 circuit form (no wire-map table
    like spec 1.4's for six shapes).
G14 Canonical test list: programme summary says T11-T23, addendum has T24;
    checklist item 21 sits between 16 and 17 (a1807). This bridge fixes T1-T24
    + T-EZ + errata checks.
G15 Addendum hash pin: no document pins 6b5e4bd9...6afa5; errata/review aNNN
    citations depend on the 1928-line version (pinned here, Section 1).
G16 Export representation of imported-weight conventions (C5 scaling, C6 CRz
    form, closing H, ring order, symbol order) beyond flags.bit1.
G17 Spec l176 relative-pronoun type is not labelled subject-relative; no
    erratum wording exists for s176 (E-12 fixes parser and test only);
    procedure 8.2 therefore names the subject type n^r n s^l n (a169)
    explicitly.
G18 Published tractability-contract record per profile (companion c300-308):
    every field has a source (Section 2.2 requirements) but no record exists.
G19 Export fields MPO rank r, MPS chi, readout definition, grammar/lexicon/type
    version, whole-file checksum; profile-identifier registry (review l402).
G20 Output status record layout (structural validity / task score / numerical
    validity with certificate / approximation status / fallback used):
    fields scattered (D, Z_min, p_min, saturation counter, nparses/FAIL,
    accept_prefix, eps^2, w_p); no struct; s217 uniform fallback must be
    flagged (companion c414 adopted as constraint).
G21 Addendum Sec. 16 rows for system overhead, traffic, string table;
    peak-vs-steady-state; bucket program bytes (E-13 "quantify").
G22 Un-applied addendum and companion edits (Section 10 item O; review
    conditions ii-v: evidence labels, Sec. 1.4 errata table, c944 "Correct",
    c132 thesis, Secs. 10-12.8/14 not moved, Sec. 17 column).
G23 Evidence labels: nothing in the set is "Measured"; the mapping Specified /
    Derived / Reference-checked / Hypothesis / Open -> normative text / errata
    + review programs / fp64 programs / 15.4 outcomes and P0 thresholds / spec
    Sec. 10, addendum Sec. 19, companion 19.2 exists only in this bridge.
G24 Device measurements: no ptxas report, HVX bring-up result, latency,
    throughput or energy figure anywhere; MCU / no-FPU / 1 mW / battery
    profiles have no target or toolchain (outside the spec's platform scope).
G25 P0 artefacts: 64-word CFG lexicon, label function, split manifests, 200
    paraphrases, seeds, B1-B3 C++ autograd baselines, latency/energy targets
    (only sizes exist in scope/exp0.c).
G26 Parser scores for ambiguity weights w_p in (11.25)/(11.26) (addendum l471).
G27 Per-profile declaration of semantically irrelevant input changes and
    invariances (companion c245; order sensitivity (12.10) is the only
    statement).
G28 Recomputation under E-11 of addendum a709 p_min, T23 threshold, 14.2
    clause table, spec Sec. 4.6 right-hand columns (Section 9 UNV list).
G29 Inference acceptance tolerance per deployed profile beyond P0 (companion
    c1038): only the certificate and the test tolerances exist.
G30 Sum-versus-Frobenius aggregation mandate for any future multi-line record
    profile (review l437-438: Frobenius merge collapses below Z_min for K >= 8
    at d = 64) -- recorded here so it is not lost with the deferral.

G31 HVX layout and residency for (1, q_s = 3), the P0 Model B configuration
    (review l552-553; addendum 15.3(a) l1553-1558): spec 3.2 l709-718 gives
    sentence-per-lane for (1,1) and amplitude-per-lane for (2,1) only, and the
    review verifies HVX residency only for the (1,1) lane layout (l553-554).
    Raised by dsdm (Appendix A); closing action dsdm QC-12 (TB7), result to be
    proposed as an addendum Sec. 12.3 / Sec. 16 row.

Deferred items (bind nothing before P0 reports; review l35, l136-137,
l172-175): companion Secs. 10-12.8 (accounting semantics, ACDOCA, grammar-
derived accounting composition, application portfolio, journal review, NL
search, reconciliation, explainability, Invoice-to-Payment, other processes,
distributed coordination and agent contracts) and Sec. 14 (temporal context /
LNN/CfC), with requirement ids R4, R5, R8 and register rows RV16-24, RV34-41,
RV43-60; companion 13.4 profiles other than P0 (64 KiB command interpreter,
phone similarity, journal scorer, distributed I2P); limits Tables A/B themes
(candidates, gated on P0 and on a quantum-form component beating a direct-
factor model at matched bytes, companion c450); addendum Secs. 13.3, 13.6,
14.5-14.7 and 15.3(c)-(e) applications beyond the evaluation plan.


## 12. Forthcoming documents and registry maintenance

The two documents announced in 1.1 are REGISTERED as of 1.2 (2026-09-24); their
rows are also in Section 1 and in qnlp_manifest.json documents[]:

| key | file | role | status | SHA-256 | authoritative for | not authoritative for |
|---|---|---|---|---|---|---|
| puremath | D/qnlp_pure_mathematics.md | pure-mathematics companion: definitions, theorems and proofs behind the conventions, bounds and algorithms (283 labelled results), correspondence tables, conjectures / open problems | REGISTERED 1.2; 3776 lines / 324707 B | 0254d2b100df04a147fd4595b54d0e412690ae1b0a7a75b7ad260601087a1996 | proofs and the statements of its results; its Appendix A correspondence tables (R14) | conventions, any implementation value (kernels, formats, tolerances, test values, memory, hardware), its Appendix B items, literature details; baseline changes only via a new erratum |
| dsdm | D/qnlp_dsdm_delivery_plan.md | DSDM delivery plan: PRL with MoSCoW, increments I0-I7, timeboxes TB0-TB17, gates G0-G7 (P0 gate G4), roles, risks, traceability, estimation, benefits | REGISTERED 1.2; 1789 lines / 199966 B | 3e2b0ba07867e9e527bf7372092366962a01f83fcfeb2e28b1f994ef2a7cf9ce | schedule, priority and acceptance sequencing only (R14) | any technical statement; precedence (inherits R1-R13); its estimates are UNVERIFIED (Section 10 group Q) |

Superseded in 1.2: the 1.1 placeholder hashes (64 zeros) and expected file
names of both reserved rows (the plan's file name is qnlp_dsdm_delivery_plan.md,
not qnlp_dsdm_sprint_plan.md). No hash of spec, errata, addendum, companion,
review, limits or verify.c changed. Steps 1-6 below were applied for both
documents in 1.2: rows (step 1), precedence R14 (step 2), notes in Sections 4,
5 and 7 (step 3), Section 2.2 support note (step 4), Section 10 groups P and Q
and gap G31 (step 5), reading-order entries 5a and 7a in Section 0 (step 6).

How the registry is updated when a document arrives:
  1. Compute sha256sum and wc -lc; replace the placeholder row in Section 1 and
     in qnlp_manifest.json documents[] (key, path, role, status, sha256,
     authoritative_for, not_authoritative_for); bump manifest.version.
  2. Assign the document a precedence position: a pure-mathematics document
     ranks with the review for derivations (below the errata for any spec line;
     if it corrects the baseline it must do so through a new erratum, Section
     8.8); a sprint plan ranks nowhere in technical precedence.
  3. Add its labels to Section 5 (equation index) and its symbols to Section 4
     with collision qualifiers per R10; add its tests to Section 7 only if a
     program in D/ reproduces them.
  4. Scan it for statements that touch any Section 2.2 conflict; if it takes a
     side, record the row it supports; if it contradicts a resolution, open a
     new conflict row under R9 and flag the engineer.
  5. Add its UNVERIFIED items to Section 10 and its gaps to Section 11.
  6. Re-run `jq . qnlp_manifest.json` and re-read this bridge's Section 0
     reading order to decide where the new document is inserted (a
     pure-mathematics document after step 5; a sprint plan after step 7).

Any change to the spec, errata, addendum, review, limits or verify.c
invalidates the corresponding SHA-256 in Section 1 and the manifest; the
bridge is then re-issued with a new version number and the old hashes listed
under "superseded".

End of bridge. Word count and manifest cross-check: see qnlp_manifest.json
(version 1.2.0) generated alongside this file.
