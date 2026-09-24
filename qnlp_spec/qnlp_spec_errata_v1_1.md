# Errata and clarifications for qnlp_first_principles_spec.md (SHA-256 7408d5b7fba156b29bd5e5b88f03f4fbb1f480ed95e48d834c41691f68efe86c) -- v1.1

The baseline stays byte-identical (companion v1.1 pins its hash). Each CONFIRMED correction gives spec section and
line (as on disk), original text, replacement text, one-line reason, and consequences. Derivations, Monte Carlo
numbers and program paths are in review_of_companion_v1_1.md Secs. 2 and 8; every item was re-derived and re-run
independently in a second pass (scratchpad/second/), which upheld all thirteen in substance and corrected five
details (marked in place below: E-1 sd, E-5 status, E-9 registers, E-10 Rz ratio, E-11 numbers, E-12 pattern).
E-5 is a CLARIFICATION (the spec's text is not wrong); E-1..E-4 and E-6..E-13 are corrections. verify.c is
unaffected by every item. "a" = addendum line.

## A. Corrections

E-1. Sec. 1.2, line 218. Original: "E[Z] = 1 for Haar-random words; the trained model alone drives Z small."
Replacement: "E[Z] = 2^{-2 q_n} (0.25 at q_n = 1, 2^-8 at q_n = 4) for a Haar-random verb and ANY unit nouns: sigma is
the projection of phi_V onto the span of n1 (x) e_s (x) n2, so Z ~ Beta(d_s, D - d_s) with d_s = 2^{q_s}, D = 2^{Q_V},
sd = sqrt(d_s (D - d_s) / (D^2 (D + 1))) = 0.144 / 0.042 / 0.0019 at (1,1) / (2,1) / (4,2) (for large D, sd ~ 2^{-2 q_n
- q_s/2}; that form is 23 % high at (1,1)); this is the Sec. 4.5 heuristic p_surv ~ 2^-m with m = 2 q_n and the
Sec. 5.1 statement E[D] = 2^{-|K|} (s967). Sec. 8's Z = 0.1403 is a typical draw. MC 1e5: Haar 0.2497 / 0.0624 /
0.0039 at (1,1) / (2,1) / (4,2) (second run 0.2502 / 0.0626 / 0.0039); ansatz 0 L = 1 with uniform angles 0.2505 at
(1,1) but 0.041 at (2,1) with 53 % of sentences below 2^-6 and exact cancellations: the Z_min guard is load-bearing at
q_n >= 2."
Reason: Haar covariance delta_ij / (d_n^2 d_s). The spec already carries the correct value in three places -- s858
(p_surv ~ 2^-m), T6 (s1272, Z = 0.25) and, explicitly, s967 "E[D] = 2^{-|K|}" with D = Z (s285) and |K| = 2 q_n -- so
s218 is the single inconsistent line.
Consequences: s858 "untrained A ~ 2^{q_n+2}"; s863-864 "69 % of untrained (1,1) sentences fall below 0.31, 47 % of
(2,1) below 0.05: a per-sentence guard, not a rare fallback"; s872 "p_surv = 0.1" holds for q_n = 1 or trained models;
s1104-1105: the (2,1) untrained mean equals the fp16 floor, q_n >= 3 fails both; Sec. 9 add T-EZ (mean Z 0.250 +-
0.002 / 0.0625 +- 0.0005 over 1e5 Haar triples). Addendum a705-712 already agrees.

E-2. Sec. 5.4, lines 1051-1052. Original: "Heuristic: the estimator variance is ~(P-1) mean_j (dL/dtheta_j)^2 / c^2,
so expect O(P) more steps than adjoint; ..."
Replacement: "Heuristic: for i.i.d. Rademacher Delta, Var(ghat_i) = sum_{j!=i} (dL/dtheta_j)^2 + sigma^2/(2 c^2),
sigma^2 the variance of additive evaluation noise (~u |L|: 5e-4 fp16, 1e-7 fp32); bias O(c^2) from third derivatives.
The first term is c-independent, E||ghat - g||^2 = (P-1)||g||^2, hence O(P) more steps than adjoint; the 1/c^2 term is
the only reason c matters and is what c ~ eps_mach^{1/3} balances against the bias (~2e-5 at c = 0.08 fp16). Use only
where Sec. 5.3 is unavailable."
Reason: exact for linear L; the spec's form has units [L]^2/[theta]^4.
Consequences: no design decision changes (s1049, s1108, s1252 stand); a tuning rule that enlarges c is wrong; test:
variance flat in c on a linear surrogate. Addendum: add to Sec. 19 (silent on SPSA).

E-3. Sec. 7, line 1148. Original: "MODEL (parameters), V = 1e4, mean P_w = 5 ... 100 KB (uint16 turns) ... 200 KB as
cos/sin pairs". Replacement: two rows -- "MODEL angles (learned weights): 100,000 / 200,000 B; 10 / 20 B per word vs
32 B for a 16-dim fp16 embedding row" and "MODEL word records + header (Sec. 6.3): 32 + 20 V = 200,032 B in the export
file; resident 0 B (P_w fixed per type class, computed offsets) .. 40,000 B (u32 offset per word)".
Reason: Sec. 6.3 (s1134) counts records; Sec. 7 omits 200,032 B. Note: s1148's row label is "MODEL (parameters)", so
this is an omission, not a self-contradiction; the self-contradiction is s1172 against s1148 (E-4).
Consequences: s1165 -> "16 x P_total: 96 KB at 6000 (V = 1e3); 800 KB at 50,000 (this row)"; s542 "(100 KB class)" ->
"(100 KB of angles + 0-40 KB compacted offsets: the model's weight table)".

E-4. Sec. 7, lines 1172-1178. Original: "Parameter storage is 2 bytes per angle and nothing else; no unitary, weight
table or word vector is ever stored; ... 'almost zero memory' means zero weight tables and zero materialised
unitaries, not zero state."
Replacement: "Three quantities must not be conflated. (a) PERSISTENT PARAMETERS: the angle table IS the model's weight
table (learned, indexed by word id): 100,000 B at V = 1e4 (200,000 B pairs), 200,032 B of records in the export file,
an external string table (~9 B/word), 16 x P_total in training -- about 1/3 of a 16-dim fp16 embedding table by
angles, near parity per word in the export format: small, not zero. (b) MATERIALISED OBJECTS: no 2^q x 2^q unitary and
no dense word or sentence state is ever persisted: 0 B. (c) WORKING SET: 40-400 B per sentence (per-gate scratch <=
128 B on scalar-coefficient paths), register-resident, traffic tens of bytes in and 12 B out. 'Almost zero memory' is
a claim about (b) and (c) only, a sub-kilobyte working set 750-7500x smaller than the persistent model, never about (a)
or whole-system RAM. The 2^{W_hw} state of the largest word is the smallest object for the state-vector /
fused-tensordot evaluator of Sec. 1.3, not a universal bound (Feynman-path summation needs O(W) memory at exponential
time; MPS/MPO forms store cores). The monolithic form at (2,1) with adjectives is out of budget."
Reason: s1172 contradicts s1148 and Sec. 6.3; a token-indexed learned table is a weight table by construction.
Consequences: addendum a1560, a1566, a1657 -> "no dense embedding table; the angle table is a 10 B/word weight table".

E-5 (CLARIFICATION, not a correction). Sec. 6.3, lines 1137-1139. Original: "The whole table (100 KB class) is loaded
once into VTCM / shared memory / L2; ..." The object the spec loads is the 100 KB ANGLE table; the spec never says the
20 B records are loaded on-chip, so "300,032 B > 256 KB" is not a defect of the spec and an earlier version of this
item wrongly stated it as one. What s1137-1139 leaves unstated is where the word-id -> param_offset lookup lives.
Replacement (additive): "The angle table (100,000 B at V = 1e4) plus a word-id -> offset lookup is the resident model:
0 B per word when P_w is fixed per type class and offsets are computed, or 4 B per word (u32), i.e. 100-140 KB in
total against the 256 KB VTCM low end of s639 (of which s641 says to assume only a fraction). The 20 B records of the
export file are a file format and are not loaded; the loader derives the offsets from them." Reason: makes explicit
the residency the spec assumes. Consequences: Sec. 9 item 10 gains a test that computed offsets reproduce
param_offset.

E-6. Sec. 4.7, line 930. Original: "Q1.15 path (words generated in fp32 and narrowed ONCE; ...):"
Replacement: "Q1.15 path (words generated in fp32, multiplied by S = 1 - 2^-15 and narrowed ONCE -- the tensordot
analogue of the 0x7FFF init; ...). With the pre-scale every component lies in [-S, S], every store rounds with error <=
2^-16, ||R|| <= S^2 and ||sigma|| <= S^3 stay in the box, and any saturation is a projection onto a convex set
containing the scaled target (non-expansive); P_c is invariant. WITHOUT it an fp32 component in (1 - 2^-16, 1]
saturates with error up to 2^-15 (at most one per unit object), so each constant becomes sqrt(2n + 3) 2^-16: (1,1)
2.38e-4 (2.5e-3 at Z = 0.1403, 7.6e-3 at 2^-6), (2,1) 3.33e-4 (1.07e-2 at 2^-6)."
Reason: [1.0f, 2^-16, 2^-16, 2^-16] narrows with error sqrt7 2^-16 = 1.32x the stated Q = 1 bound.
Consequences: s866-867 "(error 3e-5)" -> "adds NO error relative to the scaled reference (1 - 2^-15) U|0>, which lies
inside the box; the 3e-5 is the scale error and cancels in P_c"; T8 add the endpoint case; s1105-1106 follow; one
host-side fp32 multiply per component; s1258 tolerance 2e-3 is 67x above the measured 3.1e-5.

E-7. Sec. 9 T8, lines 1281-1282. Original: "(sum of 2^11 terms of 2^-11 in fp16 stalls)". Replacement: "(every partial
sum k 2^-11, k <= 2048, is exact and sum(2048) == 1.0; the 2049th addition 1 + 2^-11 is a half-ulp tie (ulp(1) = 2^-10)
that RNE resolves to even, so sum(k) == 1.0 for all k >= 2048: assert sum(2048) == sum(2049) == sum(4096) == 1.0. Second
series of 2^-12: sum(2048) == 0.5, sum(4096) == 0.5 vs exact 1.0. Both pass on the fp32 path to 1e-6.)" Reason: the
stated stall does not occur within 2048 terms. Consequences: none to design; the fp16-accumulation ban is confirmed.

E-8. Sec. 0 C12, line 113. Original: "a term below 2^-11 of the running sum is absorbed". Replacement: "a term below
half an ulp of the running sum (< 2^(e-11) for a sum in [2^e, 2^(e+1)); guaranteed when term <= 2^-12 x sum) is
absorbed under RNE; an exact half-ulp term is absorbed when the sum's last significand bit is even". Reason: s = 1.5,
t = 2^-11 (1 + 2^-10) is below 2^-11 of the sum yet not absorbed. Consequences: none.

E-9. Sec. 3.2, lines 709-713. Original: "(1,1) SENTENCE-PER-LANE: 64 sentences per vector; ... constants are scalar
operands, leaving 12 registers ... (~10 ops per gate per 64 sentences). On the fp16 path pre-v79 the 8 coefficient
splats per gate make it spill -- use Q1.15 there."
Replacement: "(1,1) SENTENCE-PER-LANE: 64 sentences per vector (lane l = sentence l). Every tensordot operand (n1, V,
n2) is a per-sentence state, so all 48 real products are vector x vector (Vdd.w = vmpy(Vu.h, Vv.h) / Vxx.w +=
vmpy(Vu.h, Vv.h), v60+, DV class, same multiply-unit cost as the scalar form). Gate coefficients are LANE-VARYING
VECTORS (c, s, -s in Q2.14: 3 registers per rotation gate); a scalar Rt.h is legal only for batch-uniform constants
(1/sqrt2, polynomial coefficients, shift immediates, 0x7FFF) or a word position made uniform by sorting. Registers:
tensordot 20 + 4 accumulators = 24; CRz (diagonal) on the resident verb with n1 held 27; an Rx butterfly with
lane-varying coefficients holds output 0 while computing output 1: 20 + 2-3 + 4 + 2 narrowed temporaries = 28-29
(31 if both int32 outputs are held). Peak 27 (CRz) to 29 (Rx butterfly), 3-5 spare of 32 (1 in the worst schedule),
zero permutes. Coefficient generation (instruction-count ESTIMATES under the .td DV class; no cycle model): the
Sec. 4.4 polynomial in Q1.31 lanes via vmpye/vmpyo costs ~36 DV cycles per 64 angles per gate (meets the 2^-15
budget), ~288 DV per 64 sentences vs ~128 DV for gates + tensordot; cheaper: exported (cos, sin) pairs gathered per
lane (0 DV) or precomputed word states in VTCM. Per-lane fetch is a vgather (v65+; v60-v62: scalar core or sorted
batch). fp16 path: same 3 coefficient vectors, 25-29 registers, no spill; prefer Q1.15 for the Sec. 4.3 error bound,
not for registers."
Reason: lanes hold different words; 0 of 8 gates lane-uniform in a random 64-sentence batch.
Consequences: s655-656 "v60+" is wrong for Vxx.w += vmpy(Vu.h, Rt.h) (v65+); s657, s822 "ZERO vector registers" ->
"batch-uniform constants only; lane-varying cost 3 per gate"; s714-715 two sentences per pair -> half-vector-varying
coefficients; s697-698 "4 vmux" -> "8 vsplat + 4 vmux"; s842 -> "Q1.31 lanes via vmpye/vmpyo"; Sec. 10 add vgather
(v65+) and DV-unit occupancy; T9 add the vector-form ramp and a 65536-code sincos sweep. Addendum a721 "WS <= 13" ->
"WS <= 11" (2 WS + 3 coefficient vectors + 4 accumulators + 2 butterfly temporaries <= 32; (1,1) has WS = 10 and
still fits, so the "(1,1) only" conclusion is unchanged). V6_vmpyhv / V6_vmpyhv_acc (vector x vector) are v60 per
HexagonDepInstrInfo.td; only the scalar accumulate V6_vmpyh_acc is v65.

E-10. Sec. 5.5, lines 1072-1074. Original: "Init: uniform [0, 2 pi) ... or N(0, 0.1^2) (predictable early D). Barren
plateaus (Var ~ 2^-W, |grad| ~ 2^{-W/2}) are a W >~ 20-30 phenomenon (heuristic) and do not bind at W <= 12; the
real hazard is small D."
Replacement: "Init: uniform [0, 2 pi) with a fixed seed. Do NOT use N(0, 0.1^2) for IQP words: theta = 0 is a saddle of
every IQP sentence (P_c = 1/2, gradient and diagonal Hessian exactly 0, mixed d2P_0/dt_noun dt_verb = -1/4); at
N(0, 0.1^2) RMS gradients are 6-10x below uniform init for the Rx and CRz angles and ~400x below for the Rz angles
(second order, O(theta_Rx theta_CRz); RMS dP_0/d(Rz) 0.0005-0.0006 vs 0.21-0.23; two independent runs, robust to FD
step and sample size), and E[D] = 0.2500 exactly (all cross terms have zero mean). Trainability at W <= 12 (exact
fp32): for the projector expectations N_c, D, |dN_c/dtheta| ~ 2^{-W}, Var ~ 2^{-2W} (measured once: 0.4-0.6 x
2^{-2W}, W = 4..10, Ry-Rz-CZ ansatz), not the Pauli-observable 2^{-W/2}; this does not bind the normalised readout
because D ~ 2^{-|K|}: Var[dP_c/dtheta] = 0.06 +- 0.01 flat from W = 3 to 10 (same single run). The hazards that DO
occur: (i) the 1/D tail -- at uniform init on the Sec. 8 shape 1.1 % of sentences have D < 2^-6 and the CE gradient
norm exceeds the clip of 1 for 52-54 % of sentences (two independent 2e5-sample runs; the Sec. 5.1 guard and clip are
the normal path at init); the sample maximum of |dP_0/dtheta| (71 and 27 in the two runs) is a tail statistic, bound
(1+P)/(2D) unbounded as D -> 0; (ii) the IQP saddle; (iii) non-global local minima -- 5 % of Adam runs in a W = 3
teacher-student test end at ||grad|| < 1e-8 with MSE 3e-4 to 5e-3 vs 1e-6 (measured once): use >= 3 restarts.
Re-measure the init distribution of D and Var[dP_c/dtheta] for every new profile."
Reason: wrong exponent for a projector cost; the recommended small-normal init sits on a saddle (mixed derivative
-0.25000 numerically; analytic P_0 = 1/2 - theta phi / 4 near theta = 0).
Consequences: s968-970 log clip rate and D histogram; Sec. 6.2: for |K| >= 6 the fp32 fallback is the default unless
training raises D; Sec. 8 add "dN_0/dtheta = 0 exactly for Bob t2 and loves t1" as a test vector; Sec. 9 add tests for
the saddle (mixed d2P_0/dt0 dt6 == -0.25) and E[D] == 2^{-|K|}; addendum (13.1) generator needs a non-zero bias.

E-11. Sec. 4.5, line 856 (4.7); Sec. 4.7, line 926 (4.9). Original: "||hat_psi_post - psi_post|| <= 2 ||delta|| /
sqrt(p_surv);   |delta P_c| <= 4 ||delta|| / sqrt(p_surv)" and "|delta P_c| <= 4 ||delta sigma|| / sqrt(Z)".
Replacement: "||hat_psi_post - psi_post|| <= 2 e / (2 sqrt(p_surv) - e) (e = ||delta|| < 2 sqrt(p_surv)); |delta P_c|
<= ||hat_psi_post - psi_post||; first order |delta P_c| <= 2 sqrt(P_c (1 - P_c)) e / sqrt(p_surv) <= e / sqrt(p_surv)
(4.7); runtime certificate with the computed D (|sqrt D - sqrt p_surv| <= e): |delta P_c| <= 2 e / (2 sqrt D - e), valid
iff sqrt D > e. (Sharp forms ||x/||x|| - y/||y|| || <= 2||x - y||/(||x|| + ||y||) and max_P |<a|P|a> - <b|P|b>| = sqrt(1
- |<a|b>|^2); the constant 4 is valid but exactly 4x loose.)" (4.9) likewise with Z and D.
Reason: two factor-2 looseness steps; MC max ratio to the spec bound 0.2500, tight bound attained.
Consequences: s863-864 p_min -> (e/tol + e/2)^2 against the computed D: 0.31 -> 0.020, 0.05 -> 0.0033 (0.00328;
15.2x smaller); s871-872 A -> 1/sqrt(p_surv) (12.65 -> 3.16 at 0.1), Sec. 4.6 right-hand Ng_max columns ~4x; s935
2.1e-3 -> 5.2e-4, 6.3e-3 -> 1.57e-3; s937 9.6e-3 -> 2.4e-3; s939 1.7e-3 -> 4.2e-4; s941 1.1e-2 -> 2.8e-3; s948-952
"0.078 -- FAILS" -> "0.0197 with e = 5 x 2^-11 exactly (0.0194 with e = 2.4e-3) -- passes (tight floor D >= 2^-8.6);
typical 0.0051", delete "require D >= 2^-4", "adequate for Q1.15 only" -> "for both paths"; the certificate exceeds
the first-order value by a factor 1.010 at D = 2^-6;
s1104-1108 one floor 2^-6; s1282 -> "<= 2 e / (2 sqrt(D) - e)"; s921-928 append the general-factor recurrence e_{k+1}
<= L_k e_k + eta_k, L_k = ||phi_k,mat||_op (0.54-0.85 for Haar (2,1) verbs), with addendum (12.16) or guard bits for
||phi||_F > 1. Addendum a708/a788 "2^{q_n+1}" -> 2^{q_n}.

E-12. Sec. 1.1, lines 162-163 and 176-179. Original: "Because the cups are a non-crossing matching, a stack suffices:"
and "For a wider lexicon ... replace the inner test by a depth-first search over {contract, shift} ...".
Replacement: "The cups are always a non-crossing matching; a stack can EMIT any given matching but cannot FIND one in
general (contract-vs-shift is ambiguous at a matching position). The rule below is exact only where every simple type
has at most one admissible partner (the table, as s175-176 already states). It fails one word outside the table with
single adjoints alone: 'Alice loves man in park' = n | n^r s n^l | n | n^r n n^l | n has the unique planar parse
(0,1)(4,5)(3,6)(7,8), s at 2; greedy contracts (3,4) and dead-ends; likewise 'Alice loves the man in park' (greedy
contracts loves' n^l with the's n). The failing condition: a post-nominally modified NP that is the argument of an n^l
from a word OUTSIDE the NP (object of a verb or preposition). Subject NPs with determiners or adjectives pass ('the
man in the park sleeps'), because the determiner's own n stays available for the n^r; coordination passes ('Alice and
Bob sleep'); 'Alice whom Bob loves sleeps' passes with the object-relative type n^r n n^ll s^l and fails with the
subject-relative type n^r n s^l n of s176. Do not extend by depth-first search (worst-case exponential). Use the chart
recogniser
(Addendum (11.4)-(11.5)): exact for the free pregroup on discrete basic types, (m-1)(m+1)(m+3)/24 ~ m^3/24 partner
tests and 1.375 m^2 B per committed type string; with k lexical alternatives per word, O(M^3) time over typed positions,
M <= k m. The count of parses is exponential and is charged to the ambiguity accumulators (11.25)/(11.26)."
Reason: both counterexamples verified against exhaustive search and by a second independent greedy implementation
of s165-174; DFS is exponential. Attribution: "greedy is restricted to the table" is already the spec's own statement
(s175-176); the corrections here are s163 ("a stack suffices") and s176-179 (DFS as the extension).
Consequences: Sec. 9 add the greedy-vs-chart regression with BOTH cases ('Alice loves man in park', 'Alice loves the
man in park') and the relative-pronoun type used; addendum a107 "~m^3/12" -> "~m^3/24" (exact count
(m-1)(m+1)(m+3)/24 for m = 5..197, if "inner step" means one partner test); a130-131 "fails whenever a noun is
followed by a post-nominal n^r n" -> "fails for a post-nominally modified NP that is the argument of an n^l from a
word outside the NP (object position); subject NPs with determiners pass" (NOT "a noun that is the argument of a
preceding left adjoint followed by n^r n", which 'the man in the park sleeps' matches and passes); a486/488
Word/parse need an alternatives-capable variant.

E-13. Sec. 7 lines 1163, 1173; Sec. 1.3 line 258. s1163 "(Layouts B/C only) ... 0 (Layout A/B)" -> "(Layout C only)
... 0 (Layouts A/B)" (Layout B uses shuffles, no smem). Add rows: HVX lane-coefficient vectors (optional) 3 KB / 2.5 KB;
vdelta controls at 32-bit lanes 640 B; per-gate coefficient vectors 3 x 128 B; butterfly temporaries 16 / 32 B; CUDA
Layout C reduction scratch 128 B (training +~2 KB, s627); bucket program bytes (quantify); word ids 6-12 B. s1173
"per-gate scratch is <= 128 B" -> qualified as in E-4(c). s258 (tightening, sufficient -> necessary; the original is
correct but over-conservative): "after both (all 2^q) contracted-index entries at f have been read into registers"
-> "after the accumulation at f is complete: the output overwrites only slot k = 0 at f and no other f reads it, so a
streaming accumulation that reads slot 0 first is hazard-free with one complex accumulator (8 B, not 2^q x 8 B)".
Reason: s1163 is self-inconsistent ("Layouts B/C only" vs "0 (Layout A/B)") and s556 gives Layout B no shared memory;
scratch rows absent; the 3 KB option is 10-40x the working set. Consequences: none to numbers; addendum a726 "(4,2)
fits (4160 B)" should add the constants.

## B. No change needed (companion restates the spec)

- D = Z, no 2^{-q/2} factor: s285-286, s437-438; Bell P_post = Z/4: s1218-1219 (optional after s286: "P_post(Bell) =
  Z 2^{-2 q_n} for N TV N").
- Q1.15 +1 endpoint, 0x7FFF init, saturation, norm drift, saturation counter (circuit path): s801-803, s810-811, s817,
  s865-867, s918-919.
- Controlled-rotation shift rule, repeated parameters, adjoint with general G, effect-word sign, scatter-add:
  s1002-1007, s1014, s1028-1033, s1057 (optional after s1041: direct-factor reverse rule = (5.8) seed + three
  conjugate-partner accumulations; adjoint runs on the UNFUSED gate list).
- Amplitude bytes and widths: s1145-1161, s279, s254; scratch rows: s1151-1152, s1162-1163.
- c ~ eps_mach^{1/3}, on-device SPSA condition: s1049, s1108. Norm drift: s865-866. fp16 accumulation ban: s113,
  s850. Switching lemma: s150.
