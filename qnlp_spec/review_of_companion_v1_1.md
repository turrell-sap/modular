# Review of "QNLP review synthesis and application refinement" v1.1

Reviewed document: /root/.claude/uploads/58690423-9136-5dc0-b3e6-6a28285a6c82/f1469dfb-qnlp_review_synthesis_and_application_refinement.md
(1082 lines, ~13,800 words; "companion" below). Baseline: qnlp_first_principles_spec.md, SHA-256
7408d5b7fba156b29bd5e5b88f03f4fbb1f480ed95e48d834c41691f68efe86c (recomputed on disk; matches companion line 42);
verify.c SHA-256 ecdb49e3...fa052 (matches line 45). Addendum: qnlp_spec_addendum_coverage_capacity.md, 1928 lines as
read today (under revision). Hardware-limits report: does not exist on disk (ls confirmed; addendum line 8 says the same).
Line numbers below are for these files as they stand. All verification programs are in the scratchpad directory
(paths given per item; cc -O2 -lm). Companion line references are given as "cL", spec as "sL", addendum as "aL".

## 1. Verdict

The companion is right on every technical correction to the baseline that it makes concretely, and it is right about
the two big wording defects (E[Z] = 1 at s218; "no weight table is ever stored" at s1172). Of its eleven correction
items, six are companion-right/spec-wrong (E[Z], SPSA variance, weight-table accounting, the Q1.15 endpoint on the
fused-tensordot path (4.9) at s930, fp16 T8 wording, HVX sentence-per-lane coefficients), three are partially valid
(amplitude scratch, barren-plateau statement, error-amplification policy), one splits (PARSE: "greedy is restricted"
restates s175-176; "DFS is not the only extension, polynomial methods exist" is companion-right against s163 and
s176-179), and one restates what the spec already says (D = Z / Bell factors; adjoint/parameter-shift rules).
Nothing it asserts about the baseline is false. What it gets wrong is mostly by omission: it says "recalculate" where
the recalculation was owed (HVX registers, cL482), "need measurement" where the quantity is closed-form in the linear
model (SPSA variance, cL430; only sigma and the O(c^2) bias are empirical on the real loss), and "do not specify a
qubit count" where the spec's line has the wrong exponent for its own cost function and the actual small-W hazards (a
saddle at theta = 0 under the spec's own N(0, 0.1^2) init; a 1/D tail that trips the gradient clip on half of random
sentences) go unnamed. Its Q1.15 remark (c412) is literally true of (4.9) but does not name the path, so a reader
cannot tell it is redundant against the circuit path (s801-803); it presents as new one thing the spec already has
(D = Z, s285; controlled/repeated rules, s1004-1007) and never cites the addendum, so Sections 4-9 re-request work
the addendum has delivered with numbers. Its widened scope (Secs. 10-14) is internally
consistent, contains no engineering error, and contains almost no engineering: in the typed-record profile the
pregroup fragment collapses to a fixed template plus a commutative aggregation, the only hard semantics (balance) is
exact integer arithmetic outside the model, and "almost zero memory" survives only for the per-record head. Adopt
the companion as the governing document only after (i) it carries an errata table keyed to baseline line numbers
(the file qnlp_spec_errata_v1_1.md written alongside this review is that table), (ii) it applies its own Sec. 1.3
evidence labels (currently applied to zero statements), (iii) it distinguishes "spec already covers" from "correction",
(iv) it moves Secs. 10-12.8 and 14 to a deferred profiles document and replaces the thesis sentence (cL132) with a
falsifiable experiment, and (v) its tightenings listed in Sec. 6 below are made. Until then, use it as a validated
corrections memo and scope statement, with the addendum as the technical document of record for Sections 11-19.

## 2. The companion's corrections to the baseline

| # | Correction | Companion says (line) | Spec says (line) | Verdict | Severity | Fix |
|---|---|---|---|---|---|---|
| EZ | E[Z] for Haar-random words | E[Z] = 1/d_n^2; 1/4 at q_n = 1, 1/256 at q_n = 4 (c384-396) | "E[Z] = 1 for Haar-random words" (s218); but s858 "p_surv ~ 2^-m" already implies 2^{-2 q_n}, T6 (s1272) gives Z = 0.25, and s967 states "E[D] = 2^{-|K|}" outright with D = Z, |K| = 2 q_n (s285) | companion-right / spec-wrong (spec internally inconsistent) | major | replace s218; propagate to s858, s863-864, s872, s1104-1105, s987; add test T-EZ |
| SPSA | SPSA variance heuristic | variance = sum of other g_j^2, no 1/c^2; noise adds 1/c^2 (c432) | "~(P-1) mean_j g_j^2 / c^2" (s1051) | companion-right / spec-wrong | minor | replace s1051-1052; conclusion "O(P) more steps" stands |
| WEIGHTS | "zero weight tables" | angle table is learned model storage; 100,000 + 200,000 + 32 B (c374-380) | "no unitary, weight table or word vector is ever stored" (s1172) vs MODEL row 100 KB (s1148) | companion-right / spec-wrong (s1172 vs s1148 self-contradiction; s1148 itself is an omission, its label is "MODEL (parameters)") | major | rewrite s1172-1178; add records row to Sec. 7; clarify s1137-1139 (records are a file format; the resident model is angles + 0-40 KB offsets) |
| Q15 | Q1.15 endpoint and saturation | "+1 not representable ... half-LSB bound does not cover every endpoint" (c412) | +1 not representable, 0x7FFF init, saturate (s801-803, 810, 918); drift (s817, 865-867); but (4.9) at s930 assumes 2^-16 per component on the once-narrowed words | companion-right / spec-wrong on the fused-tensordot path (4.9); redundant on the circuit path; c412 does not name the path | minor | pre-scale words by 1 - 2^-15 before narrowing, or constants sqrt(2n+3) 2^-16 (+21 %) |
| FP16 | fp16 absorption test | 2048 x 2^-11 reaches exactly 1; absorption beyond (c914) | "sum of 2^11 terms of 2^-11 in fp16 stalls" (s1281) | companion-right / spec-wrong | minor | rewrite T8 parenthesis; tighten C12 s113 |
| LANE | HVX sentence-per-lane scalar coefficients | coefficients differ across lanes; scalar operand invalid; recalculate (c480-482) | "constants are scalar operands, leaving 12 registers ... ~10 ops per gate per 64 sentences" (s709-713) | companion-right / spec-wrong; but the 64-per-vector figure survives (peak 27-29 of 32 registers, 31 worst-case schedule) | major | rewrite s709-713; qualify s656-657, s822, s697-698, s714-715, s842 |
| AMPS | amplitude table / scratch | 1,120 amps = 8.75 KiB at (4,2); "still needs separately budgeted scratch" (c361-372) | identical formulas and numbers (s1145-1163); scratch rows present | partial: numbers correct and already in spec; 5-7 scratch items missing from Sec. 7 table | minor | add rows; fix Layout B/C label s1163; tighten s258 wording (sufficient -> necessary) |
| BP | barren plateaus | do not specify a universal qubit count (c438) | "Var ~ 2^-W ... W >~ 20-30 phenomenon ... do not bind at W <= 12" (s1073-1074); init N(0, 0.1^2) (s1072) | partial: spec exponent wrong (2^{-2W} for a projector); normalised gradient O(1) at all W; real hazards omitted; companion non-quantitative | major | rewrite s1072-1074; withdraw N(0, 0.1^2) for IQP |
| ERR | error amplification 4e/sqrt(Z) | "on the order of e/||sigma||"; needs computable estimate, nonzero denominator (c402-406) | 4 ||delta|| / sqrt(p_surv) (s856, s926); D >= 2^-4 fp16 floor (s950) | partial: spec valid but exactly 4x loose; tight constant 1; computable certificate exists | major (drives D floors, p_min, A) | replace (4.7)/(4.9) constants; collapse two-tier D floor; p_min /15 |
| PARSE | parsing complexity | greedy is restricted; DFS not the only extension; polynomial methods exist (c229, 239) | "a stack suffices" (s163); greedy confined to the table (s175-176); DFS for wider lexicon (s176-179) | split: "greedy is restricted" restates s175-176; "DFS not the only extension / polynomial methods exist" companion-right vs s163, s176-179; c239's "bounded test set" warning names no target and is moot against the addendum (a88-107 is a derivation, ~m^3/24) | minor | erratum on s162-163, s176-179; addendum a107 constant m^3/12 -> m^3/24; a130-131 failure pattern |
| DZ | D = Z, Bell factors; differentiation rules | D = Z; Bell factors "already specified"; controlled/repeated rules "need their correct rules" (c396, 427, 434) | s285 (sigma exact, p_surv = Z), s1217-1219 (P_post = Z/4), s1004-1007, s1012-1037 | spec-already-covers | wording | none required; optional direct-factor reverse rule after s1041 |

### 2.1 EZ: E[Z] = 2^{-2 q_n}, not 1

Derivation. phi_V is a unit vector in C^D, D = 2^{Q_V} = d_n^2 d_s. Haar-uniform on the sphere gives
E[V_i conj(V_j)] = delta_ij / D (the only U(D)-invariant rank-2 tensor with unit trace). With sigma[s] =
sum_ab n1[a] V[a,s,b] n2[b] (spec (1.5), no conjugation),
E[Z] = sum_s sum_ab |n1[a]|^2 |n2[b]|^2 / D = d_s / (d_n^2 d_s) = d_n^{-2}. This holds for ANY fixed unit nouns; the
companion's "independent normalized noun states" is sufficient, not necessary. Stronger: the vectors n1 (x) e_s (x) n2
are orthonormal, so sigma is the projection of phi_V onto a d_s-dimensional subspace and Z ~ Beta(d_s, D - d_s):
mean 2^{-2 q_n}, sd = sqrt(d_s (D - d_s) / (D^2 (D + 1))) exactly (0.1443 at (1,1), 0.0421 at (2,1), 0.0019 at
(4,2)); the large-D form 2^{-2 q_n}/sqrt(d_s) is 23 % high at (1,1) (0.177 vs 0.144) and should be quoted only as
asymptotic.

Numeric (verEZ/ez.c, re-run today, N = 1e5): Haar (1,1) 0.24973, sd 0.1441; (2,1) 0.06243, sd 0.0420; (4,2) 0.00391.
Independent second pass (second/ez_mc.c, fresh seed, N = 1e5): 0.2502 / 0.0626 / 0.0039, sd 0.1441 / 0.0422 / 0.0019
= Beta exact; identical with the nouns fixed at |0>, confirming "any fixed unit nouns".
Spec ansatz 0 (IQP L = 1, uniform angles): (1,1) 0.25045 -- indistinguishable from Haar; (2,1) 0.04079 (below Haar)
with min 9e-13, 53 % of sentences below 2^-6 and E[1/sqrt Z] = 165: the fixed-modulus IQP verb and 1-angle IQP
nouns cancel exactly for whole angle families, so the Z_min = 1e-12 guard is load-bearing at q_n >= 2 under the
spec's own default ansatz. Sec. 8's Z = 0.1403 is 0.56 x the mean, 0.76 sd below it: an ordinary draw consistent
with 1/4 and not with 1 (Z <= 1 by (4.8), with equality only for product-structured V; max observed 0.92). The spec's
own T6 (s1272, "all angles zero: Z = 0.25") already contradicts s218, s858's heuristic p_surv ~ 2^-m with
m = 2 q_n postselected qubits gives exactly 2^{-2 q_n}, and Sec. 5.1 (s967) states "E[D] = 2^{-|K|}" outright; since
D = Z (s285) and |K| = 2 q_n for N TV N, the spec itself already carries E[Z] = 2^{-2 q_n} in Sec. 5.1, not only
heuristically. So the spec is internally inconsistent, not uniformly wrong; s218 is the odd line out.
The addendum already carries the correction (a705-712, a1923) with matching MC (0.243 / 0.061 / 0.0039).

Consequences: Sec. 4.5 rule (3) floors are not tail events under untrained words (69 % of (1,1) sentences below
p_min = 0.31; 47 % of (2,1) below 0.05); Sec. 6.2 floors D >= 2^-6 / 2^-4 sit at the untrained median for (2,1) and
reject ~100 % at q_n >= 3; A = 4/sqrt(p_surv) is ~2^{q_n+2} in expectation (MC E[4/sqrt Z] = 9.6 / 19.8 / 71). The
companion's c398 ("floors for the smallest profiles must not be transferred") is validated quantitatively.

### 2.2 SPSA: no 1/c^2 in the gradient term

For linear f, ghat_i = (g.D) D_i = g_i + sum_{j!=i} g_j D_j D_i exactly, so E[ghat_i] = g_i and
Var(ghat_i) = sum_{j!=i} g_j^2, c-independent; total E||ghat - g||^2 = (P-1)||g||^2. Dimensional check: a gradient
variance has units [L]^2/[theta]^2; the spec's (dL/dtheta)^2 / c^2 has [L]^2/[theta]^4 and cannot be right as written.
Additive evaluation noise of variance sigma^2 adds exactly sigma^2/(2 c^2). Third-order Taylor: the Hessian term
cancels in the central difference, bias_i = (c^2/2) sum_{j!=i} f_ijj + (c^2/6) f_iii; balancing bias^2 against the
noise term gives c_opt ~ (3 sigma/|f'''|)^{1/3} ~ eps_mach^{1/3} -- the spec's own c rule at s1049 is derived from
the correct model; only the heuristic sentence contradicts it.

Numeric (spsa_var.c, re-run: exact enumeration over 2^8 Delta, P = 8): var_i = sum_{j!=i} g_j^2 to six digits at
c = 0.01, 0.10, 1.00 (e.g. 0.466300 vs 0.466300); the spec formula gives 4921 / 49.2 / 0.492 at the same points.
With sigma = 0.05 noise, measured 0.86014 / 0.48437 / 0.39124 / 0.36765 at c = 0.05 / 0.1 / 0.2 / 0.4 vs predicted
0.85990 / 0.48490 / 0.39115 / 0.36771. "O(P) more steps than adjoint" survives (noise-to-signal ratio P-1 per step);
the stated reason was wrong and would have suggested enlarging c. At c = 0.08 on the fp16 forward path the noise
term is 4.9e-4^2/(2 x 0.0064) = 1.9e-5, a sharper form of s1108's "order of magnitude below the gradients".
The companion's c430 "variance ... need[s] measurement" is too weak as phrased but defensible: the variance is
closed-form in the LINEAR model (which c432 itself states), so in that model only sigma is empirical; on the real
nonlinear loss at finite c the estimator also carries the O(c^2) third-derivative bias above and c-dependent variance
terms, and those do need measurement. The right sentence is "closed-form in the linear model; only sigma and the
O(c^2) bias need measurement on the real loss". The addendum is silent on SPSA (one incidental mention, a1460): a
genuine gap there.

### 2.3 WEIGHTS: the angle table is a weight table

Numeric (verify_export_bytes.c, re-run): sizeof(header) = 32, sizeof(record) = 20 as documented. V = 1e4,
mean P_w = 5: 32 + 200,000 + 100,000 = 300,032 B (turns) or 400,032 B (Q15 pairs), plus an external string table
(~9 B/word, ~90 KB). Sec. 6.3's V = 1000 figures (32,032 / 44,032 / 24,932 B, s1134) reproduce exactly, so Sec. 6.3
does count records while Sec. 7's MODEL row (s1148) does not. The companion's three numbers are exact.

The angle table is a learned array indexed by token id via record.param_offset -- by construction a weight table
with a circuit decoder. Per word: 10 B of angles vs 32 B for the companion's 16-dim fp16 row (3.2x smaller); 30 B with
the export record (parity). So s1172's "no ... weight table ... is ever stored" is false in its middle term and
contradicts s1148; "unitary" and "word vector" are correct (0 B). Because s1148's row label is "MODEL (parameters)",
omitting the records there is an omission, not a self-contradiction; the self-contradiction is s1172 against s1148.
Working set 40-400 B vs persistent 300,032 B: ratio 750-7500x; the headline is a working-set claim, not a model-size
claim. On residency, s1137-1139 loads "the whole table (100 KB class)", i.e. the ANGLE table, not the 20 B records,
so the 300,032 B export never sits on-chip under the spec's own text; what the spec leaves unstated is where the
word-id -> param_offset lookup lives: 0 B (P_w fixed per type class, computed offsets) or 40 KB (u32 per word),
giving a resident model of 100-140 KB against the 256 KB VTCM low end of s639 (of which s641 says to assume only a
fraction). That is a clarification owed by s1137-1139, not a defect in it. Sec. 7 also mixes V = 1e4
angles with a V = 1e3 training row (s1165: 96 KB; 800 KB at P_total = 50,000). c380's "irreducible object" point is
right (a Feynman-path sum needs O(W) memory at exponential time; MPS/MPO forms store cores) but buys nothing at
W = 3..7. The addendum repeats the false wording at a1560, a1566, a1657 and must be edited in step.

### 2.4 Q15: endpoint already covered on the circuit path; a real gap on the tensordot path

Coverage: every item c412 names is in the spec with a number (s801-803, 810, 817, 865-867, 918-919). On the circuit
path, U(S e0) = S U e0 with S = 1 - 2^-15, so P_c is exactly scale-invariant (verQ15/q15_circuit.c: p0 differs by
1.1e-16 between e0 and S e0). Saturation adds NO error relative to the scaled reference: every component of S U e0
lies in [-1, 1-2^-15], and clipping to a convex box containing the target is non-expansive (q15_sat.c forced case:
error 0.000 vs S[1,0], 3.05e-5 without saturation). So s866's "error 3e-5" is conservative there.

The gap: the fused-tensordot path (s930) narrows fp32-generated unit words once with no 0x7FFF init. An fp32
component in (1 - 2^-16, 1] rounds to 32768 and saturates with error up to 2^-15, not the 2^-16 assumed by the
per-object constants sqrt(2n) 2^-16 in (4.9). At most one component per unit-norm object can lie there, so the
corrected constant is sqrt(2n + 3) 2^-16: (1,1) total 2.38e-4 vs 1.96e-4 (+21.5 %), |delta P_c| <= 2.54e-3 at
Z = 0.1403 (spec 2.1e-3); (2,1) 3.33e-4 vs 3.0e-4. Measured: [1.0f, 2^-16, 2^-16, 2^-16] narrows with error
sqrt7 2^-16 = 1.32x the Q = 1 word bound. Zero-cost fix: pre-scale words by S before the single narrowing (the
tensordot analogue of 0x7FFF); then every store rounds with error <= 2^-16 and P_c is invariant to S^3. Worked
example in Q1.15 (q15_tensordot.c): |delta p0| = 3.11e-5 vs bound 2.09e-3 (67x margin), 0 saturations; sweep of
1.98e6 random sentences with Z >= 2^-6: bound never exceeded, no int32 accumulator beyond 2^30. Independent second
pass (second/num.c): the rounding boundary x * 32768 > 32767.5 for x > 1 - 2^-16 confirmed; totals (1,1) 2.382e-4 vs
1.957e-4 (+21.7 %), (2,1) 3.330e-4 vs 2.999e-4 (+11 %). Attribution: c412's sentence ("the half-LSB interior rounding
bound does not cover every endpoint") is literally true of (4.9) at s930, so on this path it is a correct correction,
not a restatement; its defect is that it does not name the path, so the reader cannot see it is redundant against
4.3/4.5 (0x7FFF init) on the circuit path. The companion's "accumulator safety for an arbitrarily extended network" is
a scope remark: (4.8) is for factors of 2-norm <= 1 and holds there; unnormalised factors are outside the spec's
claim.

### 2.5 FP16: the stall starts at term 2049

binary16 has an 11-bit significand; every multiple of 2^-11 in [0, 1] is representable, so all partial sums
k x 2^-11, k <= 2048, are exact and the 2048th lands on exactly 1.0. The 2049th addition, 1 + 2^-11, is a half-ulp
tie (ulp(1) = 2^-10) that RNE resolves to even = 1.0; the sum then never moves. fp16_absorb.c (GCC _Float16 and an
independent RNE emulation, agreeing at every step, re-run today): sum(2048) = 1, sum(2049) = 1, sum(4096) = 1 (exact
2), sum(8192) = 1 (exact 4); first stall k = 2049. A 2^-12 series stalls at 0.5 from k = 2049 (exact 1.0 at 4096).
So a T8 implementation asserting a stall within the first 2048 additions fails, and one asserting sum == 1 passes
without exercising absorption. The companion's c914 is exactly right, including "under round-to-nearest". Secondary
(fp16_c12.c): C12's "a term below 2^-11 of the running sum is absorbed" (s113) is not the RNE criterion --
s = 1.5, t = 2^-11(1 + 2^-10): t/s = 0.67 x 2^-11 yet s + t = 1.500976562 (not absorbed). The correct criterion is
"below half an ulp of the sum", guaranteed when t <= 2^-12 s. The conclusion "never accumulate probabilities in
fp16" stands (50 % error at 4096 terms in both series).

### 2.6 LANE: coefficients are per-lane vectors; 64 sentences per vector still fits

In sentence-per-lane, lane l is sentence l, so every operand of the (1,1) tensordot (n1, V, n2) is a per-sentence
word state: all 48 real products are vector x vector. The tensordot phase never had a constant; "constants are
scalar operands" (s710) is vacuous there. Gate coefficients live in word generation (cos, sin of each lane's word's
angle) and are lane-varying unless all 64 lanes hold the same word (probability |V|^{-63} for a random batch, or by
sorting). verLane/lane_coef.c: 64 random sentences over 1000 nouns / 500 verbs, IQP L = 1: 0 of 8 angle-bearing gates
has a lane-uniform coefficient. The spec's own s711-712 ("sincos ... amortised over 64 angles at once") produces
64 different cos values in 64 lanes -- a coefficient vector -- then calls it a scalar operand.

Scalar operands that remain legal: 1/sqrt2 (and at IQP L = 1 H^{xW}|0> is the uniform vector, no multiply),
polynomial coefficients via vsplat, shift immediates, 0x7FFF, fixed gates, and word positions made uniform by sorting.
Lane-varying form: Vdd.w = vmpy(Vu.h, Vv.h) / Vxx.w += vmpy(Vu.h, Vv.h) (V6_vmpyhv / _acc, v60, DV class per
HexagonDepInstrInfo.td) -- same multiply-unit cost as the scalar form; the price is registers and generation. Side
finding: the scalar accumulate V6_vmpyh_acc requires HVXV65, so s655's "every HVX generation, v60+" is wrong for it.

Register recount (Q1.15 planar, 64 sentences): tensordot 20 state + 4 int32 accumulators = 24; CRz on the resident
verb with n1 held 20 + 3 (c, s, -s in Q2.14) + 4 = 27. That 27 is the DIAGONAL gate; a butterfly gate with lane-varying
coefficients (the nouns' Rx) must hold output 0 while computing output 1: 20 + 2-3 coefficient vectors + 4 int32
accumulators + 2 narrowed temporaries = 28-29, or 31 if both int32 outputs are held before narrowing. Peak 27 (CRz) to
29 (Rx butterfly), 31 in the worst schedule -- 64 per vector survives with 1-5 spare, not the spec's 12. fp16:
s712-713's "8 coefficient splats ... spill" is wrong twice (vectors, not splats; 2 real coefficients per rotation):
25-29 registers, no spill; "use Q1.15" stays right on Sec. 4.3 accuracy grounds. The real casualty is coefficient
generation: the Sec. 4.4 polynomial in Q1.31 lanes via vmpye/vmpyo (which s841's blanket "no 32x32 multiply"
overlooks) costs an ESTIMATED 36 DV cycles per 64 angles per gate (instruction count under the .td DV class; no cycle
model was available here -- 65536-code sweep: Q2.14 error 3.074e-5 vs budget 3.052e-5 + 3.1e-7, met), not "~10 ops":
roughly 288 DV for sincos vs 128 DV for gates + tensordot per 64 sentences, ~6.5 DV cycles per sentence. Cheaper:
exported (cos, sin) pairs gathered per lane (0 DV) or precomputed word states in VTCM.
Per-lane fetch is a vgather (v65+; v60-v62: scalar core or sorted batch). The (2,1) amplitude-per-lane path is
unaffected; the "two sentences per pair" sub-variant (s714-715) inherits half-vector-varying coefficients. Companion:
diagnosis correct; it stops at "recalculate" and implies the batch figure may fall -- it does not.

### 2.7 AMPS: all numbers already in the spec; scratch rows genuinely missing

amp72.c recomputes from the Sec. 7 header (8 / 4 B per complex amplitude): the companion's 16-cell table (c363-368)
matches; (4,2) working set 1024 + 16 + 64 + 16 = 1,120 amps = 8,960 B = 8.75 KiB and in-place 1,040 = 8,320 B;
(8,2) verb 2^18 x 8 = 2 MiB; widths 2 q_n + q_s (s279) and 4 q_n + q_s -- all correct, and all already in spec
Sec. 7 / addendum a690. The spec does budget gate scratch (32 / 128 B), sincos table (0 / 514 B), HVX permute
constants (768 B) and the CUDA exchange buffer as separate rows, so c372's "still needs separately budgeted
generation and execution scratch" is unfair as phrased. It is partially right: absent from the Sec. 7 table are the
optional HVX lane-coefficient vectors (s697: 4 per k x 6 k x 128 B = 3 KB; 2.5 KB at 32-bit lanes), 640 B of
32-bit-lane vdelta controls, the 1 KB of fp16 coefficient splats per gate the spec itself says spills, CUDA Layout C
reduction scratch (128 B; ~2 KB training, s627), butterfly temporaries (16-32 B), bucket program bytes (never
quantified), and word ids (6-12 B). The 3 KB item is 10-40x the (1,1)/(2,1) working set and must appear when that
variant is chosen. One spec defect found in passing: s1163 reads "(Layouts B/C only) S(W) | 0 (Layout A/B) | 0 / S(W)",
self-inconsistent, and s556 says Layout B has no shared memory and no barriers at W <= 6, so "Layout C only" is right.
One over-conservative wording, not a defect: s258's in-place rule "after both (all 2^q) contracted-index entries at f
have been read into registers" states a SUFFICIENT condition as the rule; since the output at f overwrites only slot
k = 0 at f and no other f reads it, a streaming accumulation that reads slot 0 first is hazard-free with one complex
accumulator (8 B, not 128 B at q_n = 4). Tighten from sufficient to necessary; nothing the spec says is incorrect.

### 2.8 BP: the spec's sentence has the wrong exponent and omits the real hazards

bp/bp.c and bp2.c (same gate conventions as verify.c; reproduce its 8 gradients to 7 digits). (A) On the Sec. 8 shape
(W = 3) RMS |dP_0/dtheta| = 0.2265 = 0.64 x 2^{-W/2}: the spec's own "barren" scale matches its own W = 3 gradients,
so exponential concentration is present at W = 3 and simply not small there. (B) For a rank-1 projector expectation
N = |psi_0|^2 with psi_0 ~ CN(0, 2^{-W}), Var[dN/dtheta] ~ 2^{-2W}, not 2^{-W} (McClean's 2^{-W} is for a Pauli
string with Tr(O^2) = 2^W); measured 0.39-0.60 x 2^{-2W} for W = 4..10 (Ry-Rz-CZ, depth W, postselect W-1 qubits;
measured once, not independently re-run). (C) It does not bind, for a reason the spec does not give: D ~ 2^{-|K|} =
2^{-(W-1)} (measured E[D] 0.2522 ... 0.0015 for W = 3..10), so Var[dP_0/dtheta] = (N' - P D')/D is FLAT at
0.055-0.074 (RMS ~0.25) from W = 3 to 10 (same single run): in exact fp32 there is no barren plateau for P_c in this
range, only a heavy tail (max 2.2-4.5, bound (1+P)/(2D)). (D) Small-W failures the spec omits (2e5 uniform inits,
Sec. 8 shape): E[D] = 0.2495 (1/4 confirmed for the IQP init, not only Haar); 1.1 % below 2^-6; the CE gradient norm
exceeds the clip of 1 for 52.1 % of sentences -- the guard and clip of s968 are the normal path at init (independent
second pass, second/bp_mc.c, fresh 2e5 inits: 0.2498 / 1.08 % / 53.6 %); max |dP_0/dtheta| = 70.6 in this sample, 27.3
in the second -- a sample-tail statistic, since the bound (1+P)/(2D) is unbounded as D -> 0, so quote it as a tail,
not a constant; at W >= 7, E[D] <= 2^-6 so 83-99.9 % of inits fall below the Q1.15 floor. (E) theta = 0 is a saddle
of every IQP ansatz-0 sentence (all |V[k]| = 2^{-Q/2}: P_0 = 1/2, first derivatives and diagonal Hessian exactly 0,
mixed d2P_0/dt_noun dt_verb = -1/4; second pass: -0.25000 numerically, analytic P_0 = 1/2 - theta phi / 4 near the
saddle); under the spec's N(0, 0.1^2) init (s1072) E[D] = 0.2500 exactly (every cross term has zero mean because <X>,
<Y> of an Rx-Rz-Rx noun are odd in the angles) -- the "predictable early D" is the saddle value -- and RMS gradients
are 6-10x below uniform init for the Rx and CRz angles and ~400x below for the Rz angles (RMS dP_0/d(Rz) 0.0005-0.0006
vs 0.208-0.225 at uniform init; second order, O(theta_Rx theta_CRz); robust to FD step 1e-5 / 1e-3 and to N = 5e4 /
2e5; an earlier figure of 50x in this review was wrong). (F) Teacher-student at W = 3 (24 sentences, 16 params, 100
Adam runs): 5 % end at non-global stationary points (||grad|| < 1e-8, MSE 3e-4 to 5e-3 vs 1e-6); 1500 steps leave
54 % unconverged (measured once; plausible, not independently re-run). (G) 4 pi periodicity is not a hazard with
unwrapped angles.

Verdict: the companion's qualification is necessary (the spec's line is a universal-qubit-count claim resting on a
Pauli-observable result with the wrong exponent for its own cost), but as written it is non-quantitative, names none
of the mechanisms above, and would be satisfied by deleting the sentence. It misses the useful true statement
(normalised gradients O(1) at every W) and the dangerous one (the spec's small-normal init sits on a saddle).

### 2.9 ERR: 4e/sqrt(Z) is valid but exactly 4x loose; a computable certificate exists

First order: delta p_c = (2/Z) Re<v, delta>, v = sigma_c e_c - p_c sigma, ||v||^2 = Z p_c (1 - p_c), so
|delta p_c| <= 2 sqrt(p_c(1-p_c)) e / sqrt(Z) <= e / sqrt(Z), attained at p_c = 1/2. All orders: the sharp
inner-product form ||x/||x|| - y/||y|| || <= 2||x-y||/(||x|| + ||y||) (the spec uses 2||x-y||/||x||, 2x loose) and
max_P |<a|P|a> - <b|P|b>| = sqrt(1 - |<a|b>|^2) <= ||a-b|| (the spec uses 2||a-b||, 2x loose) give
|delta p_c| <= 2e/(2 sqrt(Z) - e). errbound_check.c (re-run, 400k trials per cell, K = 2/4/8, Z in [2^-8, 1]):
max |delta p_c|/(4e/sqrt Z) = 0.2500 in every small-e cell, zero violations of any of the three bounds; the tight
bound is attained (ratio 1.0000). Runtime: Z is unknown but the computed D = hat_Z satisfies
|sqrt(D) - sqrt(Z)| <= e, so 2e/(2 sqrt(D) - e) is a valid computable certificate whenever sqrt(D) > e (which also
certifies sigma != 0). The spec already applies its rule to the computed D (s863, s1104, s1282) but never states the
substitution -- a rigour gap in wording, not a hole; at D = 2^-6, e = 2.4e-3 it costs a factor 1.02.

Replacement numbers (errata_numbers.c, re-run): Q1.15 (1,1) 2.1e-3 -> 5.2e-4 at Z = 0.1403, 6.3e-3 -> 1.57e-3 at
2^-6; (2,1) 9.6e-3 -> 2.4e-3; fp16-store worst at D = 2^-6: 0.078 -> 0.0197 with e = 5 x 2^-11 exactly (0.0194 with
e rounded to 2.4e-3), which PASSES tol 0.05 (tight floor D >= 2^-8.6, i.e. 2^-8.64), so s950's "require D >= 2^-4"
is unnecessary and Sec. 6.2's two-tier floor collapses to one; p_min -> (e/tol + e/2)^2, 15.2x smaller (0.31 -> 0.020;
0.05 -> 0.0033, i.e. 0.00328 -- an earlier 0.0034 in this review was a rounding slip); A -> 1/sqrt(p) first order
(12.65 -> 3.16 at 0.1), so
the Sec. 4.6 Ng_max columns grow ~4x. The addendum's 2^{q_n+1} (a708, a788) is itself inconsistent with the spec's
2^{q_n+2}; the tight factor is 2^{q_n}. The recurrence at c402: one bilinear step gives e_{k+1} <= L_k e_k + eta_k with
L_k = ||phi_k,mat||_op <= ||phi||_F = 1 for unit words, which IS (4.9); lipschitz_check.c measures L = 0.54-0.85 for
Haar (2,1) verbs, 1.000 only for product states. For ||phi||_F = s > 1 factors the gain is s^k (s = 1.5, k = 6: 11.4,
5 guard bits): the companion is right these do not inherit (4.8)/(4.9); addendum (12.16) is the existing remedy.

### 2.10 PARSE: half restates the spec, half is right against it; moot against the addendum

Switching lemma (invoked at s150): over discrete basic types a string reduces to a single target iff by contractions
alone, so the cup set is a non-crossing perfect matching and the chart (11.4)-(11.5) is exact. parseK/parsechk.c:
200,000 random strings, m = 3..19, z in [-3, 2]: chart and exhaustive search agree in every case (34,195 accepted,
max 15 parses). Exact step count (m-1)(m+1)(m+3)/24 ~ m^3/24 (m = 511: 5,592,320), so a107 "~m^3/12" is 2x high.
Greedy (s165-174) fails one word outside the six shapes with single adjoints: "Alice loves man in park" = n | n^r s
n^l | n | n^r n n^l | n has the unique planar parse (0,1)(4,5)(3,6)(7,8), s at 2; greedy contracts (3,4) and
dead-ends (independently reproduced by a second greedy implementation of s165-174, second/parse.c: FAIL with cups
(0,1)(3,4)(7,8) and three types left). So s163 "a stack suffices" is false (a stack can EMIT a matching, not FIND one)
and s176-179's DFS is worst-case exponential. Attribution: the spec already confines greedy to the table at s175-176,
so the companion's "greedy is restricted" (c229) restates the spec; its genuinely new and correct points are "DFS is
not the only extension" and "polynomial methods exist". a130-131 ("fails whenever a noun is followed by a post-nominal
n^r n") overstates the failing pattern: "the man in the park sleeps", "Alice and Bob sleep" succeed, and "Alice whom
Bob loves sleeps" succeeds with the standard object-relative type n^r n n^ll s^l (it FAILS with the spec's s176 type
n^r n s^l n, which is a subject-relative type, so that example holds only under the object type). The condition is
NOT "a noun that is the argument of a preceding left adjoint followed by n^r n" -- in "the man in the park sleeps"
the noun IS the argument of a preceding n^l (the: n n^l) and IS followed by n^r n, yet greedy succeeds, because the
determiner's own n stays available for the n^r. The correct condition: greedy fails when a post-nominally modified NP
is the argument of an n^l belonging to a word OUTSIDE that NP (verb object, prepositional object); "Alice loves the
man in park" FAILS (greedy contracts loves' n^l with the's n), while subject NPs with determiners pass. Both
counterexamples belong in the regression test. Lexical ambiguity: struct Word (a486) holds one type string; k
alternatives cost k^w charts
(131,072 charts, 3.0e8 steps at w = 17, k = 2) unless the recurrence runs over typed positions (word, alternative,
offset): O(M^3), M <= k m (2,771 steps, same 1430 parses). The exponential parse count is the semantic accumulators'
cost ((11.25)/(11.26)), confirming c239's separation of budgets. c239's "do not claim polynomial parsing from a
bounded test set" names no document (the companion never cites the addendum), so it is a generic warning, not a
misdirected one; against the addendum it is moot, since a88-93 derive the claim (switching lemma -> non-crossing
matching -> chart) and the ~500k-string tests are confirmation, not the basis. Exact step count: (m-1)(m+1)(m+3)/24
re-verified for m = 5..197 (m = 197: 323,400), so a107's m^3/12 is 1.9-2x high if "inner step" means one partner test.

### 2.11 DZ: already in the spec

adj_dz/adj_dz.c: bent-wire D = Z to 0.0 (verify.c prints P_post(bent) = 0.14026553); Bell form P_post = Z x 2^{-2 q_n}
(Z/4 at q_n = 1, Z/16 at q_n = 2, |psi[0] - sigma[0] 2^{-q_n}| <= 3.9e-17). All stated at s285, s437-438, s1217-1219.
Adjoint (5.6) needs only G Hermitian, and s1014 says so; all 8 worked-example gradients match FD to 4.5e-10; the CRz
gradient 0.3558636 = Sec. 8; a shared noun ("Alice loves Alice") gives -0.0232379 vs FD -0.0232379 by the scatter-add
of s1057; the Ry-in-effect sign_k = -1 (s1029-1033) is confirmed necessary (unsigned +0.1120352 vs true -0.1120352).
Controlled and repeated rules are at s1004-1007. The companion's c427 "need their correct rules" is a restatement.
One thing it correctly says the spec lacks (c434) is the direct-factor reverse rule; it is three lines: seed
g_sigma[s] = w_s sigma[s] (= (5.8)), then g_n1[a] += g_sigma[s] conj(V[a,s,b] n2[b]), g_V += g_sigma conj(n1 n2),
g_n2 += g_sigma conj(n1 V); verified against FD to 1.4e-10 (d_n = 2) and 8.7e-9 (d_n = 4); training memory = one
gradient tensor per word, no checkpointing. Also surfaced: the adjoint must run on the UNFUSED gate list (Ng = 11);
DENSE2/DIAG fusion (Sec. 2.6) is inference-only.

## 3. Errors and unsupported claims in the companion itself

Each checked against the file on disk (audit/audit.c and the programs above).

1. Evidence labels (cL66-75) applied to zero statements in 13,800 words (grep "[Specified]" .. "[Open]"). By its own
   rule 7.4 and 8.2 are Derived, cL77 Reference-checked, 29.4x a cited Measured result, Secs. 10-14 Specified/Hypothesis.
2. (Withdrawn as an error; kept as a wording suggestion.) cL8 reads in full "No implementation ... is established BY
   THIS DOCUMENT": a scope disclaimer about what the companion establishes, consistent with cL77 ("compiled and run ...
   matched ... supports those examples only"). An earlier draft of this review dropped the qualifier. Suggested
   wording only: "no implementation beyond the Sec. 8 reference program" (re-run: Z = 0.14026553, gradients match
   s1228).
3. cL36/cL1013 "unchanged" vs cL59/cL64 "apply the corrections": reconcilable as freeze-plus-errata. The companion's
   Sec. 17 table (cL924-940) does list every one of the five verified spec errors by baseline section ("1.2 -- Correct
   E[Z]", "3 -- coefficient-layout correction", "5 -- qualified SPSA", "7 -- parameter tables are model weights",
   "9 -- correct boundary descriptions"), so they are not "prose only". What is missing is line-keyed replacement
   text; the RV register (cL946-1007) is indexed by the third-party review's claims and is not the right home for
   baseline errata. The errata file alongside this review supplies the missing form.
4. cL944 defines "Correct" as "the original wording is unsuitable" while "original" elsewhere means the baseline
   (cL36, 58, 1013); a reader can infer the baseline claimed RV42 or RV26. grep of the spec: it did not. Fix: "the
   review's wording".
5. cL412 ("the half-LSB interior rounding bound does not cover every endpoint") is CORRECT against (4.9) at s930,
   which uses 2^-16 per component on the once-narrowed words (Sec. 2.4: +21.7 % at (1,1)), and REDUNDANT against
   4.3/4.5 (s801-803, 866, 918), where the 0x7FFF init already handles it. The companion does not say "unaddressed";
   its defect is that it names no path, so the correction cannot be placed. It should say "fused-tensordot path,
   (4.9)". The general failure to distinguish "spec already covers" from "correction" stands for cL396 (D = Z) and
   cL427 (controlled/repeated rules), and for cL229's "greedy is restricted" (s175-176).
6. cL482 "Recalculate registers, coefficient generation, and traffic" -- correct diagnosis, recalculation owed and not
   done; the implied doubt about 64 sentences per vector is not borne out (peak 27-29 of 32, 31 in the worst
   schedule).
7. cL430 "Its variance ... need[s] measurement": closed-form in the linear model (Sec. 2.2 above; cL432 itself gives
   the form), so there only sigma is empirical; on the real loss the O(c^2) bias and c-dependent variance terms do
   need measurement. Too weak as phrased, not wrong.
8. cL438: non-quantitative; the spec's exponent error and the actual small-W hazards (Sec. 2.8) are not named.
9. cL372 "still needs separately budgeted generation and execution scratch": the spec has four scratch rows; the
   companion names none of the items that are actually missing (Sec. 2.7).
10. cL406 "on the order of e/||sigma||": correct order, but the companion does not say the spec's constant is 4x loose
    nor that this looseness drives the D >= 2^-4 floor, the p_min values and the A factor -- the material consequence.
11. cL239 "Do not claim polynomial parsing from a bounded test set": generic (the companion names no document and
    never cites the addendum); moot against the addendum, whose polynomial claim is derived at a88-93 and only
    confirmed by tests.
12. cL333 QGT paraphrase "classical attention/message passing": per the WebSearch abstract (full text unreachable from
    this container; not independently verifiable here) both embedding encoding and self-attention are PQC-based; the
    29.4x number is confirmed by the same abstract. Partially incorrect, as reported.
13. cL229 / S2 (cL1049, 1067): "Bechet and collaborators" and a 2005 slide URL (HTTP 403 from here); the peer-reviewed
    result is Bechet, Studia Logica 87 (2007) 199-224, sole author (from knowledge, not from a fetched source). Claim
    correct, citation weak.
14. cL36 provenance path /Users/user/Downloads/docs/ is not reproducible by anyone else; the hash is the pointer.
15. cL108 (8.76 Wh), cL259 (2^14 >= 10^4), cL255 (IQP L = 1 constant moduli, |V[k]| = 2^{-3/2} per verify.c),
    cL314, cL310, cL537 (SAP PCA; help.sap.com blocked): correct or correct-from-knowledge; cL802 "5 x 3^20" and cL104
    ratings are third-party numbers the companion only disclaims -- unverifiable, correctly labelled.
16. Only 14 of 79 units (18 %) contain an equation, number or testable statement; Secs. 10, 12, 13, 14 (bar one
    formula), 15, 17-19 contain no design number; "must" 61 times, "specify/define/declare" 80 times; 19.2 lists 14
    open decisions with no default. A requirements document about a future specification, not a specification.

Score: 17 checkable assertions correct, 0 incorrect, 1 partially misdescribed (QGT, per abstract only), 3
unverifiable/definitional, 3 already covered by the spec and presented as new (cL396 D = Z; cL427 rules; cL229
restricted greedy, s175-176). cL412 is counted among the correct assertions (right against (4.9)), not among the
"already covered". Item 2 above is withdrawn and does not count.

## 4. Consistency with the addendum and the hardware-limits report

Hardware-limits report: absent; nothing checked. The addendum's Sec. 16 tier table and the AMPS/LANE/WEIGHTS items
above are what that report would have to carry (HVX coefficient vectors, vdelta controls, Layout C scratch, compacted
model residency).

Against the addendum the companion never cites it (grep for "addendum", "chart", "spider", "Frobenius", "DisCoCirc":
zero technical hits), so its Secs. 4-9 mostly re-request delivered work:

| Topic | Companion | Addendum | Who is right |
|---|---|---|---|
| Parsing | restricted greedy; polynomial methods exist; state complexity (c229-239) | chart (11.4)-(11.5), O(m^3), 1.375 m^2 B, verified on ~500k strings (a94-127) | agree; addendum more specific; companion's "bounded test set" warning moot; addendum's constant m^3/12 -> m^3/24 |
| Coverage | 5-item list (c203-207) | ~20-class lexicon with cups, spiders at zero parameters, cross-serial limit (11.2-11.6) | addendum more specific; punctuation delegated to front end |
| Capacity | refuses numbers (c255, 259) | dof tables, Jacobian ranks, 12.7 verdict with numbers | addendum stricter |
| MPS | O(m d chi^2), cost per operation, exact vs truncated (c314-321) | (14.7)-(14.17) per-bond memory, SVD cost, discarded weight, chi_exact rule | agree; addendum stricter |
| Density | quadratic storage (c271) | (2,1) density working set 1,120 complex, 6.7-27x ops, mixture alternative (13.2c) | agree; same number 1,120 appears for a different object in each (companion (4,2) dense; addendum (2,1) density) |
| E[Z] | 1/d_n^2 (c384-396) | 2^{-2 q_n}, MC 0.243/0.061/0.0039 (a705-712, a1923) | both right; spec wrong |
| SPSA | no 1/c^2 (c432) | silent | companion right; addendum gap; add to a19 |
| Memory | nine categories (c345-357); tables are weights (c374-380) | tier table (Sec. 16); 12.4 "almost zero is false above ~1e5 angles" (a747-753) | agree; addendum repeats the false "no embedding table" at a1560/1566/1657 and must be edited |
| Error amplification | e/||sigma|| order (c406) | 2^{q_n+1} (a708) | both loose vs tight 2^{q_n}; spec 2^{q_n+2}; addendum's own factor inconsistent with the spec it cites |
| Export | 7-item list (c490-496) | C15, 13.7 generator block | partly met; missing: MPO rank r, MPS chi, readout field, grammar version, checksum |
| Tractability contract | 7 fields (c300-308) | (11.18'), (11.19), C16, T22 | met in substance; publication as one record is boilerplate |
| Hybrid decoder | global structured decoder (c796) | local argmax + parse + FAIL-retry (13.7) | companion stricter and right in principle; moderate gap for open vocabulary only |
| HVX lanes | scalar coefficients invalid (c480) | "WS <= 13" (a721) without coefficient vectors | companion right; addendum a721 tightens to WS <= 11 (2 WS + 3 coefficient vectors + 4 accumulators + 2 butterfly temporaries <= 32), same conclusion (1,1) only ((1,1) has WS = 10 and still fits) |

One genuine disagreement: RV09 (c956) labels an "expressivity ceiling" an unsupported generalisation while addendum
12.2 derives one (sigma multilinear in word slots). The addendum's theorem is correct but its "independent of q_n, q_s,
L" is overstated: a bilinear map is free on a basis, so with d >= |V| any pair table is representable; the ceiling
holds for d << |V|, which is the only regime compatible with register/VTCM residency (12.7). The companion is wrong
against 12.2 as qualified; its remedy ("evaluate specified families") is what 15.4 already does. Companion Sec. 17
(c924-940) describes spec Sections 0-10 accurately in every "Preserve" cell, but ignores Sections 11-19, so rows 1.1
("add broader parsing" = 11.1), 1.2 ("Correct E[Z]" = 12.3), 6 ("new metadata" = C15/13.7) and 7 ("parameter tables
are weights" = 12.4) re-request delivered work; row 5 "qualified SPSA" is the one refinement the addendum lacks.

Actions on the addendum: add the SPSA correction to Sec. 19; fix a107 (m^3/24), a130-131 (failure pattern), a486/488
(alternatives-capable Word/parse); add export fields for r, chi, readout, grammar version; add system-overhead,
traffic and string-table rows to Sec. 16; qualify 12.2 to d < |V|; replace a1560/1566/1657 wording; recount a721 to
WS <= 11.

## 5. First-principles assessment of the widened architecture

(a) Algebra. The pregroup reduction (1.1) is a rewrite on ADJACENT types; cups are a non-crossing matching over
positions; word order is load-bearing (fit/fit.c block A: |sigma(n1,n2) - sigma(n2,n1)|_1 = 0.298 for Haar (2,1)
words). The companion's Journal(context, multiset_of_lines, references) (cL552) and "line extraction order should not
change an ordinary journal representation" (cL600) require a permutation-invariant K-ary composition. In the DisCoCat
toolkit there are exactly two zero-parameter candidates, both already priced in addendum 11.3: the Frobenius merge
J[i] = prod_k L_k[i] and the spider sum J[i] = sum_k L_k[i] (fit.c block B: permutation differences 5e-18 and 7e-16).
Both are commutative monoid operations -- the symmetric-monoidal fragment, not the pregroup fragment. For a journal the
derivation graph is therefore a star (K identical Line factors into one spider) and each Line is a fixed-arity
product Line(account, org_context, monetary, tx_context) (cL550). Neither depends on the input: there was never a
graph search for the grammar to eliminate, and the companion concedes as much at cL572. Its own cL310 warning applies
literally: a star's centre can hold a d^m factor; a learned 4-ary Line over d = 16 fields is 65,536 complex = 512 KB
fp32 -- the dense-verb problem of spec Sec. 7 with one more leg. A consequence neither document states: the Frobenius
merge of K unit vectors of dimension d has E||J||^2 ~ d^{-(K-1)}; at d = 64, 64^{-(K-1)} < Z_min = 1e-12 (s217) for
K >= 8 lines, so a mu-aggregated journal of 8+ lines is "undefined" under the spec's own rule unless renormalised after
every merge; the sum grows as sqrt(K). The companion leaves the choice open (cL600) where it must mandate the sum (or a
normalised mean) for multi-line journals.

(b) Balance is outside the tensor model. cL557-564 and cL594 are correct: residual[g,c] is exact integer arithmetic;
a normalised state discards magnitude (cL282) and a periodic angle map aliases amounts. The spec never claimed
otherwise -- it has no numeric-input encoder. What is left for the learned model is scoring (12.2), retrieval (12.3),
matching (12.4) and template evidence (11.3): similarity/ranking over a permutation-invariant typed-record encoding.
None is compositional semantics in the DisCoCat sense, and cL640 concedes even the readout must be a learned
bilinear form, not Fid = |<a|b>|^2.

(c) What quantum content survives. Exact identifiers have no lexical ambiguity, so density matrices (spec 1.6,
addendum 13.2c at 5.5-23x the pure working set) apply only to free-text fields and spoken instructions (cL188). The
Born readout p = |sigma|^2/Z with sum aggregation and no cups is a normalised quadratic form; complex phase carries
information only under interference, i.e. cL811's fourth option, which the companion itself flags as "not equivalent
to ordinary uncertainty over parses" -- and no business reason for two interpretations of a journal to interfere has
been proposed; cL414 ("never reinterpret a low normalization factor as evidence of financial abnormality") concedes
that Z carries no meaning in this profile. Unit-norm factors generated from angles survive unchanged, as a
parameterisation (cL276 says so honestly). In the typed-record profile "quantum" reduces to: complex unit-norm
factors, multilinear composition, quadratic readout -- a 2-byte angle quantisation of unit complex vectors. The
required ablation is already at cL450 ("PQC-generated factors versus direct factors and compact classical encoders");
it should be promoted from "required ablation" to the falsification test of the thesis.

(d) Memory by order of magnitude (fit.c block D). Per-line state 2^6 complex fp16: 256 B (register). Journal
accumulator + 2 live lines: 768 B. Identifier -> 5-angle tables for 100k accounts + 20k cost centres + 200k suppliers:
3.2 MB (10.3 MB as 16-dim fp16 embeddings) -- off-chip. Prototype store 10k x 256 B: 2.6 MB. Historical graph, 50M
ACDOCA lines/yr, edge list: ~0.6 GB (12.8 GB with per-line states). An ACDOCA row itself carries several hundred
columns, > 1 KB per record before any model. So the head is 10^-3 to 10^-6 of the system. "Almost zero memory"
survives for the per-record contraction on an endpoint with a local vocabulary of 10^2-10^3 identifiers (tables
1-10 KB, states 256-768 B, all in VTCM or one thread; cL186-188 and cL193 describe exactly this participant). It does
not survive for the enterprise journal scorer. The addendum draws the same line for hybrid NLP (a1739-1740: "the
memory of the SYSTEM is then the front end's, and the 'almost zero' claim applies to the head alone"); cL357 is the
same point; neither states the ratio for the ACDOCA profile.

(e) LNN/CfC (cL756-839). A CfC cell (h = 16/32/64, m = 32) has state 64/128/256 B fp32 and ~2.3k/6.2k/18.6k
parameters (2-19 KB int8; exactly 2,352 / 6,240 / 18,624 under the modelling assumption of a three-head cell, i.e.
three dense (m + h) -> h maps with bias -- a choice, not a fact about any particular library's cell), ~6k MACs per
event at h = 32. The state is register-compatible and sits in the slot the
addendum's recurrent context gate (13.9) already defines; the CfC only makes the update depend on elapsed time. It is
a non-unitary nonlinear recurrence, so Sec. 4.1's argument does not apply and cL402's recurrence is the right tool.
State must persist per case: 100k open cases x 128 B = 12.8 MB before hypotheses and replay (cL839 is right that the
cell size bounds nothing). Verdict: compatible as an optional angle-generator input; irrelevant to the quantum thesis;
system cost = cases x (state + hypotheses + replay). Trap: a generator initialised near zero output (addendum (13.1),
or a CfC feed) starts on the IQP saddle of Sec. 2.8(E).

(f) Coherence with the goal (s5, "NLP on quantum theory with almost zero memory"). For typed records the three terms
fail in turn: not NLP (cL165), quantum reduced to a parameterisation (c), almost-zero head-only (d). The companion is
internally consistent because it commits to almost nothing (14 open decisions, 13 application rows, 6 participant
classes, 9 process roles): good discipline, poor falsifiability. The minimal falsifiable core: (1) a sentence profile
-- pregroup, tensordot kernel, (q_n, q_s) <= (2,1), lexicon <= 1e3 words (angle table <= 10 KB), spiders for relative
clauses/coordination; working set 40-400 B, whole system <= 64 KiB; falsifier: an equal-byte real direct-factor model
matches it on addendum 15.4; (2) a typed-record profile -- fixed Line template, per-field angle factors, SUM
aggregation, quadratic readout, local vocabulary <= 1e3, exact integer balance beside it; working set <= 1 KB, resident
<= 32 KB; falsifier: a DeepSets encoder at equal parameter count with the same rule layer matches anomaly precision at
fixed review budget (cL716); (3) everything with identifier tables > 100 KB, prototype stores, historical graphs or
persisted per-case CfC state is a server/gateway profile with no "almost zero memory" claim.

## 6. Recommended edits to the companion (section-level)

- Sec. 1 (cL8, 36): optional wording "no implementation beyond the Sec. 8 reference program" (cL8 is already correct
  as a scope disclaimer); drop the local path. Sec. 1.3: apply the six labels to every assertion in Secs. 5-9; mark
  Secs. 10-14 Specified/Hypothesis.
- New Sec. 1.4 "Errata against the frozen baseline": adopt qnlp_spec_errata_v1_1.md (12 corrections and 1 clarification
  (E-5) keyed to spec lines with replacement text); one row per erratum in a new baseline-errata table beside Sec. 17,
  not in the RV register (which indexes the third-party review's claims).
- Sec. 3.1 (cL132): replace the thesis sentence with the P0 statement (Sec. 7 below) ending in a "falsified if" clause.
- Sec. 4.3 (cL229, 239): state the derivation (switching lemma -> non-crossing matching -> chart, ~m^3/24 per committed
  type string, O(M^3) over alternatives) and cite addendum 11.1 as that algorithm; "do not infer completeness of a
  parser from a bounded test set (the stack rule fails on 'Alice loves man in park' and 'Alice loves the man in park':
  a post-modified NP in object position)"; say that "restricted" is what s175-176 already states. S2 -> Bechet, Studia
  Logica 87.
- Sec. 7.2 (cL372): replace the scratch sentence with the enumerated missing items (Sec. 2.7); note "KB" is binary.
- Sec. 7.3 (cL376-380): say s1148 already books 100 KB and Sec. 6.3 already totals records, so the defect is the
  self-contradictory s1172-1178 and the missing 200,032 B row; give the per-word figures (10 vs 32 B; 30 B with
  record); records are a file format, and the spec loads only the angle table (s1137); the offset lookup adds 0-40 KB
  (resident 100-140 KB, flash 300-400 KB); name the string table (~90 KB); supply the replacement sentence it asks
  for (errata E-4); name the alternatives behind "irreducible object".
- Sec. 7.4 (cL386-398): "for any fixed unit noun states"; add Beta(d_s, d_n^2 d_s - d_s) with the exact sd; replace
  "need their own analysis" with the ansatz-0 numbers (0.2505 at (1,1); 0.041 at (2,1), 53 % below 2^-6); note s858
  and s967 already carry the value and a705 does too; add E[D] = 0.2495 for the IQP init.
- Sec. 7.5 (cL402-406): tight constant (2 sqrt(p(1-p)) <= 1 first order; 2e/(2||sigma|| - e) all orders); the
  baseline's 4e/sqrt(Z) is valid but 4x loose and drives its D >= 2^-4 floor and p_min; the computable certificate is
  2e/(2 sqrt(D) - e); the recurrence with L <= 1 IS (4.9) and is new only for ||phi||_F > 1 (addendum (12.16)).
- Sec. 7.6 (cL412): name the path: "correct against (4.9) (fused-tensordot path, s930: 2^-16 per component assumed on
  once-narrowed words); already handled on the circuit path by 4.3/4.5 (0x7FFF init; saturation non-expansive); fix:
  pre-scale by 1 - 2^-15 before the single narrowing, or constants sqrt(2n+3) 2^-16, +21 %".
- Sec. 8.2: cite the baseline's controlled/repeated rules with the verified numbers (cL427); qualify "variance ...
  need measurement" (cL430) to "closed-form in the linear model (cL432); sigma and the O(c^2) bias are measured on the
  real loss"; tighten cL432 with (P-1)||g||^2, the O(c^2) bias and sigma^2/(2c^2), and say the baseline's conclusion
  stands; after cL434 add the three-line direct-factor reverse rule ("one gradient tensor per word, no
  checkpointing").
- Sec. 8.3 (cL438): the quantitative statement (dN ~ 2^{-W}, Var 2^{-2W}; dP_c O(1) at every W; hazards: 1/D tail
  with 52-54 % clipped at init, IQP saddle under N(0, 0.1^2) with Rz gradients ~400x down, ~5 % local minima at W = 3
  measured once); require the init distribution of D and Var[dP_c/dtheta] per profile. Sec. 8.4: add "init scheme
  and restart count".
- Sec. 9.2 (cL480-484): append the recomputation (vector x vector products, peak 27-29 of 32 registers, 64 per vector
  survives, ~36 DV per gate per 64 lanes as an instruction-count estimate, vgather v65+, scalar accumulate v65+) so it
  is a bound, not a to-do.
- Sec. 11.5 (cL600): mandate sum (or normalised mean) aggregation; forbid the Frobenius product for K >= 8 at d = 64
  (Z-collapse); state the Line factor's d^4 size.
- Sec. 13.4 (cL727-733): fill the twelve fields of one profile with numbers (Sec. 7 below).
- Secs. 10-12.8 and 14 (with R4, R5, R8 and RV16-24, RV34-41, RV43-60): move to a deferred "domain profiles" document;
  keep 12.1 as a one-table appendix; one paragraph gating them on P0 and the cL450 ablation.
- Sec. 16.2 (cL914): name spec T8 (s1281), the half-ulp-tie mechanism, and the 2^-12 series (stall at 0.5 from 2049).
- Sec. 17: add "delivered by addendum section" so Secs. 4-9 shrink to residual gaps (SPSA; export fields r, chi,
  readout, grammar version; system-overhead/traffic/string rows; global decoder for open vocabulary).
- Sec. 18 (cL944): "the REVIEW's wording"; RV03 -> "recognition O(m^3)/O(M^3) by the chart; exponential cost attaches
  to parse count and k^w enumeration only".
- Sec. 19.3: S2 archival reference; S5 paraphrase corrected; S7 Sci. Rep. 15:7155 / arXiv 2308.07865; S1, S8-S12
  marked UNVERIFIED; S6 "MPS" not in the abstract.

## 7. The minimal falsifiable programme

P0 (sized in scope/exp0.c). Grammar S -> NP VP; NP -> N | ADJ N; VP -> TV NP | IV | IV ADV; lexicon 64 words (24 N,
12 ADJ, 12 TV, 8 IV, 8 ADV): 1,190,592 distinct sentences, all six baseline shapes, greedy parser exact. Model A:
(1,1), IQP ansatz 0, L = 2: 176 angles = 352 B; peak 16 amplitudes (10 in place) = 128 B fp32 / 64 B Q1.15; training
2.8 KB Adam+grad + 192 B statevectors; K = 2. Model B: (1,3): 320 angles = 640 B; 52 amplitudes = 416 / 208 B; K = 8
(the addendum's 15.3(a) configuration). Residency verified by ptxas -v (0 spills) and by the HVX Q1.15 lane layout
with vector coefficients (peak 27-29 of 32), not by argument. Label: a deterministic function of (subject, verb, object) classes
with label(A TV B) != label(B TV A) on every role-swap pair, so a bag model is at 50 % by construction. Data: 4,000
train / 1,000 IID / 500 role-swap / 300 held-out (S,V,O) triples / 200 hand-written paraphrases for parse-FAIL; 5
seeds. Baselines (int8, own C++ autograd): B1 bag-of-embeddings 528 B; B2 tiny transformer (d = 48, L = 2) 46.2 KB;
B3 GRU d = 64 28.6 KB and a d = 8 GRU (16-32 B state) as the small-state control; a real direct-factor model at equal
bytes (spec 1.6) as the ablation of the quantum parameterisation. Metrics: accuracy on IID / role-swap /
triple-holdout; parse-FAIL rate; measured state bytes; counted ops; registers/spills; D histogram and clip rate at init
and after training. Predeclared: IID QNLP >= 97 %, B2 >= 97 %, B1 <= 75 %; role-swap QNLP >= 90 %, B1 = 50 %, B2
55-80 %; triple-holdout QNLP >= 90 %, B2 70-90 %. Falsified if QNLP role-swap <= B2 at 4K sentences, or IID < 90 %, or
the measured working set exceeds 128 / 416 B, or the equal-byte direct-factor model matches QNLP on every split.
Budget: one engineer, two weeks, CPU training with one CUDA GPU for the residency check; HVX Q1.15 port in week
three on a v65+ part (vgather) or with a sorted batch.

Decisions that gate P0 (companion 19.2, answered by the baseline): first extension -> none; formalism -> pregroup,
greedy (chart when relative clauses enter); family -> IQP ansatz 0, uniform [0, 2 pi) init (NOT N(0, 0.1^2));
dimensions -> (1,1) L = 2 then (1,3); data -> CFG-generated, no SAP data; device -> CUDA Layout A fp32 train, HVX Q1.15
sentence-per-lane infer with vector coefficients; numeric acceptance -> the tight certificate 2e/(2 sqrt(D) - e)
against fp64, Q1.15 with the 1 - 2^-15 pre-scale. The other seven 19.2 decisions gate nothing before R4.

Defer: Secs. 10-12.8 (ACDOCA, Invoice-to-Payment, distributed participants) and 14 (CfC) until P0 reports and until a
quantum-form component beats a direct-factor model at matched bytes (cL450). The first typed-record experiment, when
it comes, is the profile of Sec. 5(f) item 2 with sum aggregation and a DeepSets falsifier; its "almost zero memory"
claim is head-only and must be stated as such.

## 8. Sources check summary

All 17 companion URLs were blocked by the egress proxy. WebSearch confirmed S5 (Aktar et al., Adv. Quantum Technol.
2026; 29.4x confirmed; cL333's architecture paraphrase disputed: the abstract says embeddings and self-attention are
both PQC-based), S6 (Zhao et al., KBS 2026; exists; "MPS" not in the abstract; preview only, cL1053), S7 (Harvey,
Yeung, Meichanetzidis, Sci. Rep. 15:7155, 2025; companion cites only a repository page), S14 (LTC, AAAI 2021, DOI
10.1609/aaai.v35i9.16936). From knowledge, identifiers and content correct: S3 quant-ph/0511069, S4 1008.3477, S13
N19-1357, S15 s42256-022-00556-7, S16 1907.03907, S17 2005.08926. S2: a 2005 slide deck; the peer-reviewed result is
Bechet, Studia Logica 87 (2007) 199-224, sole author; the claim is correct (also Oehrle 2004, Degeilh-Preller 2005,
Preller 2007, Moroz 2010) but the citation is weak, and Bechet et al. 2014 (NP-completeness for products of free
pregroups) should be noted since cL206-207 admits richer structures. S1, S8-S12: expected form, standard content,
UNVERIFIED; S11/S12 are S/4HANA on-premise and ERP 6.17 pages for an unspecified target release. Buszkowski 2001 (LACL
2001, LNAI 2099) for the CFG equivalence and switching lemma is cited by the addendum (a454), consistent. Each
companion source is cited exactly once in the body; no orphans.

Verification programs (all in /tmp/claude-0/-home-user-modular/58690423-9136-5dc0-b3e6-6a28285a6c82/scratchpad/):
verEZ/ez.c; spsa_var.c; verify_export_bytes.c; verQ15/q15_tensordot.c, q15_circuit.c, q15_sat.c; fp16_absorb.c,
fp16_c12.c; verLane/lane_coef.c (with HexagonDepInstrInfo.td); amp72/amp72.c; bp/bp.c, bp2.c; errbound_check.c,
lipschitz_check.c, errata_numbers.c; parseK/parsechk.c; adj_dz/adj_dz.c; fit/fit.c; audit/audit.c; consist/ez.c,
spsa.c, cyk_steps.c; scope/ez.c, exp0.c. verify.c rebuilt and re-run: Z = 0.14026553, all Sec. 8 numbers reproduced.
Independent second pass (fresh seeds, separate implementations): second/ez_mc.c (Haar and ansatz-0 E[Z], Beta sd),
second/beta.c (Beta CDFs 0.691 / 0.463), second/bp_mc.c (E[D], clip rate, saddle, N(0, 0.1^2) gradient ratios),
second/parse.c (greedy per s165-174 vs planar matching; chart step count), second/num.c (export bytes, Q1.15
rounding boundary and sqrt(2n+3) totals, fp16 stall, SPSA noise term, error-bound numbers, Frobenius K threshold,
P0 counts). Every Sec. 2 verdict was reproduced; the corrections of detail from that pass (Rz ~400x, greedy pattern,
28-29 register butterfly, s1137 loads angles not records, 0.0033 / 0.0197, exact Beta sd, s967) are marked in place.
