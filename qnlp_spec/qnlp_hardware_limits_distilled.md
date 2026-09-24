# What QNLP has not done, why, and what it becomes as GPU/NPU maths

Companion to qnlp_first_principles_spec.md (Sec. 0 conventions C1-C13, Sec. 7 memory budget) and
qnlp_spec_addendum_coverage_capacity.md (C14-C17, Secs. 11-19, Sec. 16 tiers). Plain-ASCII maths, C-like
pseudocode, no Python; everything is pasteable into C++/Mojo comments. Equation labels of the spec / addendum
((S.n), (11.n)..(19.n)) are cited, never re-derived; new labels are per theme: (NU.n), (SF.n), (G.n), (T.n),
(HO.n), (L.n). Network access was unavailable to every worker and verifier in this run (arxiv.org,
aclanthology.org, docs.quantinuum.com egress-blocked; WebSearch budget exhausted), so literature statements are
from memory unless a verifier reached a mirror (lambeq source via raw.githubusercontent.com; the "QNLP in
practice" PDF via the CQ mirror); anything else that matters is tagged UNVERIFIED. Every number quoted in
Sections 2 and 4 was produced by a plain-C fp64 reference program in the scratchpad (paths given per theme).

Memory tiers (C14): REG = one CUDA thread (~1 KB) or one HVX context (4 KB); VTCM = 8 KB slice; SMEM = CUDA
shared 48-227 KB / Adreno local 32 KB; HBM = global / L2 / DDR. (q_n, q_s) = (1,1) and (2,1) as in Sec. 7;
d = 2^{q_n}; W = live qubits; bytes are fp32 complex (8 B) unless stated.

---------------------------------------------------------------------------------------------------------------

## 1. One-page answer

The engineer's question was: what did the QNLP programme propose, want, or avoid because real quantum hardware
imposes few qubits, shallow depth, noise, finite shots, exponential postselection, unitary-only evolution, no
cheap mid-circuit measurement, restricted connectivity, a slow classical loop, qubits only, expensive tomography
and toy datasets -- and what does each of those things become when the whole model is run as a classical
statevector / density-matrix / tensor-network kernel on a GPU or NPU with a register-resident working set.

Sixty-one candidate capabilities were extracted from the literature and adjudicated by three verifier lenses
each (historian: was it actually blocked by hardware; physicist: is the maths what the candidate says; kernel
engineer: does it fit the near-zero-memory design). They collapse into nine themes. Table A holds the eight
themes in which at least one verifier found a genuinely hardware-limited member; Table B holds the members and
the one theme that QNLP has not done but NOT because of hardware. Rank = impact x cheapness x novelty, each
scored 1-5 (editorial; impact = what it would change for a text-level task, cheapness = how little it adds to the
Sec. 7 / Sec. 16 working set, novelty = distance from anything already run in lambeq / DisCoPy / numpy).

### Table A. Genuinely hardware-limited themes (ranked)

    #  theme                      hardware limit                        classical operation it becomes            kernel shape                          working set (1,1) / (2,1)                     training rule                                 already-done status                              score
    -- -------------------------- ------------------------------------- ----------------------------------------- ------------------------------------- --------------------------------------------- --------------------------------------------- ------------------------------------------------ -----
    A1 Non-unitary word maps on   block-encoded phaser succeeds w.p.    Schur-in-eigenbasis update (NU.4):        d x d dense complex gemm per word;    word tile 96 B / 384 B; pure single-Kraus     adjoint with LINEAR seed (O - P I)/T, Schur   semantics computed in numpy at word level        5x4x4
       density matrices: fuzz /   tr(sigma rho) compounding per update; U (Wgt .* U^H rho U) U^H, Kraus sums,    lane-wise Schur layer + existing      text state = unitary cost (32 B at W = 2;     layer passed as lambda .*= Wgt, occurrence    (Piedeleu 2015 .. Rodatz 2021); NEVER inside a   = 80
       phaser / Mult / Proj,      fuzz needs eigenbasis of a LEARNED    strided-reduction discard (Stinespring)   butterflies on HVX; doubled register  128 B at W = 4); doubled register 160 B      table (NU.5), dWgt rule (NU.6), free-Kraus    trained text circuit, never register-resident,
       Kraus/CPTP words, negation,operator (tomography) + discard;      neg(rho) = I - rho as an affine word-     = pure 2W-qubit kernel                (W = 2) / 2.2 KB (W = 4); ceiling W = 7      (NU.7)                                        negation impossible on any device (non-CP)
       Frobenius copy/merge       CPM doubling halves qubits; I - rho   level array op                                                                  (128 KB)
                                  is not CP (universal NOT)
    A2 Spectral functionals of    no shot-efficient estimator for       rho = P P^H (SF.1); 2x2 closed form       strided Hermitian outer-product       16-48 B (1,1); <= 400 B (2,1) single wire;    Frechet derivative F (d x d Hermitian), one   born classical on count-derived rho (Blacoe      5x5x3
       tiny Hermitian rho: S,     S(rho); purity needs two copies +     (SF.2) / cyclic complex Jacobi (SF.3);   accumulate + scalar closed form /     two-wire (2,1) d = 16: ~6 KB (SMEM/VTCM)      GEMM lambda_Psi = F_eff Psi (SF.7), then      2013, Piedeleu 2015, Meyer-Lewis 2020); undone   = 75
       Renyi, purity, Loewner /   controlled-SWAP; PSD test, k_max,     eigenvalue sums (SF.4), two d x d gemms  Jacobi                                                                              (5.6)-(5.7) or (11.28)-(11.29)                on CIRCUIT-generated, entanglement-mixed
       k-hyponymy, Uhlmann F,     Uhlmann F, trace norm need the        for sqrt/log (SF.5), MI (SF.6)                                                                                                                                             states, end-to-end trained
       trace distance, rel. ent.  full spectrum (tomography)
    A5 DisCoCirc text circuits:   controlled sub-circuit = controlled   masked dense K-wire gemv (HO.1)-(HO.2),  strided 2^K gemv over blocks,        (S+1) x 8 x 2^W + 512 B gate (1,1);           Sec. 5.3 with the control predicate,          unitary sandwich frames exist in lambeq;         4x4x4
       higher-order boxes,        U(4) (tens of CNOTs); non-unitary     superoperator vec(B') = S vec(B) (HO.3), W-independent small gemm, pointer   (2,1) 8 KB dense G -> gate-by-gate           lambda_B = sum lambda_o v^H, DFS-accumulated  classical statevector training used by Duneau    = 64
       retained s wires, bounded  frames need ancilla + postselection   retained s wire = zero-fill kron-in      offset for top-field slicing         (register tier)                              slice gradients, hedge dpsi/dt = (B - I) psi  2024; controlled / hedge / retained / sliced
       frontier, wire slicing     (exponential in # wires); width grows (HO.5), slice = pointer offset (HO.6),                                                                                                                                  forms never run anywhere
                                  with clauses; interpretability =      trace = sum over slices (HO.7)
                                  tomography
    A3 Exact pure-state overlaps  every pair needs compute-uncompute /  Hermitian GEMM G = Psi Psi^H (G.2),       small Hermitian GEMM + GEMV;          Psi 8 B D + G 8 B^2: B = 32, D = 2: 512 B +   (G.7)-(G.9): one more Hermitian GEMM          cosine similarity IS the founding classical      4x5x2
       and Gram matrices:         swap / Hadamard test at O(1/eps^2)    F = |G|^2, InfoNCE (G.4), slot GEMV +    streamable per-anchor logsumexp       4-8 KB (streamable to 12 B/anchor); slot     Lambda_Psi = (dL/dF .* conj G) Psi, then      experiment (Grefenstette-Sadrzadeh 2011);        = 40
       similarity, retrieval, QA, shots: O(B^2/eps^2) runs per gradient quadratic form (G.6)                                                          32 B / 64 B                                  (11.28)-(11.30) per sentence                  undone as a TRAINED objective for angle models
       contrastive, slot scoring  step; V x shots per slot                                                                                                                                                                                       and as batched exact-gradient kernel
    A4 Partial trace, RDMs, full  one shot = one bitstring; joint needs elementwise |psi|^2, Walsh-Hadamard       real butterfly (WHT), strided        148 B extra at (1,1) W = 3; 228 B extra at    (T.5) lambda = (Lambda (x) I) Psi; Lambda =   lambeq NumpyModel returns amplitudes; never a    4x5x2
       joint / marginal           O(2^n/eps^2) shots; RDM = 3^q Pauli   butterfly for ALL Z-correlators (T.3),   Hermitian Gram accumulate, tiny GEMM (2,1) W = 6; nothing 2^W x 2^W              -log2 rho for S, 2 rho for purity, MI form    TASK / OBJECTIVE (entropy timelines, MI,         = 40
       distributions              bases; no-cloning: one question per   marginals (T.4), all m RDMs in one pass                                                                                                                                 all-answers QA) with a tiny working set
                                  run                                   (T.1), all-answers GEMM
    A6 Exact training: Fubini-    FS metric = O(P^2) Hadamard/SWAP      complex Jacobian J by 2^{q_s} adjoint     Sec. 2 butterflies + Gram GEMM        Jacobian 320 B / 864 B; Hessian 384 B /       QNG theta -= eta (F_FS + lam I)^-1 grad;      adjoint + weight tying standard in lambeq        3x4x3
       Study natural gradient     tests; Hessian = 4-eval shift rule,   sweeps with seed e_{b(c)}; F_FS = Re(     J^H J + P x P solve / Jacobi eigen    1536 B + K; solves P^3/3                     GN/LM (F_cl + mu I) delta = -grad;            classical backends; exact FS metric never in     = 36
       (only the QNG member is A) O(P^2 S) shots; SPSA forced on        J^H J / D - conj(v) v^T / D^2); Hessian                                                                                    saddle-free |H|                               QNLP; Hessian/GN/LM/BP diagnostics = tier B
                                  devices                               by forward-over-reverse
    A8 Vocabulary / dataset       no QRAM: loading a d-vector costs     gathered lexicon rows into the fused      batched GEMV with gathered operands;  96 B / 320 B streamed operands (80 / 288 in   (11.27)-(11.29) with the gathered row;        classical DisCoCat did pretrained / low-rank     4x4x2
       scale: streamed lexicon,   O(d) depth; vocab 17 (MC) / ~115 (RP) tensordot (L.3), low-rank verb init      verb-resident, noun-streamed          place); tables MB-class (HBM / VTCM >= 2 MB)  Euler identity makes the raw gradient         verbs in numpy (2011-2020); lambeq tensor path   = 32
       pretrained amplitude       words; shots x queue per circuit      (L.2), fidelity loss (L.4)                                                                                                  tangent: step + rsqrt retraction              random-init, one JAX lambda per diagram
       encoding, shape batching
    A7 Wire representation:       every hardware run used q_n = q_s = 1 mixed-radix register butterfly, Givens / real strided radix-d gemv; tile     (2,2) 80 B; (4,2) 288 B; register d = 4, two tree adjoint with digit maps; four-term      arbitrary d / real field / dense tensors =       3x4x2
       qudit / mixed-radix dims,  (per-cup postselection 1/d^2, depth,  Reck U(d) generated transiently from     shapes: 16x16 complex = one 32x32 fp16 wires 128 B + 2 KB gate                       shift or adjoint (generators have eigen-      original classical DisCoCat / lambeq             = 24
       multi-qubit types, dense   SWAP); DisCoCirc gates in U(d^2),     d^2 angles; field choice = 4M / 3M /     HMX tile; unequal radices fill tiles                                               values {0, +-1})                              TensorAnsatz; the unitary, angle-generated,
       U(d) maps, number field    d > 2 never run                       real GEMM                                                                                                                                                                 postselected form with q >= 2 / d > 2 unrun

### Table B. Undone, but NOT because of hardware (ranked)

    #   candidate / member                             real reason it was not done                          classical operation                       kernel shape                 working set (1,1) / (2,1)     training rule                            already-done status                       score
    --- ---------------------------------------------- ---------------------------------------------------- ----------------------------------------- ---------------------------- ----------------------------- ---------------------------------------- ----------------------------------------- -----
    B1  postselection norm Z = ||sig||^2 as            community discarded the norm (cosine focus,          ln Z_k per parse; softmax over parses     reduction + log per parse    K fp32 (32 B at K = 8)        root seed lambda_sigma = sig_k / Z_k;    never proposed as a feature; lambeq       4x5x4
        plausibility / parse-choice score              symbolic grammaticality); on devices it is the                                                                                                            parse-posterior CE                       returns it unnormalised                   = 80
                                                       noisiest number (accepted-shot fraction)
    B3  coherent superposition of parses; quantum-     programme chose MIXING on semantic grounds           S = sum c_k sig_k; Gram K x K; switch =   axpy + Hermitian GEMM; two   K d_s complex + K(K+1)/2      lambda_c = <sig_k|lambda_S>; both gate  no QNLP paper proposes either             2x5x5
        switch ordering of DisCoCirc gates             (Piedeleu 2015; arXiv:2504.00040)                    two gate orders + one dot                 gate orders + dot            Gram + K fp32 Z (448 B at K   orders scatter-add                      (UNVERIFIED negative)                     = 50
                                                                                                                                                                                   = 8, d_s = 2; +32 B w or c); switch 64 B
    B2  soft parse forests, Sinkhorn cup placement,    parser (DepCCG / Bobcat) taken as a fixed prior;     w = softmax(alpha), A = Sinkhorn(L),      n x n normalisations +       36 B (n = 3) + 20 iterations  dP_c/dalpha_k = w_k (P_c^(k) - P_c);     Gumbel-Sinkhorn / DIORA exist in          3x4x4
        learned frame nesting                          classical structure learning never imported          n~_a = sum_b A[a][b] n_b                  mixing                       stored or recompute           unrolled Sinkhorn adjoint                classical NLP, not in QNLP                = 48
    B5  exact Hessians, Gauss-Newton, Levenberg-       community focused on hardware-executable optimisers  forward-over-reverse K; H, F_cl, LM solve P x P dense solve / Jacobi   H 4 P^2 B; K 8 2^{q_s} P^2 B  (M + mu I) delta = -grad; mu > -lam_min  none in QNLP; trivial in any autodiff     3x4x3
        Marquardt on word-parameter sets               and toy datasets                                                                                                                                                                             framework                                 = 36
    B19 generative QNLP: sampling / scoring sentences  the model defines p(answer | text) via (14.19), not  slot scoring GEMV (G.6); Sec. 14.7        GEMV over vocabulary table   32 B / 64 B + streamed table  slot chain of (G.6) in (11.27)          Sec. 14.7 already prices reranking;       3x3x3
        from circuit semantics                         p(sentence); community never posed it                                                                                                                                                       no experiment                             = 27
    B6  barren-plateau diagnostics (gradient variance, always a classical-simulation activity; not done for  Monte-Carlo Jacobians; Pauli bit-set     one draw per thread / lane;  Jacobian WS per draw; dim^2   none (diagnostic)                        McClean 2018 / Ragone 2024 general;       2x4x3
        DLA dimension) for QNLP ansaetze               DisCoCat / DisCoCirc ansaetze by anyone              closure for dim g                         XOR closure                  x 2W-bit                                                               not for QNLP ansaetze                     = 24
    B10 dense per-word unitaries / arbitrary tensors   Sim14 already saturates the state manifold (12.1)    U(d) Reck product from d^2 angles, or     transient d^2 gemm            8 d^2 B transient             (11.30) on the Givens list               lambeq TensorAnsatz = arbitrary tensors   3x4x2
        instead of 1-3 IQP / Sim14 layers              for Q <= 3; dense buys expressivity only for MAPS    stored row                                                                                                                                                                         = 24
    B12 noise-free scaling studies, ansatz sweeps,     Castillo et al. 2025 already compared IQP/Sim14/     same kernel, loop over (ansatz, L, q_n,   shape-batched launches       as Sec. 7                     as baseline                              partially done (Castillo 2025);           3x4x2
        standardised 1e4-1e5 sentence benchmarks       Sim15 classically; no benchmark by community choice  q_s) headers                                                                                                                                 no standard benchmark                     = 24
    B13 cross-lingual / language-independence by       existing studies are purely grammatical (Waseem     F = |<sig_1|sig_2>|^2/(Z_1 Z_2), Tr(rho^1 dot + (14.19) per noun         2 x 2^{q_s} + d^2             (L.4) seeds, shared words scatter-add    grammar-only comparisons                  2x4x3
        exact state fidelity                           2022 and follow-ups): no trained states              rho^2)                                                                                                                                                                             = 24
    B11 dataset scale: thousands of sentences per      engineering; classical backends could always have    bucket by (shape, W); word-id triples +   batched launch per shape     B x (2^{q_s} + 1) fp32 out    as baseline                              lambeq compiles one JAX lambda per        4x5x1
        launch                                         done it; nobody built the batched kernel             per-shape program                                                                                                                             diagram (no shape batching)               = 20
    B7  joint / curriculum training with weight tying  Duneau 2024 already trained classically before H1-1; "+=" into theta_w across occurrences     scatter-add                  10 B / word angle table       Sec. 5.5 tied gradient                   done at small scale in lambeq             3x3x2
        on the largest simulable texts                 scale limited by datasets, not physics                                                                                                                                                                                                = 18
    B4  exact adjoint gradients for 1e5-1e6 parameters lambeq classical backends already autodiff; scale    Sec. 5.3 adjoint; sparse row update       butterflies                  24 x 2^W B                    (5.6)-(5.7)                              done (lambeq NumpyModel / Pytorch)        3x5x1
        instead of SPSA                                never attempted because datasets are toy                                                                                                                                                                                                = 15
    B14 word-level non-unitary updates (fuzz / phaser  born classical: Coecke-Meichanetzidis 2020, Lewis    (NU.1)-(NU.4) on a stored rho             d x d gemm                   96 B / 384 B                  (NU.6)                                   computed in numpy 2015-2021               2x5x1
        / negation on a stored rho)                    2019/2020, Rodatz-Shaikh-Yeh 2021 used numpy                                                                                                                                                                                             = 10
    B15 entropy / fidelity / Loewner order on hand-    born classical (Blacoe 2013, Sordoni 2013, Piedeleu  (SF.2)-(SF.5) on a mixture rho            closed form / Jacobi         16-400 B                      (SF.7)                                   done (2015-2020)                          2x5x1
        built lexical mixtures                         2015, Meyer-Lewis 2020 PyTorch)                                                                                                                                                                                                       = 10
    B16 cosine / inner-product sentence similarity on  founding classical experiment (Grefenstette-         dot product                               reduction                    2^{q_s} complex               n/a                                      done (2011-2014)                          2x5x1
        DisCoCat vectors                               Sadrzadeh 2011, Kartsaklis 2013, Milajevs 2014)                                                                                                                                                                                       = 10
    B17 reading amplitudes / per-actor RDMs at         lambeq NumpyModel / PytorchModel already return      (14.19)                                    strided Gram                 32 B / 128 B per wire         n/a                                      done for interpretability (Duneau 2024,   2x5x1
        simulable width for interpretability           amplitudes; Duneau et al. inspect Bloch states                                                                                                                                                  UNVERIFIED detail)                        = 10
    B8  arbitrary / unequal local dimension per type,  original classical DisCoCat (d_n ~ 2000); lambeq     radix-d contraction with digit strides    strided radix-d gemv          d_n d_s (d_n + 1) amps        digit-mapped tree adjoint                done (2010-2020, lambeq Dim)               2x5x1
        qudit Frobenius spiders                        TensorAnsatz ob_map takes any Dim                    (12.16')                                                                                                                                                                             = 10
    B9  real / complex / mixed number field            design choice; Ry+CNOT real ansaetze run on QPUs;    re[] only, or 4M / Gauss 3M complex GEMM  GEMM variant                 halves bytes when real        same                                     done (real ansaetze standard)             2x5x1
                                                       only phase READOUT is a hardware cost (theme A4)                                                                                                                                                                                       = 10
    B18 multi-qubit / multi-dimensional types in       lambeq tensor ansatz takes any Dim; only the         (1.3) with larger d                        fused tensordot              48 amps at (2,1)              (11.27)-(11.30)                          done classically                          2x4x1
        non-unitary tensor form                        unitary Born-rule form is unrun (theme A7)                                                                                                                                                                                                = 8
                                                       counted under A7; listed here only for its non-unitary tensor half
    B20 syntactic ambiguity as a parse mixture         VUB (Eisinger et al. 2025, arXiv:2504.00040) did it  (11.26) accumulator                       K x d_s^2 complex            32 B                          dP_c/dw_k closed form                    done classically 2025                     2x4x1
        (density matrix over derivations)              classically                                                                                                                                                                                                                             = 8

### Table A/B reconciliation (member -> row)

Every Table B row is a member of one of the nine themes; the table below maps each theme's members to their tier so
that the headline count can be checked by hand. B18 is the non-unitary TENSOR half of A7's multi-qubit-type member
(the unitary, angle-generated, Born-rule half of the same member is tier A) and is not counted twice.

    theme  members  tier-A members (count)                                                                          tier-B rows
    -----  -------  ----------------------------------------------------------------------------------------------  ------------------------------------------------
    A1     5        DisCoCirc updating in-circuit, CPTP verbs, copy/merge at scale, trained non-unitary maps (4)     B14
    A2     5        S / Renyi / purity, F and T for WSD, Loewner + divergences (3)                                    B15, B20
    A3     4        overlap as trained objective, semantic search over circuit meanings (2)                           B16, B19
    A4     2        all-answers joint / marginals (1)                                                                B17
    A5     2        full DisCoCirc text circuits, bounded-cut many-actor texts (2)                                    --
    A6     5        exact Fubini-Study natural gradient (1)                                                          B4, B5, B6, B7
    A7     4        UNITARY angle-generated q_n, q_s >= 2 / d > 2 DisCoCat and U(d^2) DisCoCirc gates (1)           B8, B9, B10  (B18 = the non-unitary TENSOR
                                                                                                                     half of the same member; not counted twice)
    A8     4        streamed lexicon + pretrained amplitude encoding (1)                                              B11, B12, B13
    B      3        -- (0)                                                                                            B1, B2, B3
    -----  -------  ----------------------------------------------------------------------------------------------  ------------------------------------------------
    total  34       15                                                                                               19 distinct members (+ B18 as a half-member)
                                                                                                                     + 27 tier C (Section 3) = 61 candidates

Headline finding. Of 61 candidates, 15 drew at least one hardware-limited vote from a verifier (14 if the
multi-qubit-type member of A7 is filed under its tensor form B18), 19 (20) were judged undone for other reasons
(community focus, a semantic choice, or plain engineering that nobody built), and 27
were already done classically or rejected on inspection (Section 3; three of those 27 also drew a hardware
vote but the capability was found already implemented off-device, so they were filed as tier C). The engineer's
question therefore has a narrower true answer than its framing suggests: the QNLP programme was a CLASSICAL
programme first (DisCoCat 2010, density matrices 2015, lambeq's numpy / PyTorch backends), and most of what
hardware forbids was simply computed in numpy at word or phrase level long before any device run. What hardware
genuinely prevented -- and what no classical backend then went back and did -- is the combination: non-unitary
and spectral semantics INSIDE a trained text circuit, on states whose mixedness comes from entanglement between
noun wires rather than from a hand-built mixture, read out as exact density matrices, overlaps, entropies and
joint distributions, and differentiated exactly through all of it in a working set of tens of bytes to a few KB.
Every one of the Table A themes is that combination seen from a different side; every one of them reduces to
four primitives on a register-resident state (Section 4): a small dense complex matmul, a butterfly, a strided
Hermitian reduction, and a d x d eigensolve. None needs a 2^W x 2^W object, and none leaves the register tier at
(1,1) or the VTCM / SMEM tier at (2,1) for texts of up to 6-7 noun wires.

---------------------------------------------------------------------------------------------------------------
## 2. The themes, with their distilled maths

Each theme follows the same eight headings: (1) quantum-theoretic definition, (2) why it is undone (the honest
version after verification), (3) exact classical formulation, (4) kernel shape / flops / working set, (5)
composition with the register-resident evaluator (Sec. 1.3 tensordot, Sec. 1.4 circuit, Sec. 11.7 tree, Sec. 14.1
text register, Sec. 14.3 MPS), (6) training rule, (7) minimal test with exact reference numbers, (8) caveats and
UNVERIFIED items. The text is the workers' distillation, verbatim up to consistent conventions.

### 2.1 Theme A1. Non-unitary word maps on density matrices: Kraus/CPTP channels, fuzz/phaser/Mult/KMult/Proj updates, negation, Frobenius copy/merge

Members: DisCoCirc meaning updating of density matrices (fuzz and phaser, Lewis's Mult / KMult / Proj) | verbs
and functional words as general CPTP maps (Kraus / Choi form): open-system DisCoCat and the CPM doubling |
Frobenius copy/delete and non-unitary word maps at scale | training non-unitary word maps with unconstrained
parametrisations | non-unitary meaning updates: conversational and logical negation on density matrices.
Reference program: scratchpad/nonunit/nu.c (fp64, C1-C13). Tier A-hardware-limited with the scope correction in (2).
New labels (NU.n); addendum labels cited, not re-derived.

(1) Quantum-theoretic definition. State space: a q_n-qubit noun wire carries rho in D(C^d), d = 2^{q_n}, Hermitian
PSD, stored row-major rho[a][a'] (a = ket = row, a' = bra = column, exactly the doubled-index convention of (13.10)).
In CPM(FHilb) every wire is doubled and a word is a completely positive map Phi(rho) = sum_{k<r} K_k rho K_k^H
(Kraus rank r); DisCoCirc lets a sentence/adjective act as such a map on the wires it mentions instead of the unitary
G_V of (14.4). Coecke & Meichanetzidis 2020 (arXiv:2001.00862) derive from the two CPM spiders two canonical noun
updates by a positive operator sigma = U diag(r) U^H = sum_i r_i P_i (UNVERIFIED wording, definitions from memory):

    fuzz_sigma(rho)   = sum_i r_i P_i rho P_i                 Kraus ops sqrt(r_i) P_i, rank d      (NU.1)
    phaser_sigma(rho) = sqrt(sigma) rho sqrt(sigma)           single Kraus op, rank 1              (NU.2)
    Mult(rho, sigma)  = rho .* sigma   (Schur product)        = the QUANTUM spider merge (11.7) applied to ket and bra index at once   (NU.3)
    KMult = (NU.1),  BMult = (NU.2)  (Lewis 2019 names; the members' "phaser = KMult" is probably swapped: UNVERIFIED)
    Proj_sigma(rho)   = P_supp rho P_supp,  P_supp = U diag(r_i > 0) U^H   (Lewis's exact "Proj" UNVERIFIED; the
                        alternative B rho B = sigma rho sigma is also covered below)
    dec(rho)          = sum_i |i><i| rho |i><i| = fuzz_I     (the CLASSICAL spider of Sec. 11.3, i.e. (11.6) doubled)
    neg(rho)          = (I - rho) / (d - 1)                   logical negation (Lewis 2020, UNVERIFIED exact form)
    neg_ctx(rho)      = normalise( Upd_{sigma_ctx}( I - rho ) ), Upd in {Mult, fuzz, phaser}   (Rodatz-Shaikh-Yeh 2021 shape, UNVERIFIED)

All of (NU.1)-(NU.3), Proj and B rho B are ONE family, the Schur-in-eigenbasis update:

    Upd(rho) = U ( Wgt .* (U^H rho U) ) U^H,   Wgt[a][a'] = fuzz: r_a delta_{aa'};  phaser: sqrt(r_a r_a');  B rho B: r_a r_a';
               Proj: [r_a>0][r_a'>0];  Mult: U = I, Wgt = sigma;  interpolation: sqrt(r_a r_a') (a==a' ? 1 : gamma)   (NU.4)

By the Schur-product theorem the map is CP for every PSD Wgt, so any PSD weight matrix (including the gamma-family,
gamma in [0,1], from fuzz to phaser) is a valid update. Pre-normalisation trace T = sum_a Wgt[a][a] (U^H rho U)_aa =
tr(sigma rho) for fuzz AND phaser (identical T; they differ only in the eigenbasis off-diagonals). Frobenius copy on a
density wire is (11.8) read twice (ket and bra); merge is (NU.3); the classical copy/merge is dec followed by (11.7) on
the diagonal.

(2) Why undone -- honest version. Three verifier lenses agree that the density-matrix strand was born classical and
computed in numpy at word/phrase level (Piedeleu et al. 2015; Bankova et al. 2019; Lewis 2019; De las Cuevas et al.
2020; Meyer & Lewis 2020; Rodatz-Shaikh-Yeh 2021), so "the semantics were never computed" is false. What IS
hardware-blocked, and therefore never appeared in any device or device-shaped (lambeq quantum ansatz) QNLP run, is
these maps INSIDE a trained text circuit: phaser is a block-encoded non-unitary with success probability tr(sigma rho)
compounding over every update of a text; fuzz needs the eigenbasis of a LEARNED operator (tomography), mid-circuit
measurement and discard; a rank-r Kraus word needs ceil(log2 r) ancillas per occurrence plus discard, and CPM doubling
halves the usable qubit count; parameter-shift exists only for the dilated angles; and neg(rho) = I - rho is affine
and equals Y rho^T Y at d = 2 -- the (non-CP) universal NOT, impossible on any device even with postselection. Duneau
et al. 2024 (arXiv:2409.08777) accordingly used unitary noun updates only. lambeq's to_tn(mixed=True) and DisCoPy
channel.py do implement CPM doubling classically (verifiers checked the source), but as dense numpy tensors, not as a
register-resident kernel with exact gradients: the unlock is the KERNEL and its composition with Secs. 14.1/14.3, not
the semantics.

(3) Exact classical formulation. Doubled register: vec(rho)[a + d a'] = rho[a][a'] (ket bits low, bra bits high,
C1/C7). rho -> A rho B^H is the gate A on the ket field and conj(B) on the bra field (C2 Kronecker conj(B) (x) A).
Conjugating a C10 gate list: conj(Rx(t)) = Rx(-t), conj(Rz(t)) = Rz(-t), conj(CRz/CRx(t)) = same with -t, conj(Ry) =
Ry, H/CNOT/CZ unchanged (the complement of the C11 transpose rule, which negates only Ry).

    // Schur-family update of noun wire j (q_n qubits) on a W-wire text density matrix, register v[0 .. 4^W-1]
    // (2W qubits: ket field of wire j = bits [q_n j, q_n j + q_n), bra field = the same + W q_n)
    apply_word(v, 2W, ket_field(j), Udag_gatelist);  apply_word(v, 2W, bra_field(j), UT_gatelist);     // U^H rho U
    T = 0;
    for (i = 0; i < 4^W; ++i) { a = field(i, ket_field(j)); b = field(i, bra_field(j));
                                 v[i] *= Wgt[a][b];  if (a == b && rest_ket(i) == rest_bra(i)) T += re(v[i]); }   // (NU.4) layer + trace
    apply_word(v, 2W, ket_field(j), U_gatelist);     apply_word(v, 2W, bra_field(j), conjU_gatelist);  // back
    log_T_text += log(T);      // never divide (Sec. 4.5); every readout is a ratio, (14.19) divided by its trace
    // Kraus word (free K_k, d x d): rho' = sum_k K_k rho K_k^H : for each k apply K_k on ket, conj(K_k) on bra into an
    //   accumulator (second 4^W vector), or Stinespring: extend by q_e = ceil(log2 r) env qubits on BOTH sides in |0>,
    //   apply U_env (C10 ansatz on q_n + q_e qubits) on ket and conj(U_env) on bra, then
    //   rho'[b][b'] = sum_k v[(b + d k) + D (b' + d k)], D = d 2^{q_e}   -- strided reduction = the discard of (13.7)
    // negation (word level, before Kronecker-in (2.7)): rho[a][a'] = (a == a') - rho[a][a'];  T = d - 1
    // merge (coreference, quantum spider): rho[a][a'] *= sigma[a][a'];  classical merge: keep a == a' only

Pure-state shortcut (the important one): any SINGLE-Kraus update (phaser, Proj, B rho B, Mult with rank-1 sigma =
|v><v|, whose Kraus op is diag(v)) acts on the 2^W statevector of Sec. 14.1 as one dense non-unitary q_n-qubit gate K
on wire j, K = U diag(sqrt r) U^H (d x d, generated transiently), Z *= ||Psi||^2 carried as log Z. Working set
UNCHANGED from Sec. 14.1. Only fuzz (rank d) and rank-r Kraus words genuinely need mixedness: either branch (d or r
pure vectors, mixed only in the readout as in (14.18)) or move to the doubled register; branching beats doubling while
the branch count is below 2^W, i.e. for fewer than W/q_n fuzz updates per text.

(4) Kernel shape, flops (FMA-class, C9), working set (fp32 complex, 8 B).
 Word-level tile (rho d x d, dense S = U diag(sqrt r) U^H precomputed per word, cached d^2 complex): two d x d gemms
 = 8 d^3 FMA, working set 3 d^2 complex. (1,1): 64 FMA, 96 B. (2,1): 512 FMA, 384 B. Eigenbasis-butterfly form
 instead: 32 Ng d^2 FMA (384 at (1,1), Ng = 3) -- dense wins whenever d < 4 Ng, i.e. always at d <= 4. Kernel shape:
 small dense complex matmul (tensor-core / HMX tile only when batched over sentences; on HVX use the Schur layer,
 which is a lane-wise multiply, plus the existing butterflies).
 Text register, doubled: phaser 8 d 4^W FMA, fuzz 16 d 4^W + 2 4^W, Kraus rank r: 8 r d 4^W with 2 x 4^W working
 set. Working set 8 (4^W + d^2) B: W = 2 (two (1,1) nouns) 160 B (thread registers); W = 3: 544 B; W = 4 (two (2,1)
 nouns): 2.2 KB (warp Layout B); W = 6: 32 KB (SMEM); W = 7: 128 KB, the ceiling of the doubled path (Sec. 13.2's
 2^{2 N q_n} warning stands; use it only when fuzz / rank > 1 words are present). Per-target ceilings of the doubled
 register, 8 (4^W + d^2) B at d = 2: CUDA thread (~1 KB) W = 3 (544 B); HVX context (4 KB) W = 4 (2,080 B); VTCM
 8 KB slice W = 4 (W = 5 = 8,224 B exceeds by 32 B unless the d^2 gate tile is held in registers, then W = 5 exactly
 fills the slice); Adreno local 32 KB W = 5 (W = 6 = 32,800 B exceeds by 32 B, same remedy); CUDA SMEM W = 7
 (131,104 B, needs the > 48 KB dynamic shared-memory opt-in, cc 7.0+). At d = 4 the tile is 128 B instead of 32 B
 (8 (4^W + 16) B) and 4^W still dominates: the same W per tier.
 Text register, pure (single-Kraus): 4 d 2^W FMA, 2^W complex: identical to a unitary sentence gate, e.g. 32 FMA and
 32 B at W = 2 (1,1); 256 FMA, 128 B at W = 4 (2,1).
 Reduced-state path: apply the update to rho_q of (14.19) (d^2 complex) when the update is the LAST operation on that
 wire (readout-time adjectives, negation): 8 d^3 FMA, 3 d^2 complex; exact only there (an update followed by another
 gate on an entangled wire needs the joint state).

(5) Composition. Statevector (14.1)-(14.2): single-Kraus updates are extra gate-list entries G1F(field, dense K)
with a log-Z accumulator; commutation with unitary gates is NOT free (K is not diagonal), so text order is kept.
Doubled register: every gate G_t of (14.2) becomes the pair (G_t on ket fields, conj(G_t) on bra fields) -- a
14-qubit pure kernel runs a 7-qubit density-matrix text exactly; discard = strided reduction; readout = (14.19) with
the bra field read directly (no conj product), then divide by T. MPS (14.3): a single-Kraus update is a 1-site gate
A_i[alpha][s'][beta] = sum_s K[s'][s] A_i[alpha][s][beta], d^2 chi^2 MACs, NO SVD and no bond growth; fuzz / rank-r
words as a locally purified MPS: attach an environment leg e < r to site i, A_i[alpha][s'][e][beta] = sum_s
K_e[s'][s] A_i[alpha][s][beta] (memory of that site x r; (14.19) traces e as well; later 2-site gates on site i see
local dimension d r in (14.8)-(14.10)). Spider merge of two wires on the MPS is not a 1-site op (it identifies
indices); do it on the statevector as (11.7) doubled.

(6) Training. Parametrise sigma by its EIGENDECOMPOSITION, not its entries: U = C10 word unitary U_w(theta), r_a =
sigmoid(t_a) (keeps sigma <= I, hence T <= 1 and the update trace-non-increasing). No Jacobi eigensolve is ever
needed; sqrt(sigma) and fuzz are free. Gradients: the doubled register is a pure gate list plus one Schur layer, so
(11.30)/(5.6)-(5.7) run verbatim with (i) a LINEAR readout seed lambda_j = (1/2) vec(O - P I)[a][a'] / T for
P = tr(O rho')/T (replaces (5.8)), (ii) the Schur layer passed as lambda .*= Wgt with psi restored from a d^2 (or 4^W)
checkpoint, never by inverting Wgt (fuzz has zeros), (iii) the angle occurrence table for the scatter-add of Sec. 5.5,
verified in test 5a:

    ket U^dag: sign -1 all gates;   bra U^T: +1 (Ry: -1);   ket U: +1 all;   bra conj(U): -1 (Ry: +1)          (NU.5)
    dL/dWgt[a][a'] = 2 Re( conj(lambda_after[a,a']) x_before[a,a'] );  fuzz dL/dr_a = dL/dWgt[a][a];
    phaser dL/dr_a = dL/dWgt[a][a] + sum_{a' != a} dL/dWgt[a][a'] sqrt(r_a') / (2 sqrt(r_a))   (guard r_a > eps)   (NU.6)
    free Kraus K_k (quantum-inspired):  grad_K = 2 Lam K_k rho = (O - P I) K_k rho / T,  dL/dRe K = 2 Re grad, dL/dIm K = 2 Im grad   (NU.7)
    Stinespring K: all-angle model, (11.30) on the extended doubled register, partial trace is part of the linear readout.

Parameter-shift is not needed but would hold for the angles of U_env (the dilated form is a unitary circuit, C13
caveat for CRz/CRx); the quotient rule enters only through the (O - P I)/T seed.

(7) Minimal tests (exact; nu.c reproduces every digit).
 T1 d = 2, rho = |+><+|, sigma = diag(1, 1/4): fuzz -> diag(0.5, 0.125), T = 0.625, normalised diag(0.8, 0.2);
 phaser -> [[0.5, 0.25],[0.25, 0.125]], T = 0.625, normalised [[0.8, 0.4],[0.4, 0.2]] (pure); B rho B -> [[0.5,
 0.125],[0.125, 0.03125]], T = 0.53125; Proj -> rho (full support); Mult -> diag(0.5, 0.125) (= fuzz because sigma is
 computational-diagonal); neg -> |-><-| = [[0.5, -0.5],[-0.5, 0.5]]; merge(|+><+|, sigma/1.25) -> diag(0.4, 0.1),
 normalised diag(0.8, 0.2).
 T2 d = 2, rho = |0><0|, sigma = H diag(1, 1/4) H = [[5/8, 3/8],[3/8, 5/8]]: fuzz -> [[0.3125, 0.1875],[0.1875,
 0.3125]], normalised [[0.5, 0.3],[0.3, 0.5]]; phaser -> [[0.5625, 0.1875],[0.1875, 0.0625]], normalised [[0.9,
 0.3],[0.3, 0.1]] (det 0: pure).
 T3 two (1,1) wires, Psi = (|00> + |11>)/sqrt2, update wire 0 with sigma = diag(1, 1/4): both give T = tr(sigma rho_0)
 = 0.625 and marginals rho_0 = rho_1 = diag(0.8, 0.2); joint <00|rho|11> = 0.4 (phaser) vs 0 (fuzz) -- the difference
 is invisible in single-wire readouts (14.19) and visible in the two-wire readout (14.19').
 T4 Stinespring amplitude damping (K_0 = diag(1, 0.8), K_1 = [[0, 0.6],[0, 0]], U4 rows b + 2k, cols a + 2e =
 [[1,0,0,0],[0,.8,-.6,0],[0,.6,.8,0],[0,0,0,1]]) on |+><+| via the 4-qubit doubled register and strided trace:
 [[0.68, 0.4],[0.4, 0.32]].
 T5a phaser with U = Rx(t2) Rz(t1) Rx(t0), theta = (0.3, -1.1, 0.7), r = (1, 0.25), rho = [[0.7, 0.2-0.1i],[0.2+0.1i,
 0.3]], P = rho'[0][0]/T = 0.8520079519; adjoint dP/dtheta = (-0.0678584686, -0.0428192659, -0.1541439754), central
 differences agree to 7e-11. T5b (NU.7) on two random K_k: 16 real derivatives agree to 1.1e-10.

(8) Caveats / UNVERIFIED. (a) Exact fuzz/phaser/Proj definitions and the Lewis KMult/BMult naming: from memory,
arXiv unreachable. (b) Negation is defined here at word level or on a reduced state; a channel-form negation of one
wire of an ENTANGLED text is not in the literature and none is proposed here (I - rho is not linear-CP; the best CP
surrogate at d = 2 is the Buzek-Hillery (2I - rho)/3, a different semantics). (c) Compounding T over a text amplifies
fp16/Q1.15 error by 1/sqrt(prod T) exactly as Sec. 13.2's postselected form: fp32 accumulate and carry log T; with
r_a = sigmoid the per-update T >= min_a r_a tr(P_a rho) can still be tiny -- keep the Z_min guard of (5.1). (d)
Whether Sim14/Sim15 at small L reach the eigenbasis needed for a given sigma is the same UNVERIFIED capacity question
as Sec. 13.2(c). (e) The Stinespring gradient path (angles of U_env with the strided trace as readout) reuses the
verified mechanism of T5a but was not itself run. (f) The HMX/tensor-core mapping of d = 2..4 gemms requires batching
over sentences (tiles are >= 16x16 UNVERIFIED for HMX); on HVX the Schur-layer form is the native one. (g) All member
votes rate the semantics "partially/already done"; the Tier-A claim is upheld only for the in-circuit, trained,
register-resident form and for negation.

### 2.2 Theme A2. Spectral functionals of tiny Hermitian density matrices: von Neumann / Renyi entropy, purity, Loewner order and k-hyponymy, Uhlmann fidelity, trace distance, relative entropy

Members: von Neumann entropy, Renyi entropies and purity of word/sentence density matrices as ambiguity measures |
fidelity and trace distance between mixed word/noun states for WSD and DisCoCirc noun comparison | density-matrix
semantics with exact Loewner order, k-hyponymy, entropies and divergences | ambiguity as mixed states with exact
von Neumann entropy and fidelity | graded entailment / hyponymy benchmarks with the Loewner order. Tier A
(hardware-limited), with the historian qualification in (2). Obeys C1-C13; reference program:
scratchpad/spec/spec_test.c (fp64), scratchpad/jacobi.c (fp32 Jacobi sweep counts). All reference numbers verified
(closed forms agree with the eigen path to 1e-8, the adjoint rule agrees with finite differences to 2e-10, the
analytic Loewner case is exact).

(1) Quantum-theoretic definition. The CPM / doubling construction (Piedeleu, Kartsaklis, Coecke, Sadrzadeh 2015,
arXiv:1502.00831) sends a word wire of dimension d = 2^q to positive operators rho on C^d; composition is the DisCoCat
tensordot on doubled indices, already specified as (13.10) / Sec. 1.6. Mixedness arises two ways: (a) as a MIXTURE,
rho = sum_k p_k |w_k><w_k| (lexical ambiguity, K senses; syntactic ambiguity as a distribution over parses, Eisinger,
Gauderis, de Huybrecht, Wiggins 2025, arXiv:2504.00040 -- VUB, not Quantinuum; the earlier attribution "Krawchuk et
al." is wrong); (b) as a REDUCED state, rho_q = Tr_rest |Psi><Psi|, the DisCoCirc noun wire after a text (C17,
(14.19)), or the sentence wire with the cup qubits traced instead of postselected (Sec. 1.5). The functionals the
programme defined on these operators are all spectral or spectral-pair functionals: S(rho) = -Tr rho log rho
(ambiguity; disambiguation = entropy drop after composition), S_alpha = log(Tr rho^alpha)/(1 - alpha), purity
Tr rho^2; graded hyponymy A <=_k B iff B - k A >= 0 (Loewner order; Bankova, Coecke, Lewis, Marsden 2016/2019,
arXiv:1601.04908; Lewis 2019 RANLP), relative entropy S(A||B) = Tr A(log A - log B) for entailment (Balkir,
Sadrzadeh, Coecke 2016, arXiv:1506.06534); Uhlmann fidelity and trace distance for "Alice before vs Alice after" /
word-sense comparison (Coecke 2020 arXiv:1904.03478; Wang-Mascianica, Liu, Coecke 2023 arXiv:2301.10595). Convention
fixed here: F_root(rho, sigma) = Tr sqrt(sqrt(rho) sigma sqrt(rho)); F_sq = F_root^2. The spec's Sec. 1.5 closed form
Tr(rho1 rho2) + 2 sqrt(det rho1 det rho2) and addendum 14.6's "(Tr sqrt(...))^2" are both F_sq; Bures angle =
arccos(F_root), Bures distance^2 = 2(1 - F_root), Fuchs-van de Graaf 1 - F_root <= T <= sqrt(1 - F_sq).

(2) Why undone -- honest version. Three verifiers per member agree on the physics: none of these is a linear
observable. Von Neumann entropy has no shot-efficient estimator (poly(d) copies, Acharya et al. arXiv:1711.00814,
UNVERIFIED id); Renyi-2 / purity need two copies + controlled-SWAP or randomized measurements (Brydges et al. 2019,
shot-estimable at k <= 2 but biased); the PSD test, k_max, Uhlmann F and the trace norm need the full spectrum, i.e.
tomography (3^k Pauli settings, only 3-9 at k = 1-2, but every quantity is a nonlinear function of shot-noisy
estimates and two-copy circuits give only bounds, Miszczak et al. 2009). So they never entered the circuit-QNLP
pipeline (Meichanetzidis 2020, Lorenz 2021, lambeq, Duneau 2024 -- all pure-state readouts). BUT the historian and
physicist votes are right that the science was not blocked: the density-matrix strand was born classical
(Blacoe-Kashefi-Lapata 2013, Sordoni 2013, Piedeleu 2015, Meyer & Lewis 2020 PyTorch) and stayed classical, on count-
or neural-derived rho, small d. What is genuinely undone is: these functionals on CIRCUIT-generated states, where
mixedness comes from ENTANGLEMENT with other wires (DisCoCirc partial trace, traced cups) rather than from a
hand-built mixture, computed exactly inside the same register-resident kernel, trained end to end with exact
gradients. That is what the maths below provides; the (13.10) density composition and the K-run mixture form
(13.10') are cited, not re-derived.

(3) Exact classical formulation. Objects: rho is d x d Hermitian PSD, d = 2^k (k = q_n for a noun wire, q_s for the
sentence wire, 2 q_n for a two-wire relational readout). Hermitian packing: d reals on the diagonal + d(d-1)/2
complex strictly-lower = d^2 reals.

  Partial trace (14.19): rho[a][a'] = sum_e Psi[scatter(e) | (a << f)] conj(Psi[scatter(e) | (a' << f)])  = P P^H
     with P the d x E matrix P[a][e] = Psi[scatter(e) | (a << f)], E = 2^{W-k}, f the C7 field start.   (SF.1)
  Eigendecomposition: rho = V diag(lam) V^H, lam in [0, Tr rho]. d = 2 closed form (Tr rho = t):
     r = sqrt( (rho00 - rho11)^2 + 4 |rho10|^2 ),  lam = (t +- r)/2,  Bloch (r_x, r_y, r_z) = (2 Re rho10, 2 Im rho10, rho00 - rho11)  (SF.2)
  d = 4, 8: cyclic complex Jacobi (scratchpad/jacobi.c, spec_test.c): for each pair p < q: ph = A[p][q]/|A[p][q]|,
     th = 0.5 atan2(2|A[p][q]|, A[q][q] - A[p][p]), c = cos th, s = sin th; columns k: A[k][p]' = c A[k][p] - s conj(ph) A[k][q],
     A[k][q]' = s ph A[k][p] + c A[k][q]; rows likewise with ph <-> conj(ph); accumulate V by the column rule only when
     eigenvectors are needed. Each rotation is the C6 1-qubit butterfly on rows/columns (p, q) with a phase.          (SF.3)
  Eigenvalue-only functionals (no V):
     S = -sum_l lam_l log lam_l (skip lam_l <= 0);  S_alpha = log(sum lam_l^alpha)/(1-alpha);  purity = sum |rho_ij|^2 (no eigensolve);
     T(rho, sigma) = 1/2 sum_l |mu_l|, mu = eig(rho - sigma);  Loewner test B >= k A  iff  min eig(B - k A) >= 0.        (SF.4)
  Eigenvector functionals:
     g(rho) = V diag(g(lam)) V^H (sqrt, log, x^{-1/2}) -- two d x d GEMMs;
     F_root = sum_l sqrt(eig_l( sqrt(rho) sigma sqrt(rho) ));  d = 2: F_sq = Tr(rho sigma) + 2 sqrt(det rho det sigma);
     S(A||B) = sum_l la_l log la_l - sum_j log(mu_j) <v_j|A|v_j>  (= +inf if <v_j|A|v_j> > 0 for some mu_j = 0);
     k_max(A <= B) = 1 / lam_max( B^{-1/2} A B^{-1/2} ) on supp(B), = 0 if supp(A) is not in supp(B);
       d = 2, B > 0: k_max = 2 / ( t + sqrt(t^2 - 4 det A / det B) ),  t = (Tr A Tr B - Tr(AB)) / det B  (no eigensolve);
       cheap linear proxy (addendum a1630): Tr(AB)/Tr(A^2).                                                              (SF.5)
  Mutual information between noun wires q, q' (new; uses (14.19) and (14.19')): I(q:q') = S(rho_q) + S(rho_q') - S(rho_qq'),
     bounded by 2 S_cut <= 2 log2 chi under the MPS (14.15)-(14.16); the Schmidt entropy S_cut itself is free from the
     (14.10) singular values.                                                                                            (SF.6)

C-like kernel for one noun wire (d = 2, fp32 accumulate, register-resident):
  // in: psi[re], psi[im] of 2^W; f = field start; out: S, purity, lam[2]
  float r00 = 0, r11 = 0, r10re = 0, r10im = 0;
  for (e = 0; e < (1 << (W-1)); ++e) { i0 = scatter1(e, f); i1 = i0 | (1 << f);
      r00 += re[i0]*re[i0] + im[i0]*im[i0];  r11 += re[i1]*re[i1] + im[i1]*im[i1];
      r10re += re[i1]*re[i0] + im[i1]*im[i0];  r10im += im[i1]*re[i0] - re[i1]*im[i0]; }   // rho10 = psi1 conj(psi0)
  t = r00 + r11;  r = sqrtf((r00 - r11)*(r00 - r11) + 4*(r10re*r10re + r10im*r10im));
  lam[0] = (t + r)/(2*t); lam[1] = (t - r)/(2*t);  purity = (r00*r00 + r11*r11 + 2*(r10re*r10re + r10im*r10im))/(t*t);
  S = 0; for (l = 0; l < 2; ++l) if (lam[l] > 1e-7f) S -= lam[l]*logf(lam[l]);
Divide by t = Tr rho (Sec. 4.5: every output is a ratio) whenever truncations or postselections left the state
unnormalised.

(4) Kernel shape, flops, working set. Partial trace = rank-E Hermitian outer-product accumulate (GEMM P P^H, d x E
times E x d): 2^{W-k} d(d+1)/2 complex MACs = 2 x 2^{W-k} d(d+1) real FMA; a reduction over e (warp shuffle / vror
tree, fp32). Eigen: d = 2 closed form ~14 flops + 1 sqrt + 2 log; d = 4 Jacobi: 6 pairs x (24 d + 24 d) = 1,152 real
flops per sweep without V, 1,728 with V, plus 6 atan2/sincos on the scalar core; 3 sweeps at fp32 tol 1e-14
(jacobi.c: d = 4: 3, d = 8: 4, d = 16: 5; spec_test.c 4x4: 3-4). sqrt/log/inv-sqrt functionals: two d x d complex
GEMMs = 8 d^3 real FMA each (64 at d = 2, 512 at d = 4). Working set (fp32): (1,1) noun wire k = 1: rho packed 16 B,
eigen scratch 16 B, F/T/k with a second rho: 48 B; two-wire d = 4: 64 B packed, dense 128 B, V 128 B, Jacobi in
place: <= 400 B. (2,1) noun wire k = 2: as the two-wire (1,1) case, <= 400 B; two wires at (2,1) d = 16: 2 KB dense +
2 KB V + scratch ~ 6 KB (VTCM / SMEM tier, 5 sweeps x 92,160 real flops eigenvalues-only (138,240 with V), i.e. 0.46 /
0.69 MFLOP per two-wire (2,1) readout, plus 120 atan2/sincos pairs per sweep on the scalar core; 120 pairs x 48 d
real flops per pair at d = 16, the same per-pair rule as the d = 4 line above). Sentence wire q_s = 1: identical to k = 1.
Everything at (1,1) and (2,1) single-wire is REGISTER tier on one CUDA thread or one HVX context; there is no HBM
traffic beyond the state already resident.

(5) Composition. Statevector: (SF.1) reads the live register after the last gate (Sec. 2 kernel or the Sec. 1.4
bent-wire form); for the sentence wire it replaces the Pi_K postselection by a trace over K (Sec. 1.5), giving a
mixed sentence meaning at zero extra amplitudes. Tree / fused tensordot (Sec. 1.3, 11.4): the survivor field is pure
by construction, so mixedness must enter via (13.10) / (13.10') (K^{n_w} pure runs, then rho = sum_i w_i sigma^{(i)}
sigma^{(i)H}, d^2 complex accumulator) -- entropy of that rho measures residual ambiguity after composition, the 2015
experiment on circuit states. DisCoCirc exact: (14.19) then (SF.2)-(SF.6); MPS: rho_q = d^2 chi^2 MACs from the
centre site, rho_qq' by (14.19'), S_cut free; mean-field variant of 13.2(a) already stores rho_j and needs only
(SF.2)-(SF.5). Ambiguous coreference (14.18): mix the readout matrices, then apply the functionals once.

(6) Training rule. Every functional here is a trace function or a composition of them, so its Frechet derivative is
a d x d Hermitian F with df = Tr(F drho): S: F = -(log rho + I); purity: F = 2 rho; S_alpha: F = alpha rho^{alpha-1}
/ ((1-alpha) Tr rho^alpha); T: F = 1/2 sgn(rho - sigma) (non-differentiable at mu = 0; use a smoothed |mu|); F_root
wrt rho: F = 1/2 sigma^{1/2} M^{-1/2} sigma^{1/2}, M = sigma^{1/2} rho sigma^{1/2} (pseudo-inverse on supp M);
S(A||B) wrt A: log A - log B + I; wrt B: in B's eigenbasis F~[i][j] = -A~[i][j] L(mu_i, mu_j), L = (log mu_i - log
mu_j)/(mu_i - mu_j), = 1/mu_i on the diagonal (Daleckii-Krein), rotated back by V; k_max: dk/dA, dk/dB from the top
eigenvector w of B^{-1/2} A B^{-1/2} (Hellmann-Feynman; a loss should use k_max only through a smooth surrogate, or
the linear proxy). Normalisation rho_n = rho/t: F_eff = (F - Tr(F rho_n) I)/t -- the quotient rule of Sec. 5.1 in
matrix form. Then, since rho = P P^H, the (11.27) cotangent of the state is one d x d by d x E GEMM:
     lambda_Psi[scatter(e) | (a << f)] = sum_a' F_eff[a][a'] Psi[scatter(e) | (a' << f)]                          (SF.7)
and it is seeded as B in the Sec. 5.3 adjoint (circuit form) or as lambda_sigma / lambda_r in (11.28)-(11.29) (tree
form); for (13.10') the weights w_i also receive gradients through the rank-1 sum. Verified: (SF.7) with F = -(log
rho_n + I) against central differences on all 8 real components of a W = 2 state: max deviation 2.0e-10. Parameter
shift: each rho entry is <Psi|E_ij|Psi>, so Sec. 5.2's two/four-term rules give drho/dtheta entry-wise, then df =
Tr(F_eff drho/dtheta); hardware-realisable only as tomography, never used on the simulator.

(7) Minimal test (fp64 reference; fp32 tolerance 1e-6 on all values). T-SF1, Sec. 8 state, W = 3: rho_s = Tr_{q0,q2}
= [[0.5, 0.40523146 - 0.17132911 i], [conj, 0.5]]; lam = 0.93996159 / 0.06003841; S = 0.22707320 nat (0.32759738
bit); Renyi-2 = 0.11976104; purity = 0.88713240 (Frobenius = eigen path); F_sq(rho_s, |sigma><sigma|/Z) = 0.93359620;
T = 0.09984988. T-SF2, DisCoCirc d = 2, two wires: Alice (Sec. 8 angles) on qubit 0, Bob on qubit 1, gate H(0) H(1)
CRz(0.8)(0->1): Psi = [0.02969547 - 0.49107324 i, 0.33980462 - 0.22169693 i, 0.58809952 - 0.08540425 i, -0.00848436 +
0.49002208 i]; rho_A = [[0.59518967, 0.07212047 - 0.44774271 i], [conj, 0.40481033]]; lam = 0.96339612 / 0.03660388
(rho_B same spectrum); S = 0.15699665 nat; purity 0.92947193; <Alice|rho_A|Alice> = 0.10869068 ("Alice changed");
F_root(rho_A, rho_B) = 0.97417081 (closed form = eigen path); Bures angle 0.22777690; T = 0.22581240 (= the upper FvdG
bound: equal-spectrum qubits); S(A||B) = 0.17992942; k_max(A <= B) = 0.31988774 with min eig(B - k A) = 5.6e-17
(closed form identical); linear proxy 0.94513956. T-SF3 analytic: A = |0><0|, B = I/2: S(A) = 0, S(B) = ln 2, T =
0.5, F_root = sqrt(0.5), S(A||B) = ln 2, S(B||A) = inf, k_max(A <= B) = 0.5, k_max(B <= A) = 0 (support failure).
T-SF4: jacobi.c d = 4/8/16 fp32: 3/4/5 sweeps, |dS| <= 9e-8 vs fp64. T-SF5, the (2,1) reference NUMBER for the d = 4
path ((SF.1) rank-4 outer product, (SF.3) 4x4 Jacobi, (SF.4) S / purity, (SF.6) MI): the T-W4 state of Sec. 2.4 (7)
(q_n = 2, W = 4, IQP ansatz 1; rho_Alice printed there): eig = 0.97766824, 0.02233176, 0, 0; S = 0.15433977 bit;
purity = 0.95633390; I(Alice:Bob) = 0.30867954 bit; the two exact zeros exercise the clamp of (8)(iii). Working set
= the (2,1) single-wire column of (4) (rho 64 B packed, Jacobi in place).

(8) Caveats and UNVERIFIED. (i) The symmetry S(A||B) = S(B||A) and k_max(A<=B) = k_max(B<=A) in T-SF2 is an artefact
of equal-spectrum 2x2 states (unistochastic 2x2 overlap matrices are symmetric), not a property. (ii) Entropy of a
reduced circuit state measures ENTANGLEMENT with the other wires, not lexical ambiguity; only (13.10') mixtures
reproduce the 2015 experiment. Whether the C10 ansatze reach arbitrary rho eigenvectors is UNVERIFIED (addendum
Sec. 19). (iii) Tiny eigenvalues (T-SF4's 4x4 case has lam_min = 7e-7) make S ill-conditioned in fp32 near zero:
clamp lam <= 1e-7 f, never compute in fp16. (iv) k_max is discontinuous at support changes; Loewner comparisons need
rank-consistent operators (Bankova et al. use Tr <= 1 positive operators, not normalised states). (v) Literature
checks were from memory (network blocked for the verifiers): Acharya et al. id, Lewis 2019 k_BA form, and whether
lambeq exposes density-matrix training (mixed machinery lives in DisCoPy CQMap) are UNVERIFIED. (vi) HVX has no
vector atan2/sqrt: Jacobi angles on the scalar core, d <= 4 per thread; d = 16 relational readout at (2,1) is a
VTCM/SMEM object.

### 2.3 Theme A3. Exact pure-state overlaps and Gram matrices as reductions/GEMMs: similarity, retrieval, question answering, contrastive objectives, vocabulary slot scoring

Members: exact inner products, fidelities, trace distances and relative entropies between sentence/text meanings
(similarity and QA as the model-native primitive) | amplitude-level and inner-product objectives: exact fidelity,
contrastive (InfoNCE) sentence similarity, entropy regularisers | text similarity and semantic search over
circuit-valued meanings (swap test replaced by exact inner products) | generative QNLP: sampling and scoring
sentences/texts from circuit semantics. Tier A (hardware-limited), with the honest narrowing in (2). Conventions
C1-C17, errata E-1..E-13 obeyed; (S.n) labels are the baseline's / addendum's, (G.n) are new. Reference program:
scratchpad/gram/gram.c (S0 reproduces Sec. 8 exactly, Z = 0.14026553; analytic cotangent matches central differences
to 2.9e-10).

(1) Quantum-theoretic definition. DisCoCat: sentence meaning = state sigma in C^{2^{q_s}} obtained by the functor F:
pregroup -> FdHilb, sigma = (cups) o ((x)_w phi_w) (Sec. 1.2, (1.5)). Similarity = the Hilbert inner product
<sigma_1|sigma_2> (Grefenstette-Sadrzadeh 2011 evaluate cosine of exactly this object; Zeng-Coecke 2016 make
closest-vector on it the quantum speedup target). Normalised fidelity F = |<psi_1|psi_2>|^2, psi = sigma/sqrt(Z), Z =
<sigma|sigma> (Sec. 1.5). QDisCoCirc (Laakkonen-Meichanetzidis-Coecke 2024, arXiv:2408.06061, confirmed via abstract):
text similarity = overlap of text states on N q_n noun wires; QA = the same overlap restricted to the queried wires,
i.e. <c|rho_q|c> with rho_q = Tr_{rest} |Psi><Psi| ((14.19), Sec. 14.6); shown BQP-hard (verifiers add "and in BQP,
hence complete": UNVERIFIED). Contrastive objective: a batch {psi_a} with positive pairs (a, a+) defines InfoNCE
L = -sum_a ln( exp(F_{a a+}/T) / sum_{b != a} exp(F_ab/T) ). Slot scoring / generation: fix all words but one
type-position of width q_slot; by multilinearity of the tensordot the map from the open word to the sentence is a
LINEAR map M: C^{2^{q_slot}} -> C^{2^{q_s}} (Sec. 1.3's partial R with the slot factor left open); score of candidate
w against target tau is |<tau| M w>|^2 / <w|M^H M|w> (Sec. 14.7 fill-the-slot, made explicit). Density-matrix versions
(Tr rho sigma, Uhlmann F, trace distance, relative entropy) are Sec. 13.2 (13.10), Sec. 14.6 and Sec. 1.5 and
Theme A2; NOT re-derived here.

(2) Why undone -- honest. The three verifier lenses agree the "QNLP never did similarity" framing is wrong for the
CLASSICAL lineage: cosine sentence similarity IS the founding experiment (Grefenstette-Sadrzadeh 2011, EMNLP D11-1129;
Kartsaklis-Sadrzadeh 2013; Milajevs 2014) and density-matrix fidelity/entropy/Loewner entailment was computed
classically (Piedeleu 2015; Balkir-Sadrzadeh-Coecke 2016 for von Neumann / relative entropy; Bankova-Coecke-Lewis-
Marsden and Lewis 2019 for Loewner k-hyponymy -- attribution per verifier corrections). What IS hardware-blocked, and
undone for circuit-parameterised (angle) models: (a) the overlap as a TRAINING OBJECTIVE -- lambeq's hardware path
outputs measured probabilities only; no hardware QNLP paper trains a fidelity, ranking or InfoNCE loss, because every
pair (a,b) needs its own compute-uncompute test (U_b^dag U_a|0>, 2x depth, ancilla-free, |G|^2 only), swap test (2n+1
qubits) or Hadamard test (controlled preparation, gives Re/Im G) at O(1/eps^2) shots, so a B-sentence batch costs
O(B^2/eps^2) circuit runs per gradient step, and every compute-uncompute / swap-test run is itself postselected on
the cups of BOTH sentences, so the accepted fraction is P_post,a P_post,b with E[P_post] = 2^{-4 q_n} each (E-1:
Z 2^{-2 q_n} per N TV N sentence): 2^{-8 q_n} per pair at init, 1/256 at (1,1), 1/65536 at (2,1); (b) QA in
QDisCoCirc is DEFINED as an overlap but the H1-1 experiment
(Duneau et al. 2024, arXiv:2409.08777, confirmed) reads a single noun qubit's Z-expectation, and Lorenz et al. 2021
read one postselected wire; (c) vocabulary-wide slot scoring is O(V x shots) circuits per step (Karamlou et al. 2022
therefore annealed over a tiny vocabulary). Kernel-engineer correction adopted: phase IS accessible on hardware via
the Hadamard test; what is lost is only the shot-cheap route. Novelty here is therefore "partially-done": the maths
is old; the register-resident batched Gram/GEMV form and its exact gradient into angles are what the simulator adds.

(3) Exact classical formulation. Dimensions: D = 2^{q_s} (sentence wire; D = 2 at both (1,1) and (2,1)), or D =
2^{N q_n} for text states; B sentences; Psi is B x D complex, row a = psi_a (C3 split planes: Psi_re, Psi_im,
row-major).

    Z_a      = sum_k re_a[k]^2 + im_a[k]^2;   psi_a = sigma_a / sqrt(Z_a)                                   (G.1)
    G_ab     = sum_k conj(psi_a[k]) psi_b[k]  = (Psi Psi^H)[a][b]   (Hermitian, G_aa = 1)                   (G.2)
    F_ab     = |G_ab|^2;  Td_ab = sqrt(1 - F_ab) (pure trace distance);  ang_ab = acos(sqrt F_ab) (Bures)   (G.3)
    InfoNCE: L = -sum_a [ F_{a a+}/T - ln sum_{b != a} exp(F_ab/T) ]                                        (G.4)
    slot:    M[s][x] = tensordot over all words except the slot, slot factor x open (Sec. 1.3 partial R)    (G.5)
             u = M^H tau (D_slot),  P = M^H M (D_slot x D_slot Hermitian),
             raw_w = |u^H w|^2,  fid_w = raw_w / (w^H P w),  p_w = softmax(beta fid_w) or raw_w / sum raw    (G.6)

    // Gram + InfoNCE forward, one thread block per batch (C-like, fp32 accumulate, C9)
    for a in 0..B-1: Z[a] = dot(sig[a], sig[a]);  rs[a] = rsqrt(Z[a]);
    for a in 0..B-1: for b in a..B-1:            // Hermitian: B(B+1)/2 dots of length D
        gr = 0; gi = 0;
        for k in 0..D-1: { gr += re[a][k]*re[b][k] + im[a][k]*im[b][k];  gi += re[a][k]*im[b][k] - im[a][k]*re[b][k]; }
        gr *= rs[a]*rs[b]; gi *= rs[a]*rs[b];  G[a][b] = (gr, gi); G[b][a] = (gr, -gi);  F[a][b] = F[b][a] = gr*gr + gi*gi;
    for a: m = max_{b!=a} F[a][b]/T;  S = sum_{b!=a} exp(F[a][b]/T - m);  L += -F[a][pos[a]]/T + m + ln S;   // stable logsumexp
    // Slot scoring for a vocabulary table Wt (V x D_slot):  two GEMVs + one quadratic form per row
    u = M^H tau;  P = M^H M;
    for w in 0..V-1: { ov = sum_x conj(u[x]) Wt[w][x];  nrm = sum_{x,y} conj(Wt[w][x]) P[x][y] Wt[w][y];  fid[w] = |ov|^2 / nrm; }

Index conventions: k is the C1 little-endian sentence-wire index; for text states k = sum_j s_j 2^{q_n j} (C17). For
DisCoCirc QA use (14.19)/(14.19') and F = <c|rho_q|c>; nothing new.

(4) Kernel shape, flops, working set. Gram (G.2): B(B+1)/2 complex dots of length D = a small Hermitian GEMM
Psi Psi^H, 4 D real FMA per entry; 2 B(B+1) D FMA total. At D = 2 (both (1,1) and (2,1)): B = 32 -> 4,224 FMA; B =
128 -> 66 K FMA. Working set: Psi 8 B D + G 8 B^2 (or F 4 B^2 only, if the phase is not needed) + Z, rs 8 B: B = 32,
D = 2: 512 + 8,192 (F: 4,096) B; B = 128: 2 KB + 128 KB G (64 KB F) -- G is the object that grows, so tile it
(b-blocks of 32) and keep only the per-anchor running logsumexp (m, S) + F_{a a+}: 12 B per anchor, making the whole
loss streamable with Psi resident (2 KB) -- REG/VTCM tier (C14). Text-state overlaps, D = 2^{N q_n} up to 2^14: one
dot = 4 D FMA = 64 K FMA; layout B (warp per state pair, warp-shuffle reduce) or HVX vector reduce over 1024-bit
lanes; retrieval against M stored texts = M x D by D GEMV, 8 D B per stored text (128 KB at D = 2^14 -- HBM/VTCM
streaming, not register). Slot scoring (G.6): M is D_s x D_slot = 2x2 at (1,1) (4 complex, 32 B), 2x4 at (2,1) (8
complex, 64 B); u, P: 2 + 4 complex / 4 + 16 complex; per candidate 4 D_slot + 4 D_slot^2 FMA = 24 / 80 FMA; V =
1e4: 240 K / 800 K FMA, i.e. ~2-4 us on one SM at 64-128 FMA/clk; the vocabulary rows are either the stored Sec. 1.6
vector table (V x D_slot x 8 B = 160 / 320 KB, streamed) or generated per candidate from angles (2^{q_slot}
amplitudes, 3-6 gates; +~40-100 FMA, zero table). Everything except the table is register-resident.

(5) Composition with the register-resident evaluator. sigma_a comes out of the fused tensordot (Sec. 1.3, peak
14-16 / 44-48 amplitudes) or the tree (11.18'); the Gram kernel consumes only the D = 2^{q_s} outputs: memory per
sentence in the batch is 8 D B (16 B) -- the batch, not the sentence, is what occupies memory. The slot matrix M is
the tensordot's own intermediate: for "Alice loves [ ]", M[s][b] = R[s,b] = sum_a Alice[a] V[a + 2^{q_n} s +
2^{q_n+q_s} b], i.e. stop the Sec. 1.3 sweep one word early. For an open subject use the a-field; for the verb slot M
is D_s x 2^{Q_V} (the audit matrix A_w of (15.1)). Text states / MPS: <c|rho_q|c> via (14.19) or the MPS transfer form
(14.19'); a full text-text overlap on the MPS is the standard transfer-matrix contraction, N d chi^3 MACs, no 2^W
vector (UNVERIFIED that it is worth adding at N <= 8; the dense dot is cheaper there).

(6) Training rule. Closed-form cotangent (Wirtinger, convention (11.27): lambda_x = dL/dconj(x)), verified by
central differences to 2.9e-10 (gram.c):

    dL/dF_ab from (G.4):  dL/dF_{a a+} += -1/T;  dL/dF_ab += p_ab/T,  p_ab = exp(F_ab/T) / sum_{b'!=a} exp(F_ab'/T)   (G.7)
    lambda_psi_a[k] += (dL/dF_ab) conj(G_ab) psi_b[k];   lambda_psi_b[k] += (dL/dF_ab) G_ab psi_a[k]                  (G.8)
    normalisation:  lambda_sigma_a = ( lambda_psi_a - psi_a Re<psi_a|lambda_psi_a> ) / sqrt(Z_a)                        (G.9)

(G.8) is one more Hermitian GEMM: Lambda_Psi = (dL/dF .* conj(G)) Psi, B x B by B x D. (G.9) REPLACES the root rule
(11.27') (which is the classification special case); then (11.28)-(11.30) run per sentence unchanged, so the batch
coupling is confined to the Gram step and the backward stays 3-4 forward passes per sentence at Sec. 11.7's memory
(11.31). Slot loss (G.6) is the same chain with tau, M and w as operands: lambda_u += (dfid/draw) ov conj(w)...,
lambda_M = tau lambda_u^H + ...; all bilinear, all (11.27). Parameter-shift alternative (hardware form only): F_ab
is a QUOTIENT of four projector-free bilinears; Re G_raw, Im G_raw, Z_a, Z_b are each cos/sin-linear in one angle,
so (5.2) applies to each (four-term (5.4) for CRz/CRx) and the quotient rule (5.3) combines them; 2P+1 passes per
pair, never on the ratio.

(7) Minimal test (W = 3 sentences, D = 2, fp64 reference; program gram.c). Words as Sec. 8: Alice (0.3, 0.7, 1.1),
Bob (0.5, -0.4, 0.9), loves (0.8, -0.6); add hates (-1.2, 0.4), IQP ansatz 0. Sentences S0 "Alice loves Bob", S1 "Bob
loves Alice", S2 "Alice hates Bob".
    sigma_0 = [-0.02640234 - 0.28465252 i, 0.08196341 - 0.22764750 i]   Z_0 = 0.14026553  (= Sec. 8)
    sigma_1 = [-0.01927012 - 0.25903817 i, 0.09018148 - 0.29128266 i]   Z_1 = 0.16045040
    sigma_2 = [ 0.12750899 - 0.39514143 i, -0.06050665 - 0.29885989 i]  Z_2 = 0.26537358
    <sigma_0|sigma_1> = 0.14794600 - 0.00199101 i;  <sigma_0|sigma_2> = 0.17218684 + 0.00845865 i;  <sigma_1|sigma_2> = 0.18149572 - 0.00393205 i
    G_01 = 0.98618261 - 0.01327170 i;  G_02 = 0.89247416 + 0.04384265 i;  G_12 = 0.87956341 - 0.01905545 i
    F_01 = 0.97273229 (Td 0.16512938);  F_02 = 0.79843230 (Td 0.44896291);  F_12 = 0.77399490 (Td 0.47539994)
    InfoNCE, anchor 0, positive 1, negative 2, T = 0.5:  L = 0.53396114,  p(pos) = 0.58627803
    lambda_sigma_0 = [0.47246644 - 0.41869576 i, -0.71537034 + 0.21117907 i];  lambda_sigma_1 = [0.06778313 + 0.24696691 i, 0.02321861 - 0.21692385 i];
    lambda_sigma_2 = [-0.36497333 - 0.11078877 i, 0.50716421 - 0.11191516 i]   (2 Re / 2 Im = dL/dRe sigma, dL/dIm sigma; FD deviation <= 2.9e-10)
    dL/dtheta (central differences h = 1e-5; Alice t0 t1 t2, Bob t0 t1 t2, loves t0 t1, hates t0 t1):
      -0.1968229  0.0652650 -0.2864580  0.2972581 -0.0116222  0.2655083 -0.1289842 -0.0474569  0.1203810 -0.0240814
    Slot test: context Alice + loves, object open, target tau = psi_1:
      M = [[0.12721606 - 0.29034520 i, 0.12721606 - 0.29034520 i], [0.37739763 - 0.22566461 i, 0.18405988 - 0.39934377 i]]
      u = M^H tau = [0.43070792 - 0.31986865 i, 0.51347711 - 0.14017495 i];  P = M^H M: P_00 = P_11 = 0.29383775, P_01 = 0.26006578 - 0.10917559 i
      cand Alice: raw 0.23764531, ||Mw||^2 0.23920329, fid 0.99348680;  cand Bob: raw 0.13644081, ||Mw||^2 0.14026553 (= Z_0), fid 0.97273229 (= F_10 to 2e-16)
      softmax(beta = 4): [0.52074260, 0.47925740]
Pass criteria: fp32 kernel within 1e-6 of every number; adjoint into angles within 1e-6 of the FD row; fid(Bob) ==
F_10 bitwise-ish (the slot path and the Gram path must agree).

(8) Caveats / UNVERIFIED.
- Phase of G_ab is gauge-dependent (each sentence's global phase); only |G| is meaningful unless both states share the
  |0> reference, which the simulator does provide (needed for coherent sums such as (11.17') or sentence
  superpositions); do not train on arg G.
- At D = 2 the sentence embedding is 2 complex numbers: contrastive/retrieval capacity is bounded as 15.3(e) states;
  recall saturation UNVERIFIED. Use (2,1) -> larger q_s or text states for retrieval.
- E-1 applies: Z_a ~ 2^{-2 q_n} at init; (G.9) divides by sqrt(Z_a), so the Sec. 5.1 guard (D >= eps_D) and grad-norm
  clip apply to every anchor.
- Trace distance / Bures given for pure states only; mixed versions are Sec. 14.6 / Theme A2 (two Jacobi eigensolves).
- Literature: abstracts of arXiv:2408.06061, 2409.08777, EMNLP D11-1129 confirmed via search snippets; full texts not
  fetched. BQP-completeness (vs hardness) UNVERIFIED. Attribution of entropy-entailment to Balkir et al. rather than
  Bankova et al. is taken from verifier corrections, UNVERIFIED by the worker.

### 2.4 Theme A4. Partial trace, reduced density matrices and full joint/marginal distributions as strided reductions over the statevector

Members: full joint probability distribution over a multi-qubit register ("all answers at once" QA and full
marginals in DisCoCirc) | direct readout of inner products, reduced density matrices and entropies for text-level
tasks. Tier A. Verification programs (fp64, C1-C13): scratchpad/ptrace/ptrace.c, grad.c, ptrace2.c. Network
unavailable to every verifier; literature claims from memory, UNVERIFIED where marked.

(1) Quantum-theoretic definition. Text register of W = q_n m qubits (C17, (14.1)-(14.2)), Psi in C^{2^W}. For a
subset A of wires with complement R, the partial trace is the CPTP map Tr_R: L(H_A (x) H_R) -> L(H_A),

    rho_A = Tr_R |Psi><Psi|,   rho_A[a][a'] = sum_r Psi[ins(r,A,a)] conj(Psi[ins(r,A,a')])            (T.1)

Categorically it is the counit (cap) of the compact structure applied to the doubled (CPM / Selinger) category:
DisCoCat in CPM(FHilb) (Piedeleu, Kartsaklis, Coecke, Sadrzadeh 2015; Bankova / Lewis entailment) is exactly
"compose, then discard". Frobenius delete eps (11.9) on a wire equals postselecting a spider; the partial trace is the
DIFFERENT operation of discarding without postselection (the unique causal effect). Derived quantities: the joint
Z-basis distribution p[x] = |Psi[x]|^2 (the measurement channel on all wires), marginals p_A(a) = diag(rho_A),
conditionals p(a|b) = p_{AB}(a,b)/p_B(b), von Neumann entropy S(A) = -Tr rho_A log2 rho_A, mutual information
I(A:B) = S(A)+S(B)-S(AB) (the "who is related to whom" quantity of DisCoCirc), and the QA readouts F = <c|rho_q|c>,
Tr(rho_i sigma_j) of (14.19) and Laakkonen-Meichanetzidis-Coecke 2024.

(2) Why it is undone. The hardware limit is real and fundamental: one shot returns one bitstring, so the joint
distribution over n qubits needs O(2^n/eps^2) shots for rare entries; rho_A for q qubits needs 3^q Pauli bases x
shots (tomography), entropies inherit the tomography error non-linearly, and no-cloning means one execution of a text
circuit serves one incompatible question. This is why QDisCoCirc (arXiv:2408.06061) DEFINES its tasks as single
expectation values or one overlap per circuit, and why Duneau et al. (arXiv:2409.08777, Quantinuum H1-1) answer one
binary question per circuit (UNVERIFIED: which readout primitive, how many actors). The honest qualifier from the
verifiers (3 of 3 on member 2, 2 of 3 on member 1 voted "not hardware-blocked at W <= 14"): at simulable width the
programme ALREADY reads amplitudes and per-actor states classically (lambeq NumpyModel / PytorchModel return exact
amplitudes; Duneau et al. inspect per-actor Bloch states for interpretability, UNVERIFIED detail). What has never
been done is (a) making entropy / mutual-information timelines, marginal-based losses and all-questions-at-once QA
into TASKS or TRAINING OBJECTIVES, and (b) doing it with a small working set rather than numpy einsum over a
materialised state. That is what is distilled below.

(3) Exact classical formulation. Indexing (C1/C7): field A = [f, f+q), d = 2^q, R = 2^{W-q} remaining indices;

    ins(r, f, q, a) = (r & (2^f - 1)) | (a << f) | ((r >> f) << (f+q))                                  (T.2)

(a) Joint: p[x] = re[x]*re[x] + im[x]*im[x], one elementwise pass.

(b) All marginals at once (new): the Walsh-Hadamard transform of p is the Sec. 2.1 butterfly with U = [[1,1],[1,-1]]
on every qubit, real, adds only:

    z[S] = sum_x (-1)^{popcount(x & S)} p[x] = <Z_S>,  S = 0..2^W-1 ;  z[0] = 1, z[1<<k] = <Z_k>       (T.3)
    p_A(b) = 2^{-|A|} sum_{T subset A} (-1)^{popcount(b & T)} z[T]     (marginal of ANY subset, |A| <= W)   (T.4)

Every one of the 2^W Z-string correlators and hence every marginal comes from one W-stage butterfly; conditionals are
elementwise divides of (T.4) outputs.

(c) All m single-wire RDMs in one pass over Psi ((14.19) shared):

    for r in 0..R-1: for i in 0..m-1: for a in 0..d-1: for b in 0..a:
        v = Psi[ins(r, q*i, q, a)] * conj(Psi[ins(r, q*i, q, b)]);  rho[i][a][b] += v;  (b != a) rho[i][b][a] += conj(v)

Two-wire rho_{qq'} from the exact state: (T.1) with A the union of both fields, d^2 x d^2 Hermitian (16 complex at
q_n = 1); from the MPS use (14.19') verbatim.

(d) Entropy: q = 1 closed form, tr = rho00 + rho11, det = rho00 rho11 - |rho01|^2, lambda = (tr +- sqrt(tr^2 - 4
det))/2, S = -sum lambda log2 lambda (guard lambda <= 1e-15). q = 2: the 4x4 Hermitian Jacobi of scratchpad/jacobi.c
(Sec. 14.6, Theme A2 (SF.3)). Purity Tr rho^2 needs no eigensolve: sum |rho[a][b]|^2.

(e) All-answers matrix: F[i][j] = <c_j| rho_i |c_j> (pure candidates) or Tr(rho_i sigma_j) (mixed): an m x Q table of
Frobenius inner products, d^2 complex MACs each, one tiny GEMM (rho rows flattened to d^2, candidates as d^2 columns
of |c><c| or sigma^T).

(4) Kernel shape, flops, working set. FMA-class per C9; complex MAC = 4.

    quantity                    shape                           flops                         working set (extra to Psi)
    p[x]                        elementwise                     2 x 2^W                       0 (in place over re[])
    WHT all correlators         real butterfly, W stages        W 2^{W-1} adds                0 (in place)
    one marginal p_k            masked reduction                2^{W-1} adds                  1 fp32
    all m RDMs (q_n = 1)        strided Gram accumulate         m x 3 x 4 x 2^{W-1} = 6 m 2^W  4 fp32 per wire (Hermitian 2x2)
    all m RDMs (q_n = 2)        same, d = 4                     m x 10 x 4 x 2^{W-2} = 10 m 2^W 16 fp32 per wire
    two-wire rho_{qq'} (q_n=1)  Gram, 4x4 Hermitian             10 x 4 x 2^{W-2} = 10 x 2^W    16 fp32
    entropy 2x2 / 4x4           scalar closed form / Jacobi     ~30 / ~2000                   0 / 32 fp32
    F[i][j] GEMM                m x Q x d^2 complex MACs        16 m Q (q_n=1), 64 m Q (q_n=2) m Q fp32

At (q_n, q_s) = (1,1), m = 3, W = 3: RDMs 144 FMA-class, WHT 12 adds, F 3x3 = 144; total < 400 flops, working set 12
+ 9 + 16 = 37 fp32 = 148 B on top of 64 B of state. At (2,1), m = 3, W = 6: RDMs 1920 FMA-class, working set 48 + 9 =
57 fp32 = 228 B on top of 512 B of state. Everything is memory-free beyond d^2 accumulators per wire; nothing
materialises a 2^W x 2^W object and the CPTP form (13.7) is never used.

(5) Composition with the resident state. Register-resident (Layout B, warp-per-sentence, W <= 6): p[x] per thread; a
marginal over qubit k is a __shfl_xor_sync add-reduction over the W-1 lane bits other than k (k = W-1 is
thread-local); the WHT is W stages of shfl_xor + add/sub, exactly the H-layer of Sec. 3.1 (3.1) on a real vector. rho
for wire k: fetch the partner amplitude psi[t ^ (1<<k)] (one shuffle of re, im), form the 4 Hermitian reals per
thread, then the same W-1-step xor reduction; all m wires reuse the reduction network, so "one pass" is literal.
Thread-per-sentence (Layout A) and HVX sentence-per-lane (E-9): everything is per-lane scalar arithmetic, zero
permutes. HVX amplitude-per-lane: partner via vror by 2^k lanes, reduction via log2 vror + vadd in fp32 (C12: never
accumulate p or rho in fp16). Fused tensordot (Sec. 1.3) and tree (Sec. 11): the readout tensor before the final cup
IS a bipartite state; rho_s of the sentence wire after tracing (not postselecting) the cup fields is the (1.6)
mixed-state readout. MPS (Sec. 14.3): single- and two-wire RDMs by (14.19)/(14.19'); the full joint p[x] is NOT
available from an MPS without 2^W work (perfect sampling, O(chi^2 d) per site, is the substitute -- UNVERIFIED cost
constant). Bound (14.16): I(a:b) <= 2 log2 chi is the diagnostic the MPS exposes for free.

(6) Training rule. Every quantity is a function of rho_A, so with Lambda = dL/drho_A (Hermitian d x d, frozen) the
loss is locally the expectation of O_eff = Lambda (x) I_R and Sec. 5.3 applies unchanged:

    lambda_Ng = (Lambda (x) I) Psi   (one d x d butterfly, Sec. 2.1 / 2.2, on field A);  then (5.6)-(5.7)                (T.5)
    Lambda for Tr(rho M): M;   for -Tr rho log2 rho: -log2 rho (the -I/ln2 term drops since Tr drho = 0);
    for Tr rho^2: 2 rho;   for I(A:B): -log2 rho_A (x) I - I (x) log2 rho_B + log2 rho_AB (on the AB field);
    for a loss on p[x] or on marginals (T.4): diagonal, lambda[x] = (dL/dp[x]) Psi[x]  (this is (5.8) with D = 1;
    with postselection / MPS renormalisation use the (5.8) quotient with D = Tr rho)

Several seeds ADD into one lambda (entropy timeline over T sentences: T seeds injected at their snapshot positions
during the backward sweep, cost still ~3 forward passes). Tree form: (11.27)-(11.29) with the same seeds.
Parameter-shift: valid for the linearised forms only (Tr(rho M), N_c, D, each Z-string correlator (T.3)); entropy via
the chain rule dS = -sum_k log2(lambda_k) <v_k| drho |v_k>, i.e. d shift-rule expectations with M_k = |v_k><v_k|,
four-term rule (5.4) for CRz/CRx. Verified (grad.c): dS_0/dtheta_CRz = 0.42460500 adjoint vs 0.42460500 central;
dF/dtheta = 0.06697503 both.

(7) Minimal tests (exact to 8 digits; ptrace2.c).
T-W2: Psi = (|00>+|11>)/sqrt2: p = (0.5, 0, 0, 0.5); rho_0 = diag(0.5, 0.5), S_0 = 1.0000 bit; WHT = [1, 0, 0, 1] =
(<1>, <Z0>, <Z1>, <Z0Z1>).
T-W3 (DisCoCirc, W = 3, q_n = 1): nouns Rx Rz Rx |0> with Alice (0.3, 0.7, 1.1) on wire 0, Bob (0.5, -0.4, 0.9) on
wire 1, Carol (0.2, 0.9, -0.5) on wire 2; verb gate H(0) H(1) CRz(0.8)(0->1).

    p[x]  = 0.23226815 0.15797409 0.33890441 0.23050132 0.00976660 0.00664262 0.01425052 0.00969230
    WHT   = 1 0.19037934 -0.18669710 -0.03554327 0.91929592 0.17501495 -0.17162988 -0.03267478
    P(bit0=1) = 0.40481033, P(bit1=1) = 0.59334855, P(bit2=1) = 0.04035204
    p_12(b1,b2) = 0.39024223 0.56940572 0.01640922 0.02394283 (WHT and direct agree);  p(b1|b2=0) = 0.40665145 / 0.59334855
    rho_0 = [[0.59518967, 0.07212047-0.44774271i],[c.c., 0.40481033]];  rho_1 = [[0.40665145, -0.05211578-0.45089462i],[c.c., 0.59334855]]
    S_0 = S_1 = 0.22649829, S_2 = 0, S_01 = 0, I(0:1) = 0.45299658, I(1:2) = 0; purity_0 = 0.92947193
    F[i][j] (rows wires, cols fresh Alice/Bob/Carol): [0.10869068 0.07484161 0.76058650; 0.03830983 0.05736967 0.56906946; 0.44898774 0.40336022 1.0]
    ||rho_0 - rho_1||_F^2 = 0.10198248

T-W4 (2,1) -- the shared (2,1) reference for Themes A2, A4 and A5 (cited from Secs. 2.2 (7) and 2.5 (7)); fp64,
produced by scratchpad/critic2/extra.c (C1-C13). q_n = 2, two nouns, W = 4. IQP ansatz 1 (closing H layer, spec
Sec. 0 C10) so that the noun and verb H layers do not cancel -- under ansatz 0 the same construction gives an
almost-product state with lam_max = 0.99994, a C10 consequence (Sec. 2.5 (8)(1)). Alice = IQP ansatz 1, L = 1,
t = 0.3 on field [0,2): H(0) H(1) CRz(0.3)(0->1) H(0) H(1); Bob = same with t = 0.5 on field [2,4). Verb gate (14.4)
= IQP ansatz 1, L = 1 on qubits 0..3, angles (0.8, -0.6, 0.4): H(0..3), CRz(0.8)(0->1), CRz(-0.6)(1->2),
CRz(0.4)(2->3), H(0..3). CRz(t)(c->k) = diag phase exp(-+ i t/2) on the target when the control bit is 1 (C5).

    Psi[0..15] = (+0.85857861 +0.00126688i) (+0.07042478 +0.00608809i) (+0.02157761 -0.24960174i) (-0.00035745 +0.24224677i)
                 (+0.00838243 +0.12976146i) (+0.04028242 +0.01064367i) (+0.03772361 -0.14277022i) (-0.03661202 +0.00236509i)
                 (-0.03103498 -0.20534578i) (-0.00110030 -0.00728021i) (-0.02580275 +0.00389970i) (+0.05793803 -0.00875648i)
                 (+0.03103498 +0.20534578i) (+0.00110030 +0.00728021i) (+0.02580275 -0.00389970i) (-0.05793803 +0.00875648i)
    marginal p_Alice(a) = 0.84032725 0.00684110 0.08593501 0.06689664 ;  <Z0> = 0.85252452, <Z1> = 0.69433671 (WHT z[1], z[2])
    rho_Alice (row-major, a = ket row; (14.19) / (T.1) over the Bob field) =
      [[0.84032725,    0.06524995,    0.23126091i,  -0.23709717i],
       [0.06524995,    0.00684110,    0.02424644i,  -0.01841018i],
       [-0.23126091i, -0.02424644i,   0.08593501,   -0.06524995 ],
       [0.23709717i,   0.01841018i,  -0.06524995,    0.06689664 ]]
    eig (4x4 cyclic complex Jacobi, SF.3) = 0.97766824, 0.02233176, 0, 0
      (rank 2: the H layers are local to each field and only CRz(-0.6)(1->2) crosses the Alice | Bob cut, an operator of
       Schmidt rank 2 applied to a product state, so Psi has Schmidt rank <= 2 across the cut and rho_Alice rank <= 2)
    S(rho_Alice) = 0.15433977 bit ; purity = 0.95633390 ; I(Alice:Bob) = 2 S = 0.30867954 bit (pure two-noun state, S_AB = 0)
    working set: state 16 complex = 128 B, rho 16 fp32 packed = 64 B, Jacobi in place (Sec. 2.2 (4)): the (2,1) column of the table in (4).
    pass criteria: fp32 kernel within 1e-6 on every entry above; the two zero eigenvalues must come out <= 1e-6 and be clamped
      before the log (Sec. 2.2 (8)(iii)); the 16x16 dense verb path (Sec. 2.5 (4), G = H^{x4} diag(CRz phases) H^{x4}) and the
      gate-by-gate path (8 single-qubit H butterflies + 3 CRz diagonal phases) must agree on Psi to 1e-6.

T-Sec8 (bent-wire N TV N register, W = 3): rho of the s qubit (1) = [[0.5, 0.40523146-0.17132911i],[c.c., 0.5]], S =
0.32759738; p(q0=q2=0) = 0.14026553 = Z of Sec. 8 (E-1 consistency), p(s=1 | q0=q2=0) = 0.41736118.

(8) Caveats and UNVERIFIED.
- T-W3 shows z[3] = z[1] z[2]: for an IQP verb (diagonal after H) the Z-basis joint distribution is a PRODUCT
  distribution while I(0:1) = 0.45 bits. Entanglement created by phase gates is invisible to p[x] and every marginal;
  only the RDM off-diagonals (or WHT in other bases) see it. Joint-distribution readouts are therefore not a substitute
  for RDMs under ansatz 0/1.
- The joint p[x] needs 2^W fp32 (C12: fp16 subnormal from W = 15); RDM accumulation in fp32 always; entropy's log2
  near lambda = 0 needs the guard.
- Tr rho_A is 1 only for unitary texts; postselected or truncated texts divide by Tr rho (Sec. 14.6).
- UNVERIFIED: exact readout primitive and actor counts of Duneau et al. 2024; whether their interpretability inspection
  was classical RDMs; CPM/DisCoCat citation details (Piedeleu et al. 2015); MPS perfect-sampling cost constant.
  Nothing here changes the capacity ceiling of Sec. 12 or the memory tiers of 14.1.

### 2.5 Theme A5. DisCoCirc text circuits as sequences of small gates on persistent noun wires: higher-order boxes, retained sentence wires, bounded-frontier contraction and wire slicing

Members: full DisCoCirc text circuits (non-unitary noun updates, retained sentence/proposition wires, higher-order
boxes) instead of the unitary-only single-qubit-readout restriction used on hardware | DisCoCirc texts with many
actors: bounded-cut contraction along the text circuit instead of qubit reuse. Tier A. Obeys C1-C17 and E-1. Labels
(HO.n); cited, not re-derived: text state and unitary bending (14.1)-(14.4), gate-list construction and residency
table (Sec. 14.1), MPS (14.6)-(14.14), reduced density matrix (14.19)/(14.19'), branching for mixtures (14.18),
CPTP / ancilla forms (13.7), spiders (11.6)-(11.9), tree adjoint (11.27)-(11.30), qudit odometer (12.16').
Reference program (fp64): scratchpad/discocirc_ho/ho2.c (Tests A-G; every check closed to <= 1.1e-16).

(1) Quantum-theoretic definition. Text = morphism in a symmetric monoidal category whose objects are noun wires
(Coecke 2019, arXiv:1904.03478; Wang-Mascianica, Liu, Coecke 2023, arXiv:2301.10595). Three constructions go beyond
(14.2):

(a) Higher-order box (frame). A sentence box B on K wires (K = 2 for a transitive verb, B in End(C^{d^K}), d =
2^{q_n}) is itself the ARGUMENT of a word: "Claire thinks that [B]", "[B_1] because [B_2]", "not [B]". Semantically a
frame is a map F: End(C^{d^K}) -> End(C^{d^{K'}}) that is linear in B, i.e. a superoperator, vec(B') = S vec(B), S in
C^{d^{2K'} x d^{2K}}. Special cases: sandwich B' = V B U (lambeq Frame conversion, unitary, hardware form); controlled
frame G = sum_c |c><c|_C (x) F_c(B) on the frame's own noun wire C (coherent control, "Claire believes / doubts");
hedge B' = (1-t) I + t B (modality, non-unitary); reversal B^dag; any learned S.

(b) Retained sentence wire. Instead of closing the verb's s factor with an effect ((14.3) e[s]) or absorbing it
((14.4), a d^2 x d^2 unitary), keep it: the verb is a (2 q_n + q_s)-qubit gate G_V in U(d^2 d_s) acting on (subj, obj,
s_fresh), s_fresh = |0>^{q_s}. The s wire is a proposition-valued entity: its reduced state rho_s is the truth-value of
the clause, it can be an argument of a later 2-wire gate ("Claire doubts [s]"), asserted (postselect s = 1) or
discarded (partial trace).

(c) Bounded frontier. The text state lives on F live slots (actors and propositions currently referenced), W = q_n F_n
+ q_s F_s. A finished wire k is removed by partial trace Tr_k, which commutes with every later gate on other wires, so
rho_Q(final) is unchanged by tracing early. A new actor enters a freed slot as |0> then U_noun. This is the exact,
noiseless version of the mid-circuit measure-and-reset that compressed 108 logical qubits into 20 physical on H1-1
(Duneau et al. 2024, arXiv:2409.08777; UNVERIFIED numbers).

(2) Honest reason it is undone. Hardware-blocked (all three verifiers agree on this core): (i) coherent control of a
whole sub-circuit by an entity qubit needs a controlled version of every gate inside B (controlled arbitrary U(4)
costs tens of CNOTs, UNVERIFIED count) -- absent from every hardware run; (ii) non-unitary frame maps and asserted
propositions need ancillas plus postselection, whose shot cost is exponential in the number of postselected wires
(Laakkonen, Meichanetzidis, Coecke 2024, arXiv:2408.06061, formalises the postselection-free, unitary-only,
one-qubit-per-noun QDisCoCirc restriction for exactly this reason; quantitatively, S postselected wires of width q
accept w.p. prod_s p_s with E[p_s] ~ 2^{-2 q} each (the E-1 projection argument applied per wire), i.e. E[P_post] =
2^{-2 q S}; Table A's A7 figure 4^{-q} per cup is this same number); (iii) retained s wires make the width grow with
the number of clauses, not entities, so Duneau et al. absorbed s into a 2-qubit unitary and read out single noun
qubits; (iv) the interpretability data the programme asks for (per-box effect on a wire, entity-entity entanglement,
Duneau 2025 arXiv:2507.02940) are reduced density matrices, i.e. tomography. NOT hardware-blocked, per the verifier
corrections: unitary sandwich frames exist in lambeq (Frame -> sandwich, UNVERIFIED exact form; arXiv:2505.13208 by
Krawchuk, Khatri, Ortega, Kartsaklis is the text-to-circuit pipeline, not a semantics paper); classical statevector
training of small texts was already the training backend of Duneau et al.; lambeq's PytorchQuantumModel contracts
text circuits with tensornetwork/opt_einsum and a doubled network for Discard (verified from source by the
historian). So the classical contribution is (a)-(c) as exact, register-resident maths plus the exact-sum /
trajectory-sampled slicing that hardware can only sample noisily.

(3) Exact classical formulation. Register psi[0..2^W-1], W = live qubits, C1 order, C3 planar. Slot table: uint8_t
slot_of_wire[N] (logical entity -> bit field start, or FREE), uint8_t free_list[]. Transient wires (fresh s) are
allocated at the TOP field so that kron-in and slice are contiguous.

    // dense K-wire gate, local m = sum_r bit_{w[r]}(i) << r (generalises C6), optional control predicate           (HO.1)
    apply_dense(psi, W, K, w[K], G[2^K][2^K], ctrl_mask, ctrl_val):
      for each block base i with all gate bits 0:  if ((i & ctrl_mask) != ctrl_val) continue;
        gather v[m] = psi[i | bits(m)];  o = G v (2^K x 2^K complex gemv);  scatter psi[i | bits(m)] = o[m]
    // controlled frame: ctrl_mask = field of C, ctrl_val = c << start(C); sum_c form = one call per c with G = F_c(B)   (HO.2)
    // superoperator frame, once per frame occurrence, W-independent:
    vec(B') = S vec(B), row-major vec: vec(B)[mo * 2^K + mi] = B[mo][mi];  k-term form B' = sum_k L_k B R_k (2k gemms)  (HO.3)
    // sandwich (lambeq): B' = Gb (I (x) B) Gt as 2 gemms of size 2^{K'} then ONE gate, or as 3 gates (Test C)        (HO.4)
    // retained s wire: kron-in |0>^{q_s} at the top: psi[2^{W} .. 2^{W+q_s}-1] = 0 (zero-fill, no multiplies), W += q_s;
    //   then apply_dense with K = 2 q_n + q_s on (subj, obj, top)                                                    (HO.5)
    // slice wire field (f, q) at value v: psi_v[j] = psi[ins(j, f, q, v)], ins = low | v << f | high << (f+q); W -= q;
    //   top field: psi_v = psi + v * 2^{W-q}, a POINTER OFFSET, zero copies                                            (HO.6)
    // partial trace = sum over slices:  rho_Q = sum_v rho_Q(psi_v)   (Test D, 1.1e-16), so Tr_k is never formed        (HO.7)
    // slot reuse (measure-and-reset, exact): for v in 0..2^q-1: psi_v = slice(k, v); kron-in |0> at bit k
    //   (out[ins(j,k,0)] = in[j], out[ins(j,k,1)] = 0; in place when the move is done top-down); U_noun on k; continue  (HO.8)
    // assertion "that-clause is true": keep only slice v = 1 of the s field, Z *= p_v; negation: X on s first          (HO.9)
    // trajectory sampler: draw v ~ p_v = ||psi_v||^2 (exact, one reduction), continue with psi_v / sqrt(p_v);
    //   E over T trajectories of rho_Q^{(traj)} is unbiased for (HO.7)                                                 (HO.10)

Exact DFS over (HO.8): recursion depth = number of sliced wires S; each level keeps its parent vector so the next v
can be produced. Interpretability outputs: rho_k (14.19), entropy S(rho_k) from the 2x2 closed form or the 4x4
Jacobi (ptrace.c), per-box effect Delta = rho_k(after) - rho_k(before) with trace distance (1/2)||Delta||_1 = |eig|
for 2x2.

(4) Kernel shape, flops, working set. Every op is a strided small dense complex gemv over blocks (butterfly
generalised to 2^K), a strided Hermitian reduction (rdm), or a W-independent small gemm (frame maps). Complex MACs
(x4 for FMA-class, C9); state 8 x 2^W B fp32 (4 x 2^W fp16 store, C12).

    op                                (q_n,q_s) = (1,1)                               (2,1)
    retained-wire verb (HO.5)         K = 3, G 8x8 = 512 B; 8 x 2^W cMACs             K = 5, G 32x32 = 8 KB (SMEM/VTCM; or gate-by-gate Sim14); 32 x 2^W cMACs
    controlled 2-wire box (HO.2)      16 x 2^{W-3} cMACs (half the blocks of a 2-wire gate)   256 x 2^{W-6} cMACs (control field value c: 1/4 of blocks)
    superoperator S (HO.3)            S 16x16 = 2 KB, 256 cMACs once per occurrence  S 256x256 = 512 KB: out of budget -> k-term (HO.3), 2k x 4096 cMACs, 2k x 2 KB
    sandwich (HO.4) as gemms          2 x 512 cMACs + one 3-wire gate; cheaper than 3 gates only for W >= 7    2 x 32768 cMACs: use 3 gates
    slice top wire (HO.6)             0                                               0
    slice / kron-in interior bit      2^{W-1} moves                                   3 x 2^{W-2} moves
    rdm of one wire (14.19)           3 x 2^{W-1} cMACs, result 32 B                  10 x 2^{W-2} cMACs, result 128 B
    working set (exact DFS)           (S+1) x 8 x 2^W + 512 B gate + 32 B per queried wire; trajectory sampler: 2 x 8 x 2^W

At W = 3 (Tests): retained-wire verb 64 cMACs, controlled box 16, rdm 12. Register tier (C14, (1,1)), from the
working-set row above: CUDA thread ~1 KB: W <= 5 for exact DFS with one sliced wire (2 x 256 + 512 = 1 KB), W <= 6
only for the trajectory sampler (2 x 512 = 1 KB, gate applied from angles, no 512 B G); HVX context 4 KB: W <= 7 for
DFS with S = 1 (2 x 1 KB + 512 B) and W <= 8 for the sampler; beyond that VTCM / SMEM. At (2,1) the 8 KB dense G
forces gate-by-gate application from angles (Sim14 on 5 qubits, 16 L butterflies) and the state itself (8 x 2^W)
sets the tier as in Sec. 16.

(5) Composition with the statevector and the MPS. Slots ARE the register of Sec. 14.1: C17 wire_map becomes a
runtime slot table; the gate opcode is G2F/G1F of Sec. 14.1 with a ctrl field added. Frontier bound: W_max = q_n
f_max + q_s (one live s wire at a time when propositions are consumed by the next clause). MPS (14.3): a controlled or
non-unitary K-wire gate is applied by (14.8)-(14.12) unchanged (the SVD does not need unitarity; renormalise by Z); a
retained s wire is a new site inserted next to the verb's subject; slicing a site = fixing its physical index and
merging the remaining chi x chi matrix into a neighbour (d chi^2 MACs), which lowers no bond dimension but removes the
site. Mixtures from (14.18) and slices (HO.7) compose: both are sums of pure branches with rho_Q accumulated.

(6) Training rule. All maps are linear in psi, so Sec. 5.3 / (11.27)-(11.30) apply with these additions: controlled
gate adjoint = the same predicate with G^dag; non-unitary B': lambda_psi = B'^H lambda_psi', lambda_{B'} = sum_blocks
lambda_o v^H (4^K 2^{W-K} cMACs, equal to the forward cost), then lambda_{vec(B)} = S^H lambda_{vec(B')}, lambda_S =
lambda_{vec(B')} vec(B)^H, and (11.30) run on the K-wire word circuit with matrices A = B, B = lambda_B in place of
vectors (d^K x the state cost); hedge parameter: dpsi/dt = (B - I) psi (one extra gate). Renormalisation by Z enters
only through the root weights (11.27'). Slicing: rho_Q and its gradient are sums over branches (accumulate in DFS);
trajectory sampling gives unbiased N_c and D separately, ratio bias O(1/T) as in (13.10'). Parameter-shift holds for
angles inside B under controlled/sandwich frames (generators unchanged; C13 caveat for CRz/CRx), not for S entries
(no gate). Adjoint is the recommended rule.

(7) Minimal test (ho2.c, fp64; W = 3, q_n = q_s = 1, ansatz 0 L = 1). Nouns Rx Rz Rx |0> (C10): Alice (0.3, 1.1,
-0.7) = (+0.835531-0.458701i, +0.250590+0.169370i); Bob (1.9, 0.4, 2.2) = (-0.451882-0.196438i, -0.029689-0.869674i);
Claire (-1.3, 0.8, 0.5) = (+0.848353-0.242066i, -0.305042+0.358678i); Dave (0.6, -0.9, 1.4). Verb "loves" = IQP dense
(C10) with angles (0.9) on 2 wires, (0.9, -1.7) on 3 wires.
- Test A (retained s wire, HO.5): Alice bit 0, Bob bit 1, s bit 2 kron-in |0>; 3-wire gate. PsiA[0..7] =
  (-0.293981-0.360127i), (-0.352236+0.044051i), (-0.288217+0.129064i), (-0.207857+0.122240i), (-0.293981-0.360127i),
  (-0.352236+0.044051i), (+0.165123+0.269185i), (+0.148002+0.190375i); p(s=1) = 0.500000, rho_s = [[0.5,
  0.321786+0.156557i],[., 0.5]].
- Test B (controlled frame, HO.2): Claire bit 2 controls "loves" on (0,1): max|diff| vs dense |0><0| (x) I + |1><1| (x)
  B = 0. rho_Alice before [[0.908518, 0.131685-0.256460i],[., 0.091482]], after [[0.847144, 0.174929-0.133889i],[.,
  0.152856]]; p(Claire=1) = 0.221700 = |Claire[1]|^2; eig(rho_Alice) = 0.911139, 0.088861, S = 0.432655 bits; per-box
  trace distance 0.1437 (from printed digits).
- Test C (sandwich, HO.4): 3 gates vs 2 gemms + 1 gate: 7.9e-17.
- Test D (slice = partial trace, HO.7): branch weights 0.340659 + 0.659341 = 1; sum_v rho_Alice^(v) vs full: 1.1e-16.
- Test E (slot reuse, HO.8): Bob sliced, Dave into bit 1, "Claire loves Dave" on (2,1); vs W = 4 reference (Alice 0,
  Bob 1, Claire 2, Dave 3): rho_Alice 2.8e-17, rho_Dave = [[0.278850, -0.237196-0.327522i],[., 0.721150]] 2.8e-17.
- Test F (hedge, HO.3): B' = 0.3 I + 0.7 B on (Alice, Bob): Z = 0.526433, rho_Alice/Z = [[0.682740,
  0.329724+0.317417i],[., 0.317260]].
- Test G (assertion, HO.9): PsiA postselected s = 1: p = 0.5, rho_Alice|s=1 = [[0.631685, 0.326742+0.296409i],[.,
  0.368315]].
- Test H ((2,1) reference, extra.c): the T-W4 state of Sec. 2.4 (7) is the (2,1) two-noun DisCoCirc register of this
  theme (W = 4, q_n = 2, verb gate on both 2-qubit fields = the K = 4 dense 16x16 G of the table in (4), 2 KB fp32,
  or the gate-by-gate path from angles: 8 H butterflies + 3 CRz diagonal phases). Psi[0..15], p_Alice, rho_Alice and
  its eigenvalues listed there are the pass criteria for BOTH paths (max |diff| <= 1e-6 fp32); the dense and
  gate-by-gate paths must agree to 1e-6 with each other, and the rank-2 spectrum (0.97766824, 0.02233176, 0, 0)
  checks the (14.19) RDM over a 2-qubit field with R = 4 remaining indices.

(8) Caveats and UNVERIFIED. (1) Ansatz 0 at L = 1 with a fresh |0> s wire gives p(s = 1) = 1/2 IDENTICALLY (H then
phases only, C10 consequence): the retained-wire readout is uninformative under it; use L >= 2 or ansatz 1. (2) Exact
DFS slicing has time sum_t 2^{q s_t} x 2^{W_t}, i.e. up to the full 2^{q N} statevector time, with memory (S+1) 2^W
instead of 2^N: it trades memory for time exactly as circuit slicing does; the sampler is the memory-flat alternative
with 1/sqrt(T) error. Early trace-out without slicing needs the 4^W doubled state (physicist correction; do not use,
13.2). (3) Discarding a finished wire that is in a product state with the rest costs one branch (Tr rho_k^2 = 1
check, 3 x 2^{W-1} cMACs); generic wires are entangled after any 2-wire gate. (4) UNVERIFIED: lambeq's exact Frame ->
sandwich construction and which wires V, U span; controlled-U(4) CNOT counts; the 108 -> 20 qubit-reuse figure and
the text sizes of arXiv:2505.13208; whether Duneau et al. reported noiseless simulations of the larger instances;
BQP-hardness (arXiv:2408.06061) is worst-case and does not bound bounded-frontier narratives. (5) The learned 16x16 S
at (1,1) has 256 complex parameters per frame word, more than the whole word table of Sec. 7 for small vocabularies;
prefer the k-term or controlled forms. (6) At (2,1) the dense retained-wire verb (8 KB) is out of the register tier;
use gate-by-gate application.

### 2.6 Theme A6. Exact training on the simulator: adjoint gradients, weight-tied lexicon updates, Fubini-Study natural gradient, Hessians / Gauss-Newton / Levenberg-Marquardt, gradient-variance / DLA landscape diagnostics

Members: exact adjoint gradients for 1e5-1e6 parameters instead of SPSA / parameter-shift with shots | exact quantum
natural gradient / Fubini-Study (QFI) metric instead of Bures approximations | exact Hessians, Gauss-Newton and
Levenberg-Marquardt on tiny word-parameter sets | direct barren-plateau diagnostics (gradient variance, DLA
dimension) for DisCoCat/DisCoCirc ansaetze | joint / curriculum training of DisCoCirc text circuits with weight-tying
and exact gradients on the largest simulable texts. Tier as adjudicated: A only for the exact Fubini-Study metric;
every other member is B (see (2) and Table B rows B4-B7). Conventions C1-C13, errata E-1/E-2/E-10, addendum
(11.27)-(11.31), (13.12) assumed and cited. Test program (fp64, plain C): scratchpad/exact_train/et.c
(cc -O2 -o et et.c -lm).

(1) Quantum-theoretic definition. Sentence circuit in bent-wire form (Sec. 1.4): psi(theta) = U_Ng(theta) ... U_1 |0>,
W qubits, postselected set K, surviving s field S, sentence state sigma[c] = psi[b(c)], c < 2^{q_s}, Z = D = sum_c
|sigma[c]|^2, P_c = |sigma[c]|^2 / D (5.1). The parameter space is a product of tori (one per word, C10), the map
theta -> [sigma] lands in projective space CP^{2^{q_s}-1}; categorically the functor F: pregroup -> FdHilb is fixed
and only the word states move. The objects of this theme are the derivatives of that map:
  J_c[j]      = d sigma[c] / d theta_j                       complex, 2^{q_s} x P          (Jacobian)
  K_c[j][k]   = d^2 sigma[c] / d theta_j d theta_k           complex, 2^{q_s} x P x P
  F_FS[j][k]  = Re( <d_j s|d_k s> - <d_j s|s><s|d_k s> ),  s = sigma/sqrt(Z)   real PSD P x P  (Fubini-Study metric = Bures metric on pure states, Stokes-Izaac-Killoran-Carleo, Quantum 4, 269 (2020))
  F_cl[j][k]  = sum_c (1/P_c) dP_c/dtheta_j dP_c/dtheta_k    classical Fisher of the readout = Gauss-Newton matrix of L_CE
  H[j][k]     = d^2 L / d theta_j d theta_k                  exact Hessian
  Var_theta[dL/dtheta_j] over theta ~ U[0,2pi)^P and dim g, g = Lie closure of {i G_k} (McClean et al. 2018; Ragone et al., Nat. Commun. 2024, s41467-024-49909-3).
Weight tying: every occurrence of word w reads the same theta_w; the loss is L(theta) with theta_w appearing r times,
so dL/dtheta_w = sum over occurrences (chain rule through concatenation, Sec. 5.5, (11.27)).

(2) Why it is undone (honest). On a device every quantity above is a shot-limited, postselection-amplified estimate: a
gradient by parameter shift costs 2P circuits x S shots x 2^{2 q_n}/Z-ish rejection (Sec. 5.2, E-1), so Meichanetzidis
et al. 2020 and Lorenz et al. 2021 (arXiv:2102.12846) used SPSA with tens to low hundreds of parameters (exact P
UNVERIFIED); the full FS metric needs O(P^2) Hadamard/SWAP-test subcircuits, which is exactly why Stokes et al. 2020
introduced the diagonal / block-diagonal fallbacks; a Hessian entry needs the 4-evaluation second-order shift rule
(Mari, Bromley, Killoran, PRA 103, 012405 (2021), arXiv:2008.06517), O(P^2 S) shots, and no QNLP paper computed one.
BUT the verifiers agree, and the record shows, that exact adjoint/autodiff gradients and weight tying have been
standard in the lineage's CLASSICAL backends since lambeq (Kartsaklis et al. 2021, arXiv:2110.04236: NumpyModel /
PennyLaneModel / PytorchQuantumModel) and are what Duneau et al. 2024 (arXiv:2409.08777) trained with before
evaluating on H1-1; barren-plateau diagnostics have always been a classical-simulation activity; Hessian/GN/LM and
natural gradient are missing because the community focused on hardware-executable optimisers and toy datasets, not
because the simulator forbade them (Toumi-Yeung-de Felice 2021, arXiv:2103.07960, is a general dual-diagram autodiff
framework, not a hardware trick). So: hardware forced SPSA on devices (true), and only the exact-metric QNG is
plausibly "A"; the rest is "not yet done in this lineage" plus "not done without Python". The 2026 QNLI "BuresQNG"
posters (arXiv:2608.16939) are UNVERIFIED and are not relied on.

(3) Exact classical formulation. Cotangent convention (11.27). All formulas use D_k = -(i/2) G_k, so dU_k/dtheta_k =
U_k D_k = D_k U_k and dU_k^dag/dtheta_k = -U_k^dag D_k (G_k from C13; G_k need not square to I, so CRz/CRx need no
shift-rule special case).

(a) Reverse-mode COMPLEX Jacobian: one adjoint sweep per output c with the constant seed lambda = e_{b(c)} (instead of
the loss seed (5.8)):
  // A = psi, B = lambda, C = scratch; J[c][0..P-1] = 0
  A = |0>; for k = 1..Ng: A = U_k A                       // once, shared by all c
  for c in 0..2^{q_s}-1:
      B = 0; B[b(c)] = 1; A_c = A (copy or recompute)
      for k = Ng down to 1:
          if param(k): C = G_k A_c; J[c][p(k)] += (-i/2) * sum_b conj(B[b]) C[b]   // COMPLEX accumulate, no 2 Re
          A_c = U_k^dag A_c;  B = U_k^dag B
Tying is automatic: "+=" over gates with the same p(k) (for effect-word Ry gates multiply by sign_k, Sec. 5.3).
Everything else is algebra on the tiny 2^{q_s} x P matrix J:
  v_j   = sum_c conj(sigma[c]) J[c][j]                                 (complex P-vector)
  dN_c/dtheta_j = 2 Re( conj(sigma[c]) J[c][j] );  dD/dtheta_j = 2 Re v_j;  dP_c/dtheta_j = (dN_c D - N_c dD)/D^2
  dL/dtheta_j   = sum_c g_c dP_c/dtheta_j   (g_c of (5.1); identical to (5.6)-(5.8), the loss seed is just J contracted with dL/dsigma)
  F_FS[j][k]    = Re( (J^H J)[j][k] / D  -  conj(v_j) v_k / D^2 )      (Gram of J, rank <= 2^{q_s+1} - 2 per sentence)
  F_cl          = sum_c (1/P_c) gP_c gP_c^T, gP_c[j] = dP_c/dtheta_j    (rank <= 2^{q_s} - 1 per sentence; = H_GN of CE)
  H_GN(MSE)     = 2 sum_c gP_c gP_c^T
Per batch: F = sum over sentences, scatter-added into the (word-pair) blocks; ranks add, so a batch of B sentences
gives rank up to 2B on the touched lexicon block.

(b) Exact Hessian by forward-over-reverse on the complex seed (P directions v = e_j, each direction one sweep with two
extra vectors t = d psi/ds, tl = d lambda/ds):
  forward: t = 0; for k: t = U_k t; A = U_k A; if param(k): t += v[p(k)] * D_k A
  reverse: B = e_{b(c)}; tl = 0; for k = Ng..1:
      if param(k): C = D_k A; C2 = D_k t;  K[c][p(k)][v] += <tl|C> + <B|C2>
                   t -= v[p(k)] C;  tl -= v[p(k)] D_k B
      A = U_k^dag A; B = U_k^dag B; t = U_k^dag t; tl = U_k^dag tl
Then, for L = -ln P_y with N = N_y:
  N_jk = 2 Re( conj(J[y][k]) J[y][j] + conj(sigma[y]) K[y][j][k] ),  D_jk = sum_c (same with c)
  H[j][k] = -( N_jk/N - N_j N_k/N^2 - D_jk/D + D_j D_k/D^2 )
Levenberg-Marquardt: solve (M + mu I) delta = -grad, M in {H, F_cl, F_FS}, P x P dense on-chip (Gaussian elimination
P^3/3 FMA; Jacobi eigen-decomposition for the saddle-free variant |H| = V |Lambda| V^T). Increase mu until L decreases
(the test shows why).

(c) Diagnostics: Var[dP_c/dtheta_j] and Var[dN_c/dtheta_j] by R draws of theta ~ U[0,2pi)^P, one Jacobian each. DLA:
for Pauli generators the closure is a set of Pauli strings (2W bits each; the commutator of two anticommuting strings
is one string = XOR, else 0), so dim g is a bit-set closure, O(dim^2) word ops, no matrices; the CRz generator
|1><1|_c Z_t = (Z_t - Z_c Z_t)/2 is split into its two strings (this gives an upper bound on dim in general; exact on
the test below, where the dense 8x8 Gram-Schmidt closure gives the same 13).

(4) Kernel shape, flops, working set. Everything is the Sec. 2.1/2.2 butterflies (unchanged kernels) plus one tiny
complex Gram GEMM and P x P real solves. F_fwd = FMA-class ops of one forward pass: 8N per 1q gate, 2N per CRz (N =
2^W).
  (1,1), W = 3, N = 8, IQP L = 1, P = 8, Ng = 11:  F_fwd = 608
  (2,1), W = 5, N = 32, IQP L = 1, P = 6 (nouns 1 each, verb 4), Ng = 15:  F_fwd = 2688
  Jacobian (a): 2^{q_s} sweeps x ~3.5 F_fwd:  4.3 kFMA / 19 kFMA;  working set 3 N complex + J = 24N + 16 P B: 192+128 = 320 B / 768+96 = 864 B
  F_FS Gram: 4 P^2 2^{q_s} FMA = 512 / 288;  F, F_cl storage 4 P^2 B = 256 / 144 B
  Hessian (b): 2^{q_s} P sweeps x ~7.5 F_fwd: 73 kFMA / 242 kFMA;  working set 6 N complex (A, B, t, tl, C, C2) = 48N B: 384 / 1536 B, plus K 8 2^{q_s} P^2 B = 1024 / 576 B and H 4P^2 B
  Solve / eig: P^3 class, < 1 kFMA at P <= 8; at P = 256 (a whole small lexicon block): 256 KB fp32 matrix, 5.6 MFMA per solve
  Variance MC: R x Jacobian: R = 2e4 -> 86 MFMA at (1,1); one draw per thread / lane, no cross-thread traffic
  DLA closure: dim^2 x 2W-bit ops; dim <= 4^W - 1 (63 at W = 3)
General W: gradient 24 x 2^W B, Hessian 48 x 2^W B (W = 12: 96 KB / 192 KB, SMEM/VTCM tier, Sec. 7), independent of
P; per-batch lexicon gradient touches only the words present (sparse row update of the 10 B/word angle table, E-4(a)).

(5) Composition with the register-resident state / bounded-cut tree. Circuit form (Sec. 1.4): the sweeps above ARE
the Sec. 5.3 loop with two changes (complex seed, complex accumulate); on CUDA Layout A/B nothing leaves registers
except the P complex partial sums per sweep (warp shuffle reduction, 128 B). Tree form (11.7): replace "U_k^dag" by
the node-op adjoints (11.28)-(11.29) and seed lambda_sigma = e_c (instead of (11.27')); J is then obtained from
2^{q_s} tree sweeps with memory (11.31); forward-over-reverse tangents t, tl follow every node op, doubling (11.31).
MPS/DisCoCirc (14.8)-(14.12): the tangent of a truncated SVD is not exact; either freeze the truncation basis (U, V
held, differentiate S and the site tensors only: exact for the truncated model, standard) or keep chi large enough
that r <= chi (no truncation, then (14.8)-(14.9) are bilinear and the tree rule applies verbatim). Tying across
occurrences in a text is the same "+=" into theta_w, whichever form.

(6) Training rule. Per batch: J (and K if second order) per sentence -> grad, F_FS or F_cl or H accumulated per
touched word block -> one of
  Adam (5.9) on the exact grad (baseline);  QNG: theta -= eta (F_FS + lam I)^{-1} grad;  GN/LM: theta += delta from (F_cl + mu I) delta = -grad (CE) or (H_GN + mu I) (MSE);  exact Newton only with |H| (saddle-free) or with mu > -lambda_min(H).
Tied parameters need no extra rule; SPSA variance (E-2) disappears entirely.

(7) Minimal test (Sec. 8 sentence "Alice loves Bob", y = 0, theta as in Sec. 8; fp64; parameter order Alice t0 t1 t2,
Bob t0 t1 t2, loves t0 t1)
  sigma, Z = 0.14026553, P_0 = 0.58263882, L = 0.54018780 (Sec. 8)
  J[0][*] = (-0.153747-0.082233i) (-0.111668-0.032824i) (-0.195298+0.034241i) (-0.155637+0.074755i) (-0.160743-0.063031i) (-0.142326+0.013201i) (-0.042811+0.089040i) (0+0i)
  J[1][*] = (-0.027744-0.059312i) (-0.091212-0.047350i) (-0.079393-0.036092i) (-0.282279+0.048580i) (-0.123180-0.171297i) (-0.255348-0.044071i) (+0.059037-0.012180i) (+0.018149-0.285853i)
  reverse-mode J (2 sweeps) vs derivative-gate insertion: max diff 8.4e-17; dP_0/dtheta and dL/dtheta reproduce the 8 Sec. 8 values to all printed digits (J[0][7] = 0 exactly: the E-10 "dN_0 = 0 for loves t1" vector)
  F_FS Alice block [[0.029656, 0, 0.022682],[0, 0.002590, -0.005394],[0.022682, -0.005394, 0.028581]]; loves block [[0.081259, 0.086578],[0.086578, 0.340786]]; max |Alice x loves cross entry| = 0.065387 (NOT block-diagonal after the cups); trace 0.672175; eigenvalues 0.422064, 0.250110, six zeros (rank 2 = dim_R CP^1)
  F_cl = g g^T / (P_0 P_1): ||g||^2 = 0.4104613, factor 4.112335, eigenvalue 1.687955
  H diag = -0.101953 +0.053268 -0.282165 +0.505458 +0.094471 +0.724574 +0.232793 +0.060376; H[6][7] = 0.471489; row 0 = -0.101953 -0.174086 -0.294110 +0.052496 -0.031183 +0.026124 -0.213108 -0.159580
  eigenvalues -0.906866 -0.515500 -0.229291 -0.023446 +0.223067 +0.297588 +0.757358 +1.683910 (indefinite: a saddle-ish point); H vs central differences of the exact gradient 9.0e-9; K by forward-over-reverse vs double insertion 9.6e-17
  Steps from L = 0.540188: GD lr 0.1 -> 0.420273; QNG (F_FS + 0.01 I), eta 0.05 -> 0.401693; GN/Fisher mu 0.1 -> 0.020774 (delta = +0.06736 +0.04388 -0.03985 +0.17462 -0.10373 +0.08690 -0.19903 -0.53082); LM exact H mu 0.1 -> 0.655513 (UP), mu 1.0 -> 2.266139 (UP), mu 3.0 -> 0.124290; saddle-free |H| + 0.1 I -> 0.389996
  Tied "Alice loves Alice" (Bob^T reads Alice's angles): L = 0.6930406, dL/dtheta_Alice = -0.0232379 +0.0013051 -0.0204914, loves +0.4262803 +0.7232927, vs FD 1.2e-10
  MC R = 2e4 uniform: E[D] = 0.2500 (E-1), 1.08 % of draws D < 2^-6 (E-10); Var[dP_0/dtheta_j] = 0.0736 0.0379 0.0548 0.0626 0.0523 0.0215 0.0203 0.0388; Var[dN_0/dtheta_j] = 0.00614 0.00372 0.00369 0.00507 0.00523 0 0.00156 0 (2^{-2W} = 0.0156)
  DLA of {X0, Z0, X2, Z2, |1><1|_0 Z_1, |1><1|_1 Z_2}: dim 13 of 63 (dense closure and Pauli-bit closure agree; the 13 strings are X0 Y0 Z0 Z1 Z0Z1 X0Z1 Y0Z1 X2 Y2 Z2 Z1Z2 X2Z1 Y2Z1)

(8) Caveats and UNVERIFIED.
- F_FS after the cups is NOT block-diagonal per word (cross block 0.065 vs diagonal 0.03-0.34 here): assemble the full
  P x P per batch; the per-word-block shortcut in the member text is wrong.
- Per-sentence F_FS / F_cl are rank <= 2^{q_s+1}-2 / 2^{q_s}-1: regularise (lam, mu) or batch >= P/2 sentences per word
  block, or the solve is singular.
- Exact Newton on this loss is indefinite at generic points (4 negative eigenvalues in the test); use GN/Fisher (CE:
  F_cl) or |H|; the LM mu must exceed -lambda_min or the step climbs (mu = 0.1, 1.0 both went up).
- The 1/D tail (E-10) enters H and F_cl through 1/D^2 and 1/P_c: apply the Sec. 5.1 D guard before forming them; clip
  delta norm as for the gradient.
- Second-order fp32: 48 x 2^W B working set and O(Ng u32) round-off per sweep are fine; never fp16 (Sec. 5.3).
- MPS tangent through a truncating SVD is inexact unless the basis is frozen (5); UNVERIFIED how much this matters at
  chi = 8-16.
- Ragone-style Var ~ 1/dim g prediction was NOT checked here (Var[dN_0] 0.004-0.006 at dim 13 is a single data point;
  the formula assumes a 2-design over exp(g), which a 1-layer IQP sentence is not). Everything else in (7) is verified
  numerically.
- Literature: Lorenz 2021 / Meichanetzidis 2020 parameter counts, lambeq model class names and versions, the QDisCoCirc
  training details, and arXiv:2608.16939 are from memory / UNVERIFIED (network unavailable); the Sec. 5.3 sign rule
  for effect-word Ry (absent in IQP) applies to J and K unchanged.

### 2.7 Theme A7. Wire representation: qudit / mixed-radix dimensions, multi-qubit types, dense per-word unitaries or unconstrained tensors, and choice of number field

Members: qudit / mixed-radix wire dimensions (d > 2 per wire, unequal dimensions, qudit Frobenius spiders) |
multi-qubit noun and sentence spaces (q_n, q_s > 1) | circuit depth: dense per-word unitaries (or arbitrary tensors)
instead of 1-3 IQP / Sim14 layers | free choice of number field (real-only, complex-with-meaningful-phase, mixed).
Obeys C1-C13; cites addendum labels where they already cover ground. Numbers from scratchpad/wirerep/qd.c (fp64).

(1) Quantum-theoretic definition. FHilb puts no constraint on F(n), F(s): F(n) = C^{d_n}, F(s) = C^{d_s}, any
integers, unequal allowed; F(t^l) = F(t^r) = F(t). A word of type t_1..t_m is a unit vector (DisCoCat) or a unitary
(DisCoCirc, addendum 14.1 (U)) on the mixed-radix space prod_i d(t_i). The cup on type t is <cup_d| = sum_{k<d}
<k|<k| (12.5, "cup pairs digit v_A = v_B"); the Frobenius spider is delta[i,j,k] over i,j,k < d ((11.6) with 2^q ->
d). Field choice: the same diagrams over R (real Hilbert space: SO(d) rotations, real spiders) or over C. Nothing in
the categorical construction distinguishes d = 2 from d = 3 or R from C; the sentence vector is (1.3)/(1.5) with digit
strides in place of bit shifts.

(2) Why it is undone (honest, per verifier notes). Three sub-claims split cleanly. (a) Arbitrary d and unequal dims,
real field, dense unconstrained tensors: NOT hardware-blocked -- this is the ORIGINAL classical DisCoCat (CSC 2010;
Grefenstette-Sadrzadeh 2011 d_n ~ 2000; Kartsaklis-Sadrzadeh 2012/2013) and lambeq's TensorAnsatz / MPSAnsatz /
SpiderAnsatz take ob_map: Ty -> Dim(any int) per type (historian verified in lambeq/ansatz/tensor.py). Real-amplitude
ansaetze (Ry+CNOT) run fine on QPUs. So the number-field item is a design choice (tier B), and its only QPU cost, phase
READOUT, belongs to the amplitude-access theme (A4). (b) What IS hardware-blocked and un-run anywhere: the UNITARY,
angle-generated, Born-rule / postselected DisCoCat with q_n, q_s >= 2 or d > 2 (every hardware run used q_n = q_s = 1:
Meichanetzidis 2020, Lorenz 2021/JAIR 2023, Duneau 2024; per-cup postselection success 4^{-q} = 1/d^2 --
kernel-engineer correction: not 1/d -- plus depth and SWAP cost; lambeq circuit ansaetze are qubit-only), and
DisCoCirc word GATES in U(d^2) with d > 2. (c) Dense per-word maps: the physicist's correction stands -- a DisCoCat
state word uses only the first column of any unitary (2^{Q+1}-2 dof, (12.1)), and Sim14 saturates that manifold for
Q <= 3 (12.1 rank checks), so "dense" buys expressivity only for word MAPS (DisCoCirc gates, MPO cores of 12.5) and
for states with Q >= 4. This distillation therefore adds: the mixed-radix register butterfly, a U(d) parameterisation
that keeps "angles in", unequal-radix tile shapes, and the field-choice cost table. UNVERIFIED (no network): whether
any 2024-2026 Quantinuum work used qudits or q_n >= 2 on hardware; the worker believes not.

(3) Exact classical formulation. Register: n wires, radices d[0..n-1], strides s[j] = prod_{j'<j} d[j'] (12.16'), D =
prod d[j], planar re[]/im[] (C3). Digit j of address i: (i / s[j]) % d[j]; use the odometer (12.16') on Hexagon (no
integer divide).

Word STATE on a wire of dimension d, angle-generated (unitary by construction, 2d-2 real angles = (12.1) exactly; real
field: phi = 0, d-1 angles = (12.2)):

    // two-level Givens on levels p<q, C5 half-angle: out[p] = c in[p] - s in[q]; out[q] = s in[p] + c in[q]   (Ry_pq)
    // phase on level q: out[q] *= e^{i phi}                                                                    (P_q)
    // state chain: v = P_{d-1}(phi_{d-2}) Ry_{0,d-1}(t_{d-2}) ... P_1(phi_0) Ry_{01}(t_0) |0>   (q = 1 first, C4)
    // closed form: v[0] = prod_q cos(t_q/2);  v[q] = e^{i phi_{q-1}} sin(t_{q-1}/2) prod_{q'<q-1} cos(t_{q'}/2)

Word MAP U in U(d) (d^2 real angles: d(d-1)/2 pairs x (theta, phi) + d output phases xi; Reck/Clements-type product,
citation details UNVERIFIED):

    U = Diag(e^{i xi_k}) * prod_{(p,q) lexicographic, (0,1) first} [ P_q(phi_pq) Ry_pq(theta_pq) ]
    // materialise transiently: apply the gate list to the d columns of I:  ~2 d^2 (d-1) complex MACs, d^2 complex regs
    // real field: drop P_q and xi -> SO(d), d(d-1)/2 angles

Tree path (DisCoCat, replaces 2^q by d in Sec. 1.3): V[a,s,b] = phi_V[a + d_n s + d_n d_s b];

    for s<d_s, b<d_n: R[s + d_s b] = sum_{a<d_n} n1[a] * V[a + d_n s + d_n d_s b]   // d_n^2 d_s MACs
    for s<d_s:        sigma[s]     = sum_{b<d_n} R[s + d_s b] * n2[b]               // d_n d_s MACs
    Z = sum_s |sigma[s]|^2 (fp32);  p[s] = |sigma[s]|^2 / Z

Register path (DisCoCirc, generalises Sec. 2.1 to radix d): dense gate U (d_j x d_j) on wire j:

    for base in 0..D-1 with digit_j(base) == 0:
      for k<d_j: in[k] = psi[base + k*s[j]]                       // d_j strided loads
      for i<d_j: psi[base + i*s[j]] = sum_k U[i*d_j + k] * in[k]  // d_j^2 complex MACs, in place
    // 2-wire gate on (j,k): local m = v_j + d_j v_k (subject low, C6/14.4), U is (d_j d_k)^2, bases = D/(d_j d_k)

Cup on a live register (Sec. 2.3 generalised): out[base] = sum_{i<d} psi[base + i (s_j + s_k)]; spider merge:
out[base + i s_j] = psi[base + i (s_j + s_k)].

Field choice per WORD class (the state inherits the widest field): all-real -> re[] only, p = sigma^2/Z; complex ->
4M (planar, C3: 4 real GEMMs) or Gauss 3M: k1 = c(a+b), k2 = a(d-c), k3 = b(c+d), re = k1-k3, im = k1+k2 (3 real GEMMs
+ 5 adds; ~1 bit lost to cancellation, fp32 only); mixed (complex nouns, real verb): complex x real MAC = 2 FMA.

(4) Kernel shape, flops, working set. Tree N TV N: d_n d_s (d_n + 1) complex MACs; peak d_n^2 d_s + d_n + d_n d_s (+
d_n), in-place d_n^2 d_s + d_n (slot rule Sec. 1.3).

    (d_n,d_s)      cMACs  FMA(complex/mixed/real)  peak amps  in-place  bytes fp32 (complex / real)
    (2,2)=(1,1)     12      48 / 32 / 12             14 (16)      10        80 / 40
    (4,2)=(2,1)     40     160 / 96 / 40             44 (48)      36       288 / 144
    (3,2)           24      96 / 60 / 24             27 (30)      21       168 / 84
    (8,4)          288    1152 / 640 / 288          296          264     2112 / 1056

    mixed = 2 d_n^2 d_s (real verb x complex noun, 2 FMA per MAC) + 4 d_n d_s (complex R x complex noun, 4 FMA per MAC):
    32, 96, 60, 640 for the four rows.

Register butterfly: 1-wire dense gate 4 D d_j FMA, 2-wire 4 D d_j d_k FMA, state 8 D B fp32 + gate 8 d^2 (or 8 d_j^2
d_k^2) B. At q_n = 1, two noun wires: D = 4, 4x4 gate 64 FMA, 32 B + 128 B; q_n = 2 (d = 4): D = 16, 16x16 gate 1024
FMA, 128 B + 2 KB. The 16x16 complex gate real-embedded ([[a,-b],[b,a]]) is exactly one 32x32 fp16 HMX / mma tile
(Sec. 3.3); UNEQUAL radices hit tile shapes without waste: d_j d_k = 32 (e.g. 8 x 4) gives a 32x32 complex = 64x64
real block. Above D ~ 16384 (6 wires d = 5: 125 KB fp32) the register leaves SMEM: switch to the MPS.

(5) Composition with the resident state / bounded-cut network. Tree: (1.3) + (12.16') digit scatter; peak formulas
above; slot rule unchanged. Register: (14.1)-(14.2) with mixed-radix D; residency table of 14.1 indexed by D instead
of 2^{q_n N}. MPS: (14.6)-(14.12) are already written for general d; with unequal radices Theta' is (d_i chi_{i-1}) x
(d_{i+1} chi_{i+1}) and site memory sum_i d_i chi_{i-1} chi_i. MPO verb (12.13)-(12.16): d_n arbitrary already.
Density matrices: (13.10) with d^2 x d^2 blocks; (13.10') mixture trick unchanged.

(6) Training. Tree adjoint (11.27)-(11.30) verbatim with digit index maps (multilinearity does not depend on radix).
Angle recurrence (11.30)/(5.6) with the qudit generators: Ry_pq(t) = exp(-i t Y_pq / 2), Y_pq = -i|p><q| + i|q><p|;
P_q(phi) = exp(-i phi G / 2) with G = -2|q><q|. Both G have eigenvalues in {0, +-1} on the full d-space, so the
two-term shift rule FAILS (as CRz, C13/Sec. 5.2): use the four-term rule with the quotient rule (5.3)-(5.4), or the
adjoint. State chain: closed-form dv/dt_q from the product formula (no gates needed). Real field: same with phases
absent. Verified: (5.6) with Y_01 on a d = 3 wire gives -0.00337194 = central difference -0.00337194 (test C).

(7) Minimal test (exact numbers, fp64). Test A, d_n = 3, d_s = 2, complex. Alice: t = (0.3, 0.7), phi = (1.1, -0.5);
Bob: t = (0.5, -0.4), phi = (0.9, 0.2); state chain gives Alice = [0.92882457, 0.06778456+0.13318036i,
0.29754212-0.16254800i], Bob = [0.94959868, 0.15378877+0.19379818i, -0.18865613-0.03824249i]. Verb phi_V[k] = 18^{-1/2}
exp(i 0.37 k^2), k = a + 3 s + 6 b. Result sigma = [0.31506046+0.13826012i, -0.18817784-0.01558615i], Z = 0.15403278,
p = [0.76853091, 0.23146909], Loss(y=0) = 0.26327449. Brute force over the radix-(3,3,2,3,3) product state with the
odometer (162 addresses, cups a = a', b = b') agrees to 3.5e-18. (A verb phase linear in k factorises and gives p =
[0.5, 0.5]: avoid it in tests.)
Test B, real field (phi = 0, V[k] = (k+1)/sqrt(2109)): Alice = [0.92882457, 0.14943813, 0.33904743], Bob =
[0.94959868, 0.24740396, -0.19249318], sigma = [0.02362573, 0.11662979], Z = 0.01416068, p = [0.03941725, 0.96058275].
Test C, register 2 wires d = 3 (D = 9), Psi[a + 3b] = Alice[a] Bob[b], U(3) with theta = (0.4, 0.8, 1.2), phi = (0.2,
-0.3, 0.6), xi = (0.1, -0.2, 0.3): ||U^H U - I||_1 = 6.8e-16; U row 0 = [0.89819135+0.09011973i,
-0.18207240-0.01826817i, -0.38747287-0.03887696i]. Dense radix-3 butterfly on wire 0 vs the Givens gate list: diff
4e-16; Psi'[0] = 0.66732534+0.10410836i, Psi'[4] = -0.07792706+0.04339028i, Psi'[8] = -0.08085513-0.08340194i;
||Psi'||^2 = 1.0000000000; wire-1 marginal unchanged: [0.90173766, 0.06120872, 0.03705363] = |Bob|^2.

(8) Caveats and UNVERIFIED.
- E[Z] for a Haar verb generalises E-1 by the same projection argument to d_s / (d_n^2 d_s) = 1/d_n^2 (not re-run by
  Monte Carlo): Q1.15 only for d_n <= 16, fp32 above; the Z_min guard is load-bearing.
- C8 nested-vs-component-wise pairing is moot for a single digit; only matters if a d = d1 d2 wire is later split.
- Angle tables for qudit states are as large as the vector (12.5): "angles in" is a format choice, not compression; for
  maps it is d^2 angles vs d^2 complex entries (half the bytes).
- Gauss 3M precision loss and HMX tile/ISA details (Sec. 3.3) UNVERIFIED on target silicon; Reck/Clements decomposition
  order UNVERIFIED in detail; binary-field semantics (Widdows-Cohen) is not Hilbert-linear and is out of scope.

### 2.8 Theme A8. Vocabulary and dataset scale: streamed lexicon tables with pretrained amplitude encoding, shape-batched kernel launches, noise-free benchmarks, ansatz sweeps and cross-lingual fidelity experiments

Members: vocabulary scale and pretrained initialisation via free amplitude encoding | dataset scale (thousands of
sentences per launch instead of ~100 per experiment) | noise-free scaling studies, ansatz search and standardised
benchmarks at 1e4-1e5 sentences | multilingual / language-independence experiments by exact state fidelity. Tier A
(vocabulary / pretrained part); the other three members are tier B (Table B rows B11-B13) and are given only where
the maths differs. Reference programs: scratchpad/lexicon/lex_test.c, scratchpad/lexicon/fid_test.c (fp64,
max |adjoint - central| <= 2e-10).

(1) Quantum-theoretic definition. DisCoCat functor as in Sec. 1.2: F(n) = C^{d_n}, F(s) = C^{d_s}, d_n = 2^{q_n} (or
any d_n on the qudit path, (12.16')). The "Dis" half of DisCoCat (Coecke-Sadrzadeh-Clark 2010, arXiv:1003.4394;
Grefenstette & Sadrzadeh 2011, Kartsaklis-Sadrzadeh-Pulman 2012, Fried-Polajnar-Clark 2015 low-rank verbs) is: noun =
a distributional vector x_w in R^m, verb = a tensor built from noun vectors (outer products, relational sums,
low-rank). Quantum version (Zeng & Coecke 2016 assumed QRAM; Coecke-de Felice-Meichanetzidis-Toumi 2020,
arXiv:2012.03755): the noun is the AMPLITUDE-ENCODED pure state

    |n_w> = sum_{a<d_n} n_w[a] |a>,   n_w = P x_w / ||P x_w||,   P in C^{d_n x m} (projection/compression)      (L.1)

and the verb is a state on n^r s n^l with C7 layout V[a,s,b] = phi_V[a + d_n s + d_n d_s b], initialised as a
low-rank combination of noun outer products,

    V[a,s,b] = sum_{r<R} c_r[s] u_r[a] v_r[b],   (u_r, v_r) = (subject, object) noun pairs seen with the verb   (L.2)

Composition is (1.5) verbatim (transpose cup, no conjugation); readout P_c = |sigma[c]|^2 / Z (1.4). Cross-lingual
member: two text/sentence states sigma_L1, sigma_L2 from shared or separate lexicons, compared by the exact fidelity
F = |<sigma_L1|sigma_L2>|^2 / (Z_1 Z_2) (this IS an inner product, conjugate on the left) and per-noun reduced density
matrices rho_k (14.19) by Tr(rho^L1_k rho^L2_k).

(2) Why it is undone (honest). Hardware: loading an arbitrary d-dimensional unit vector into log2(d) qubits needs O(d)
gate depth (no QRAM), so arXiv:2012.03755 explicitly replaces pretrained vectors by shallow parameterised word circuits
trained from sentence labels; every word must then occur in enough hardware-executed circuits, giving vocabularies of
17 (MC) and ~115 (RP) in Lorenz et al. 2021 (arXiv:2102.12846; released code: 2^13 shots/circuit, SPSA, 100
iterations; MC = 130 sentences). Duneau et al. 2024 (arXiv:2409.08777, H1-1) and the Krawchuk et al. 2025 compiler
(arXiv:2505.13208, texts to ~13 pages) still train from scratch and report proof-of-concept experiments; lambeq Gen II
(May 2025) advertises DisCoCirc coverage, not pretrained initialisation. Other reasons: the classical DisCoCat lineage
already did pretrained-noun / low-rank-verb composition in NumPy (Grefenstette-Sadrzadeh 2011 ... Wijnholds et al.
2020), and Li et al. 2019 CNM used GloVe-initialised complex embeddings; lambeq's tensor ansatz path (checked in
lambeq_ansatz_tensor.py / numpy_model.py in the scratchpad) initialises word tensors RANDOMLY and compiles one JAX
lambda PER DIAGRAM, i.e. no pretrained init and no shape batching in the released tool. So the novelty is: the
exact-gradient, shape-batched, register-resident kernel form with a streamed lexicon -- the engineering, not the
semantics. Dataset scale, noise-free sweeps (Castillo et al. 2025 arXiv:2502.20744 already compared IQP/Sim14/Sim15
classically) and the cross-lingual fidelity experiment (Waseem, Liu, Wang-Mascianica, Coecke 2022 arXiv:2208.10281 and
the Bengali/Hindi/Arabic follow-ups are purely grammatical, no trained states) are tier B: undone by community focus,
not by physics.

(3) Exact classical formulation. Lexicon tables, split planar (C3), row = word id:
    NOUN[Vn][d_n] complex (fp32 pairs, or real fp16 when P is real), unit norm enforced at write time;
    VERB[Vv][d_n d_s d_n] complex dense, OR cores A[d_n][r], B[r d_s r], C[r][d_n] per (12.13)-(12.14);
    per-verb slot for R (the a=0 slot rule of Sec. 1.3 applies unchanged to a streamed tensor).
Per sentence, N TV N, the fused tensordot of Sec. 1.3 with phi_w REPLACED by a gathered row:

    // ids: w1 (subject), wv (verb), w2 (object); all bounds compile-time per (q_n, q_s)
    n1 = NOUN[w1]; n2 = NOUN[w2]; V = VERB[wv];            // gathers: d_n, d_n, d_n^2 d_s complex
    for s: for b: R[s][b] = sum_a n1[a] * V[a + d_n s + d_n d_s b];     // d_n^2 d_s complex MAC
    for s: sigma[s] = sum_b R[s][b] * n2[b];                             // d_n d_s complex MAC
    Z = sum_s |sigma[s]|^2;  if (Z < Z_min) uniform;  P[c] = |sigma[c]|^2 / Z;                                   (L.3)

Initialisation from pretrained vectors: n_w = normalise(P x_w) with P either the top-d_n PCA directions of the
embedding matrix (real) or a random Gaussian projection (JL; keeps pairwise cosines to ~1/sqrt(d_n)); phases all zero,
so the model starts real and Im planes are all-zero until the optimiser moves them (a real-only flag halves the
tables, Sec. 6.3 flags.bit2). Verb init (L.2) then ||V||_F := 1 (range rule (12.16); E[Z] is no longer 2^{-2 q_n} of
E-1 -- Z is data-dependent, so the Z_min guard is load-bearing from step 0).

Cross-lingual fidelity loss and its cotangents (convention (11.27), lambda_x = dL/d conj(x)):

    ip = sum_s conj(sigma_1[s]) sigma_2[s];   F = |ip|^2 / (Z_1 Z_2);   L_F = 1 - F
    lambda_sigma2[s] = -( ip sigma_1[s] / (Z_1 Z_2) - F sigma_2[s] / Z_2 )
    lambda_sigma1[s] = -( conj(ip) sigma_2[s] / (Z_1 Z_2) - F sigma_1[s] / Z_1 )                                   (L.4)

then (11.29) into each sentence's words; words shared across the pair ACCUMULATE (Sec. 5.5 scatter-add). Per-noun
agreement: rho_k from (14.19), Tr(rho^1 rho^2) = sum_{ij} rho^1[i][j] rho^2[j][i], d_n^2 complex MACs.

(4) Kernel shape, flops, working set. Shape: two small dense complex matmuls (a (d_s d_n) x d_n matrix-vector, then
d_s x d_n matrix-vector) per sentence, no butterflies; batched over B sentences of one shape it is a batched GEMV
with gathered operands. Flops per sentence 4 (d_n^2 + d_n) d_s real FMA (+ 2 d_s for Z, 1 rcp): (1,1) 12 complex MAC
= 48 FMA; (2,1) 40 cMAC = 160 FMA; d_n = 64, d_s = 2: 33,280 FMA. Working set (streamed operands only): (1,1) 2 + 8 +
2 = 12 complex = 96 B fp32 / 48 B fp16, 10 in place (80 / 40 B); (2,1) 4 + 32 + 4 = 40 = 320 B / 160 B, 36 in place
(288 / 144 B); W-form: 2^{2 q_n + q_s} + 2 x 2^{q_n}. Lexicon bytes: noun 8 d_n B fp32 complex (16 B at (1,1), 32 B at
(2,1), 512 B at d_n = 64); dense verb 8 d_n^2 d_s B (64 B, 256 B, 64 KB at d_n = 64) -- 50 K nouns x d_n = 64 = 25.6
MB complex fp32 (12.8 MB real fp32 or complex fp16, 6.4 MB real fp16); 5 K dense verbs at d_n = 64 = 320 MB, so use
(12.13) cores: 8 (2 d_n r + r^2 d_s) B = 9.2 KB at r = 8 -> 46 MB. Arithmetic intensity of the dense-verb gather is 4
(d_n^2 + d_n) d_s / (8 (d_n^2 d_s + 2 d_n)) ~ 0.5 FMA per byte at every d_n: the streamed-lexicon kernel is
BANDWIDTH-bound, unlike the angle-generated kernel (Sec. 7, tens of bytes per sentence). Remedy: sort the batch by verb
id ("verb-resident, noun-streamed"): one CUDA block / HVX context per verb holds V in SMEM/VTCM (64 B .. 64 KB) and
streams only 2 noun rows per sentence (32 B .. 1 KB); nouns of a 50 K lexicon at d_n = 16 (6.4 MB complex fp32) sit
in L2 on a datacenter GPU; on Hexagon a 25 MB table lives in DDR and is gathered by the scalar core or vgather (v65+,
E-9), so keep d_n <= 16 real fp16 (1.6 MB, fits an 8 MB VTCM) on NPU.
Shape batching (tier B): bucket by (shape, W) exactly as Sec. 2.6; angles become word-id triples (6-12 B) + per-shape
program; B x (2^{q_s} + 1) fp32 out. B = 4096 sentences of one shape at (2,1) = 4096 x 160 FMA = 0.66 MFMA, ~1-10 us
of GPU time; a 1e5-sentence corpus is a few kernel launches per epoch, versus 8192 shots x queue time per circuit.
Noise-free sweeps: identical kernel, loop over (ansatz_id, L, q_n, q_s) headers; Kraus-noise reproduction of hardware
curves is NOT near-zero-memory (needs (13.10), 88 / 1120 complex): run it on the GPU tier only, or use the trajectory
unravelling of 13.2(a) with the pure working set.

(5) Composition with the register-resident statevector / bounded-cut network. The streamed row is byte-compatible
with phi_w of Sec. 1.3: a lexicon word and an angle-generated word may be mixed in one sentence (per-word record:
param_offset = 0xFFFFFFFF + a new flag "row" pointing into NOUN/VERB, extending C15). In the circuit form (Sec. 1.4) a
stored noun enters as the effect <n^T| = sum_a n[a] <a| applied to the field (no U_n^T needed: contract directly, d_n
complex MAC per surviving index); a stored dense verb cannot be a gate unless completed to a unitary (Householder/QR,
13.1(b)), so on the circuit / DisCoCirc (C17) path use the angle form for verbs and stored rows for nouns only; in
the MPS (14.6) a stored noun is simply the chi = 1 site tensor A_i[0][s][0] = n_i[s]. Tree form (C16): node kind 0
with the word tensor gathered instead of generated; nothing else changes.

(6) Training rule. Free entries: cotangent rule (11.27)-(11.29) verbatim with phi_v = the gathered row; grad rows
scatter-added into a NOUN_grad / VERB_grad table of the same shape (Adam 16 B/entry: 50 K x 64 complex = 102 MB --
HBM, GPU-side only). Because P_c is invariant under n_w -> t n_w and V -> t V, Euler's identity gives Re <n_w,
dL/dn_w> = 0 exactly: the raw gradient is already tangent to the unit sphere (verified below: sphere-projected == raw
to 1e-10), so a plain step plus one rsqrt renormalisation per touched row is the correct retraction. Chain to a
trainable projection P: dL/dP = lambda_n x_w^T / ||P x_w|| (the (I - n n^H) factor acts as identity on a tangent
gradient). Angle words in the same sentence use (11.30). Fidelity training: seed with (L.4) instead of (11.27'). No
shift rule exists (no gates); this is simulator-only, exactly as Sec. 11.7.

(7) Minimal test (fp64 reference: lex_test.c and fid_test.c). (1,1), d_n = d_s = 2. Raw "pretrained" vectors x1 =
(3,4), x2 = (1,0), x3 = (-1,2) -> n1 = (0.6, 0.8), n2 = (1, 0), n3 = (-0.44721360, 0.89442719). Verb (L.2) rank 2:
c_0 = (1, 0.5) on (n1, n2), c_1 = (0.25, 0.75) on (n3, n1), Frobenius-normalised, C7 index a + 2 s + 4 b:
    V = [0.35844615, 0.62832845, 0.06642289, 0.53976460, -0.06016010, 0.12032020, -0.18048030, 0.36096061]
    n1 V n2: sigma = (0.71773045, 0.47166541), Z = 0.73760526, P = (0.69839117, 0.30160883)
    n3 V n1: sigma = (0.34863290, 0.59469793), Z = 0.47521053, P = (0.25577063, 0.74422937)
    L = -ln P_0 (sentence 1) = 0.35897592;  dL/dRe n1 = (-0.21630772, 0.16223079);  dL/dRe n2 = (0, 0.18025643);
    dL/dRe V = (-0.50427093, -0.67236124, 0.76734607, 1.02312809, 0, 0, 0, 0);  dL/dIm V[0] = 0
    fidelity of the two sentences F = 0.80357345, L_F = 0.19642655;
    dL_F/dRe n1 = (-0.00454782, 0.00341086) [sum of both occurrences];  dL_F/dRe n3 = (-0.22693694, -0.11346847);
    dL_F/dRe V = (0.57168329, -0.12715791, -0.62032730, -0.30570408, 0.35576092, -0.71152184, -0.20855959, 0.41711919)
Rank-1 sanity case (verb = n1 (x) n2 only): sigma = (0.89442719, 0.44721360), Z = 1, P = (0.8, 0.2), all noun
gradients exactly 0 (the sentence is a pure scale of the verb's s-profile).
(2,1), d_n = 4: x1 = (1,2,2,4), x2 = (2,0,-1,2), x3 = (0,3,0,4), same rank-2 recipe: n1 V n2: sigma = (0.71214451,
0.54303144), Z = 0.80203295, P = (0.63233039, 0.36766961); n3 V n1: Z = 0.60406590, P = (0.34797401, 0.65202599); F =
0.91910731; L = 0.45834325; dL/dRe n1 = (-0.04531655, 0.06385513, -0.09063309, 0.02471812) (full V listed in the
program output). Kernel acceptance: fp32 planar kernel must match these to 1e-6; fp16 tables to 2e-3.

(8) Caveats / UNVERIFIED.
- arxiv.org, quantinuum docs blocked; paper-internal claims (2012.03755 QRAM argument, 2505.13208 experiment scope, Gen
  II feature list) are from memory / search snippets: UNVERIFIED in detail. Vocab 17 / ~115 and MC = 130 are from the
  released CQCL repository (verified by another worker), not re-checked here.
- Whether a JL / PCA projection of GloVe-class vectors to d_n <= 64 retains useful signal for DisCoCat tasks is
  UNVERIFIED; the classical literature used d_n = 100-300, i.e. q_n = 7-8, where dense verbs (2^{2 q_n + q_s}) are out
  of every on-chip tier and only (12.13)-(12.15) cores apply.
- Bandwidth figures assume 8 B complex fp32 rows; fp16 storage of verb cores at q_n >= 5 violates the Sec. 12.3 /
  (12.15) precision rule (fp32 only) -- the streamed model is fp32-in-HBM at large d_n.
- Fidelity as a language-independence metric is sensitive to the global phase-free but ORDER-sensitive verb (12.9):
  translated sentences with different production rules will show F < 1 even with perfect word alignment; that is the
  measurement, not a bug, but no baseline value exists in the literature (UNVERIFIED expectations).

### 2.9 Theme B (tier B). Parse structure as a scored, differentiable or superposable object: postselection-norm plausibility, soft parse forests and Sinkhorn cup placement, coherent superposition of derivations and quantum-switch ordering

Members: the postselection success probability ||s||^2 itself as a signal (grammaticality / plausibility /
ambiguity score) | gradients with respect to the parse / structure (soft parse forests, learned cup placement,
learned frame nesting) | coherent superposition of parses / derivations and indefinite causal order of word
processes (quantum-switch grammar). Tier B (undone, not because of hardware). Reference program:
scratchpad/parsestruct/ps.c (fp64; C1/C5/C7/C8; Sec. 8 words). The (1,1) Z(A,B) = 0.14026553 reproduces Sec. 8
exactly; adjoint and finite-difference gradients agree to 1e-8.

(1) Quantum-theoretic definition. Fix a type string with K pregroup derivations D_1..D_K (chart parser, Addendum
(11.4)-(11.5); count (11.25)/(11.26) accumulators). Each D_k is a cup set, i.e. a different multilinear contraction of
the SAME word tensors, giving an unnormalised sentence state sig_k in C^{d_s}, d_s = 2^{q_s}, with Z_k = ||sig_k||^2 =
p_ok(k) = the probability that every cup postselection of D_k succeeds (Sec. 8: D = Z, no 2^{-q} factor). Four objects
built on {sig_k}:
  (a) Plausibility score:  ln Z_k ; parse choice k* = argmax_k Z_k ; parse posterior q_k = softmax_k(ln Z_k) = Z_k / sum_j Z_j.   [selectional-preference score over GRAMMATICAL readings; ungrammatical strings have no diagram and no Z]
  (b) Soft parse forest (CPM / Selinger-doubled, Addendum (11.26)):  rho = sum_k w_k |sig_k><sig_k| / Z_k,  w = softmax(alpha),  P_c = rho[c][c].
  (c) Coherent superposition (a parse REGISTER |k> entangled with the sentence, then projected on sum_k conj(c_k)|k>):  S = sum_k c_k sig_k  (or c_k sig_k/sqrt Z_k),  P_c = |S[c]|^2/||S||^2; interference = 2 Re(conj(c_j) c_k <sig_j|sig_k>), read from the Gram matrix G[j][k] = <sig_j|sig_k>.
  (d) Quantum switch (Chiribella et al. 2013) on DisCoCirc noun wires (Addendum (14.2)): two sentence gates V, W in text order is fixed by Coecke 2021; the indefinite-order state is |0>_c (x) W V Psi0 + |1>_c (x) V W Psi0, control read in the X basis: p(+) = (1 + Re<WV Psi0|VW Psi0>)/2; conditioned on +, the noun register is (WV + VW) Psi0 (non-unitary, renormalise). The mixed-order channel (14.18)-style is rho = (WV rho0 (WV)^H + VW rho0 (VW)^H)/2.
  (e) Soft cup placement (Sinkhorn): for n noun states n_b and n argument slots a of a verb frame, a doubly-stochastic A (Sinkhorn of logits) gives n~_a = sum_b A[a][b] n_b, then the ordinary contraction. At a permutation matrix this is D_pi exactly; in between it is the mean-field relaxation (see caveat).

(2) Why it is undone (from the verifier notes). Not a hardware casualty. (a) The norm existed classically from day one
(Coecke-Sadrzadeh-Clark 2010; Grefenstette-Sadrzadeh 2011 compared by cosine and DISCARDED it; lambeq's
NumpyModel/PytorchModel and DisCoPy Tensor.eval return the unnormalised amplitude): community focus on cosine
similarity and on symbolic grammaticality, not hardware. Hardware contributed only negatively: on devices p_ok is the
accepted-shot fraction, the noisiest number produced, so nobody looked at it as a feature (Zeng-Coecke 2016 amplitude
amplification, Lorenz et al. 2021 divide it out). (b) Structure gradients: a mixture is LINEAR in w, so dP_c/dw_k is
the per-parse expectation the forward pass already measures on hardware; the blocker was the programme's decision to
take the parser (DepCCG, Bobcat) as a fixed prior, while classical NLP solved discrete-structure learning (Kim et al.
2017, Mena et al. 2018 Gumbel-Sinkhorn, DIORA 2019) without QNLP importing it. (c) Mixtures over parses were chosen on
SEMANTIC grounds (Piedeleu-Kartsaklis-Coecke-Sadrzadeh 2015: ambiguity = epistemic mixing, superposition would create
a new single meaning; Meyer-Lewis 2020; arXiv:2504.00040 states density matrices as the intended semantics), years
before hardware runs. No QNLP paper proposes coherent parse superposition or a quantum-switch grammar (UNVERIFIED
negative, no network this session). A log2 K controlled-parse register would be a genuine hardware cost, but it was
never the stated reason. Verdict: theory/community, hardware a minor contributing factor only for (a).

(3) Classical formulation (index conventions C1/C7; word tensors phi_w[j], j little-endian over factors)
  // forest forward: K parses, shared words; each parse = one tree evaluation (11.18') -> sig[k][0..d_s-1], Z[k]
  for k in 0..K-1: { sig[k] = eval_tree(tree[k], words); Z[k] = sum_c |sig[k][c]|^2; lnZ[k] = log(Z[k]); }
  // (a) plausibility: k* = argmax lnZ; q[k] = exp(lnZ[k] - max) / sum
  // log-domain option (HVX, no fp64): after every node op r_v: nv = ||r_v||^2; r_v *= rsqrt(nv); lnZ += 0.5*log(nv)
  // (b) soft forest: w = softmax(alpha); P[c] = sum_k w[k] * |sig[k][c]|^2 / Z[k]          (only the diagonal of rho is needed)
  // (c) coherent: S[c] = sum_k c[k]*sig[k][c]; ZS = sum_c |S[c]|^2; P[c] = |S[c]|^2/ZS; G[j][k] = sum_c conj(sig[j][c])*sig[k][c]
  // (d) switch on a 2^{q_n E} register: x = apply(W, apply(V, Psi0)); y = apply(V, apply(W, Psi0));
  //     ov = sum_i conj(x[i])*y[i]; p_plus = (1 + Re ov)/2; Sw[i] = (x[i]+y[i]); readout on wire w after renormalising by ||Sw||^2
  // (e) Sinkhorn: A = exp(L); repeat 10-20: rows /= rowsum; cols /= colsum;  ntilde[a] = sum_b A[a][b]*n[b] (real scalar x complex vector)
Dimensions: sig, S: d_s complex; G: K x K complex Hermitian; rho diagonal: d_s real; A: n x n fp32; switch register:
2^{q_n E} complex.

(4) Kernel shape, flops, working set (FMA-class ops, complex MAC = 4, C9). Per parse: the Sec. 1.3 tensordot chain
(N TV N: 2^{2q_n+q_s} + 2^{q_n+q_s} complex MACs = 12 -> 48 ops at (1,1); 40 -> 160 at (2,1)). Forest = K such chains
sharing word reads; the PP test sentence (two 3-factor words per parse) is 24 MACs = 96 ops per parse. p_ok reduction:
2 d_s real MACs (4 at q_s = 1) + 1 log; log-domain adds 2^{q_out}+2 ops per node. Soft forest readout: K d_s real
MACs; Gram: K(K+1)/2 x d_s complex MACs (K = 8, d_s = 2: 72 complex MACs = 288 ops), one small Hermitian GEMM.
Coherent axpy: K d_s complex MACs. Switch: 2 x gate cost + one 2^W-length complex dot (4 x 2^W ops) + one axpy.
Sinkhorn: 20 x 2 n^2 flops + n^2 exp; mixing 2 n^2 d_n ops. Working set beyond the register-resident chain: K d_s
complex + K fp32 (Z) + K(K+1)/2 complex (Gram, Hermitian upper triangle) + K fp32 (w or c). K = 8: (1,1) 8x2x8 = 128 B
+ 32 B (Z) + 36 x 8 = 288 B (Gram) = 448 B (the Table B cell), + 32 B (w or c) = 480 B in all; (2,1) same
(d_s unchanged; only the shared chain grows 14 -> 44 amplitudes = 112 -> 352 B). Switch at d = 2, E = 2: two
4-amplitude registers = 64 B fp32. Sinkhorn n = 3: 36 B for A, 20 x 36 B if iterations are stored for backprop (or
recompute).

(5) Composition with the register-resident evaluator. Parses are evaluated SEQUENTIALLY through the same in-place
slot chain (Sec. 1.3 "10 in place"); only the d_s-vector output and Z of each are retained, so peak = (11.18') + K
d_s. Word tensors are regenerated from angles per parse (0 B persisted, E-4(b)) or held once if the forest shares them
(PP example: 30 complex = 240 B). On the bounded-cut MPS (Sec. 14.3) the switch is two MPS sweeps with the two gate
orders and an MPS overlap (chi^2 d per site) for ov; the mixed-order form mixes only the reduced readout matrices as in
(14.18). Nothing needs a parse ancilla or controlled gates: the parse register is the loop index.

(6) Training rule. Weights: dP_c/dalpha_k = w_k (P_c^{(k)} - P_c) closed form (mixture linear in w). Coherent: c_k
are free complex parameters; lambda_{c_k} = <sig_k|lambda_S> (11.27), lambda_{sig_k} = conj(c_k) lambda_S with
lambda_S = (5.8) root seed; if per-parse normalisation psi_k = sig_k/sqrt Z_k is used, lambda_{sig_k} = (lambda_{psi_k}
- psi_k Re<psi_k|lambda_{psi_k}>)/sqrt Z_k. Words: K tree adjoints (11.28)-(11.30), root seed of parse k scaled by w_k
(forest) or conj(c_k) (coherent); shared words scatter-ADD across parses (verified rule of 11.7). Switch: seed each
chain with lambda_S/sqrt2, run the Sec. 5.3 recurrence on both orders, scatter-add V's and W's angles (each appears in
both chains). Plausibility as objective: d ln Z_k / d theta = 2 Re <sig_k| d sig_k/d theta>/Z_k = tree adjoint with
root seed lambda_sigma = sig_k/Z_k (a regulariser -beta ln Z_k or a parse-posterior CE on q_k). Sinkhorn: dL/dA[a][b]
= 2 Re <lambda_{n~_a}|n_b>, lambda_{n~_a} from (11.29); back-propagate to logits by unrolling the 10-20
normalisations (each is y = x/sum x: lambda_x = (lambda_y - <y,lambda_y> 1)/sum x). Anneal A toward one-hot
(temperature on L) to recover a discrete diagram. Parameter-shift is unnecessary; on hardware only (b) has a
shift-rule (per parse), (c)-(e) do not.

(7) Minimal test (fp64, ps.c; fp32 tolerance 1e-6). Words: Alice = Sec. 8 (0.3, 0.7, 1.1); man = Sec. 8 Bob angles
(0.5, -0.4, 0.9); telescope = (-0.2, 1.3, 0.4); sees = IQP (0.8, -0.6) (= "loves"); with_N: n^r n n^l IQP (0.25,
-1.0); with_V: s^r s n^l IQP (-0.7, 0.35). Sentence "Alice sees man with telescope", (1,1).
  Parse 0 (NP attachment): sig = [-0.08456503 - 0.13186831i, -0.05857063 - 0.22622780i], Z = 0.07915003, p = [0.31005035, 0.68994965].
  Parse 1 (VP attachment): sig = [-0.08147592 - 0.13190363i, -0.15706525 - 0.08833881i], Z = 0.05651013, p = [0.42535546, 0.57464454].
  (a) q = softmax(ln Z) = [0.58344344, 0.41655656]; argmax = 0.
  (b) alpha = (0.4, -0.2): w = [0.64565631, 0.35434369], P = [0.35090799, 0.64909201]; dP_0/dalpha = [-0.02637999, +0.02637999] (closed form = central FD to 1e-8).
  (c) c = (1,1)/sqrt2 on normalised psi_k: G01 = 0.79947603 - 0.44779676i, ||S||^2 = 1 + Re G01 = 1.79947603, P_coh = [0.40612163, 0.59387837] vs P_mix = [0.36770291, 0.63229709]; interference contributions [0.20178276, 0.24249992].
  (d) Switch, d = 2, 2 wires, Psi0 = Bob (x) Alice (Alice = wire 0, low bit), V = H H CRz(0.8)(0->1), W = Ry(0.5)_0 Ry(-0.4)_1 CNOT(0->1): <WV Psi0|VW Psi0> = 0.33161390 + 0.28241544i, p(+) = 0.66580695, ||Sw||^2 = 1.33161390; wire-0 readout coherent [0.55289739, 0.44710261] vs mixed order [0.50711082, 0.49288918].
  (e) Sinkhorn, logits [[0.6,-0.3],[-0.1,0.2]], 20 iterations: A = [[0.64565631, 0.35434369],[0.35434369, 0.64565631]]; sigma_soft = [-0.03731432 - 0.26159592i, 0.06661900 - 0.24173645i], Z = 0.13269939, L(y=0) = 0.64209732; dL/dA = [[-0.05361639, 0.09769544],[0.11317156, -0.06210987]] (adjoint = FD to 1e-8). Hard references: Z(Alice,Bob) = 0.14026553 (Sec. 8), Z(Bob,Alice) = 0.16045040.

(8) Caveats / UNVERIFIED.
  - Z is a plausibility signal, not grammaticality; E[Z] = 2^{-2 q_n} for Haar words (E-1) and 53 % of (2,1) IQP L=1 sentences sit below 2^-6: compare ln Z only across parses of the SAME sentence (same cup count) or after the log-domain path; fp32 underflow at long sentences is real on HVX.
  - Sinkhorn mean-field != superposition of derivations: sigma_soft = t^2 s(A,B) + (1-t)^2 s(B,A) + t(1-t)[s(A,A)+s(B,B)]; at t = 0.646 the "same noun in both slots" part ([-0.024-0.110i, 0.021-0.110i]) is as large as the derivation part ([-0.013-0.151i, 0.045-0.131i]). Use (11.25)-style Birkhoff weights over enumerated matchings when interpretability matters (n <= 3: 6 derivations); Sinkhorn only for n >= 4 with annealing. The K-parse enumeration itself is exponential (E-12).
  - Coherent superposition (c) contradicts the programme's own semantics for ambiguity (mixing); it is a modelling choice to be validated on data, and its phases c_k are unidentifiable without the Gram term.
  - Quantum-switch gate sets for DisCoCirc are UNVERIFIED (Addendum 14.1 (U)); the switch is only meaningful when V, W do not commute (diagonal Frobenius gates (14.3) commute: ov = 1, nothing to learn).
  - Literature negatives (no coherent-parse or switch QNLP paper) are from memory, network unavailable: UNVERIFIED.

---------------------------------------------------------------------------------------------------------------
## 3. Already done classically (tier C): do not re-investigate

Twenty-seven candidates were filed as tier C by the verifiers: the capability either was the ORIGINAL classical
form of the model, is implemented in a released classical backend, or was rejected on inspection. Where it lives:
lambeq classical path = lambeq NumpyModel / PytorchModel / PennyLaneModel / PytorchQuantumModel with TensorAnsatz,
SpiderAnsatz, MPSAnsatz (verified from source on raw.githubusercontent.com by the historians; docs blocked);
DisCoPy = Tensor.eval, CQMap / channel.py (CPM doubling); QI-IR = quantum-inspired information retrieval / NNQLM /
CNM / QPDN line; TN-NLP = tensor-network NLP (MPS/MPO verbs, low-rank composition, Wijnholds et al.). "hw vote"
marks the three that drew a hardware-limited vote from one verifier but were still already done off-device.

    #   candidate                                                       one-line reason                                                              where it was done
    --- --------------------------------------------------------------- ---------------------------------------------------------------------------- ------------------------------------------------
    C1  Frobenius merge/delete spiders for relative pronouns,           spiders are index equalities / diagonal contractions; (11.6)-(11.13')        Sadrzadeh-Clark-Coecke 2013/2014; lambeq SpiderAnsatz;
        possessives, anaphora (who / which / whose / that)              are the classical model, run since 2013                                      DisCoPy spiders; addendum Sec. 11.3
    C2  Coordination: "and" as Frobenius merge on sentence / noun       classical for a decade; the control-qubit-and-trace construction was only    Kartsaklis 2016 coordination; lambeq classical path;
        wires, "or" and hedging as convex mixtures                      ever run as a density matrix in numpy                                        addendum (11.16)-(11.17')
    C3  Arbitrary (non-unitary, non-square, unnormalised) word          arbitrary tensors ARE the original DisCoCat functor into FVect; verbs as       CSC 2010; Grefenstette-Sadrzadeh 2011; lambeq
        tensors; transitive verbs as N(x)S(x)N maps                     maps are the 2011-2015 relational / Frobenius verb models                     TensorAnsatz (Dim per type)
    C4  Density-matrix word meanings for lexical / syntactic            born classical: count-based rho, entropy as ambiguity                         Piedeleu et al. 2015; Meyer-Lewis 2020 (PyTorch);
        ambiguity with von Neumann entropy                                                                                                           Eisinger et al. 2025 (VUB)
    C5  Graded hyponymy / entailment via the Loewner order              computed in numpy on count / neural rho; benchmark datasets classical         Bankova et al. 2016/2019; Lewis 2019 RANLP;
        (k-hyponymy, k_BA, k_E) lifted to sentences                                                                                                  Balkir et al. 2016
    C6  Logical and conversational negation as complement in the        candidate text concedes it was computed classically; I - rho is not CP so     Lewis 2020; Rodatz-Shaikh-Yeh 2021 (numpy)
        Loewner order, weighted by worldly context                      hardware was never the reason
    C7  Postselected cups (Bell effects): pay 2^{-q} (x Z) per cup,     hw vote; on a device each cup accepts w.p. 2^{-q} x (its share of Z),         spec Sec. 1.3 / 2.3; lambeq classical path;
        or drop it entirely                                             E[Z] = 2^{-2 q_n} per N TV N (E-1), so E[P_post] = 2^{-4 q_n} (1/16 at         DisCoPy Tensor.eval
                                                                        (1,1), 1/256 at (2,1)); on a simulator a cup is an index contraction
                                                                        (Sec. 1.3, C8) and the cost only exists on devices; every classical
                                                                        backend drops it
    C8  Exact Born probabilities instead of multinomial shot            every classical backend returns exact probabilities; the shot floor is a      lambeq NumpyModel / PytorchModel; Sec. 1.5
        estimates                                                       device artefact, not a capability
    C9  Swap / Hadamard test replaced by a complex dot product and      the dot product is what the 2011 experiments computed; the swap test is a      Grefenstette-Sadrzadeh 2011; Sec. 1.5; Theme A3
        a dense Gram matrix                                             device workaround for it (Theme A3 keeps only the trained-objective form)     keeps the batched trained form
    C10 Frobenius spiders (copy / merge) realised as diagonal           same as C1: diagonal contraction is the definition in FVect                    Sadrzadeh et al. 2013; addendum (11.6)-(11.9)
        contractions instead of postselected GHZ effects
    C11 Shot-free exact gradients through postselected                  adjoint / autodiff through the (5.1) quotient is standard in lambeq's          lambeq NumpyModel (JAX), PytorchModel; DisCoPy
        probabilities (adjoint replaces SPSA / parameter shift)         classical and PennyLane backends                                              autodiff (Toumi et al. 2021); spec Sec. 5.3
    C12 Long sentences with many cups: postselection-free               tensor contraction has no postselection; contraction-order cost, not          lambeq tensornetwork / opt_einsum path; addendum
        contraction                                                     hardware, is the limit (Sec. 11.4, 11.6)                                      Sec. 11.4
    C13 All-to-all connectivity: no SWAP routing overhead               routing is a compiler (tket) cost that never existed off-device; nothing       tket / pytket; the simulator has no topology
                                                                        to unlock
    C14 Native qudit types: d_n = 3..256 without qubit padding          arbitrary d was the classical DEFAULT (d_n ~ 100-2000)                        CSC 2010; Grefenstette-Sadrzadeh 2011; lambeq Dim
    C15 Exact reverse-mode (adjoint) gradients replacing SPSA /         same as C11 for DisCoCat sentence circuits                                    lambeq NumpyModel / PennyLaneModel / PytorchQuantumModel
        parameter shift for DisCoCat sentence circuits
    C16 Training the renormalised postselected state (cups as Bell      lambeq's quantum models normalise the postselected output and backprop         lambeq QuantumModel / PytorchQuantumModel source
        effects) with exact conditional gradients                       through the ratio (verified from source)                                      (raw.githubusercontent.com)
    C17 Standard deterministic optimisers (L-BFGS with line search,     lambeq exposes PyTorch optimisers and a full-batch exact loss; nothing         lambeq trainer / PytorchTrainer; any autodiff stack
        full-batch Adam) on deterministic losses                        prevents L-BFGS
    C18 Arbitrary local dimension per grammatical type (d not a         same as C14; candidate inverts the history                                     CSC 2010 ... lambeq TensorAnsatz ob_map
        power of 2)
    C19 Arbitrary (non-unitary) complex word maps: general matrices,    same as C3; the original model; lambeq tensor ansaetze are unconstrained        lambeq TensorAnsatz / MPSAnsatz; TN-NLP
        isometries, contractions, rank-deficient / rectangular maps
    C20 Non-unitary CP composition rules on positive operators:         defined and computed classically (word / phrase level); the in-circuit          Coecke-Meichanetzidis 2020; Lewis 2019/2020;
        Fuzz, Phaser, Compression, KMult / BMult, meaning updating      trained form is Theme A1, the word-level form is done                           De las Cuevas et al. 2020 (numpy)
    C21 Non-Hermitian observables, non-orthogonal / incomplete          projection-based negation and non-orthogonal projectors are the QI-IR /         Widdows-Peters 2003; Sordoni 2013; Li et al. 2019
        "measurements", projection-based negation                       Widdows line, classical from the start                                          CNM (QI-IR)
    C22 Complex-valued embedding networks with learned density-matrix   NNQLM / CNM / QPDN are classical PyTorch models built on quantum probability;    Zhang et al. 2018 (NNQLM); Li et al. 2019 (CNM);
        construction and trainable non-orthogonal projectors            never hardware-shaped                                                           QPDN (QI-IR)
    C23 Density-matrix / fidelity-based attention and quantum-          trace inner products and vN divergence as similarity are the QI-IR /            Sordoni et al. 2013; Zhang et al. 2018; Li 2019
        probability ranking                                             quantum language model line, classical                                          (QI-IR)
    C24 Large-vocabulary high-rank DisCoCat verbs / DisCoCirc boxes     full per-word tensors were the ORIGINAL model; the low-parameter ansatz is       Grefenstette-Sadrzadeh 2011; Fried et al. 2015
        with per-word FULL tensors instead of a shared ansatz           the hardware-era departure from it (Theme A8 keeps the streamed-kernel form)    (low-rank); lambeq TensorAnsatz; TN-NLP
    C25 Postselection-free exact evaluation of long DisCoCat /          hw vote; how QNLP has always been evaluated off-device (tensor contraction);     lambeq classical path; DisCoPy; addendum Sec. 11.4
        DisCoCirc sentences (cups as exact index contractions)          the ceiling is contraction cost (Sec. 11.6), not hardware
    C26 DisCoCirc question answering on whole texts without noun        the classical training backend of Duneau et al. 2024 evaluated the full text;   Duneau et al. 2024 (classical simulation side);
        filtering or qubit reuse                                        noun filtering / reuse were device-side compilation only                         lambeq PytorchQuantumModel text circuits
    C27 Exact adjoint-gradient training in place of SPSA /              hw vote; same as C11 / C15 (third phrasing of the same candidate)                lambeq classical backends; Duneau 2024 training
        parameter shift with shot noise

What survives from tier C into the Table A themes is never the semantics or the algorithm but the FORM: register-
resident, exact-gradient, batched, inside a text circuit rather than on a stored numpy tensor. C7 / C25 / C27 are
the three "hardware vote but already done" cases: the verifier who voted hardware-limited was describing the
device cost correctly, and the other two verifiers pointed out that the classical backend had already removed it.

---------------------------------------------------------------------------------------------------------------

## 4. Cross-cutting principle: why these unlock classically, and what each primitive costs in bytes

The simulator exposes the amplitude vector and the density matrix AS DATA. Everything a device has to estimate
through measurement, and everything a device cannot do because it is not a unitary, becomes an array operation on
that data. Every Table A theme reduces to the same short list of primitives, and each has a working set that is a
closed-form function of W (live qubits), d = 2^{q_n} (local dimension), B (batch) and P (parameters) -- consistent
with the spec's Sec. 7 rows and the addendum's Sec. 16 tiers (C14: REG ~1 KB CUDA thread / 4 KB HVX; VTCM 8 KB
slice; SMEM 48-227 KB / Adreno 32 KB; HBM beyond).

    device limitation                      what it is on the simulator                    primitive                          working set, bytes (fp32 complex = 8 B)                      (1,1) / (2,1) examples                        tier
    -------------------------------------- ---------------------------------------------- ---------------------------------- ------------------------------------------------------------ --------------------------------------------- -------------
    finite shots / no amplitude access     amplitudes are the state array                 elementwise |psi|^2                8 x 2^W state (4 x 2^W fp16 store); 0 extra                   W = 3: 64 B;  W = 6: 512 B                    REG
    tomography for a reduced state         partial trace = strided Hermitian reduction    rho_A = P P^H (SF.1)/(T.1)         + 4 d^2 B per queried wire (Hermitian packed);               16 B / 64 B per wire; two wires 64 B / 1 KB   REG
                                           over the state array                                                              two-wire d^2 x d^2: 4 d^4 B
    no entropy / spectrum estimator        eigenvalues of a d x d Hermitian matrix        2x2 closed form; complex Jacobi    in place + 8 d^2 B for V when eigenvectors are needed;       0-16 B / 128 B (+128 B V); d = 16: ~6 KB      REG -> VTCM/SMEM
                                                                                          (SF.2)/(SF.3)                      + 4 d B eigenvalues
    swap / Hadamard test per pair          inner products are reductions; a batch is a    Hermitian GEMM Psi Psi^H (G.2)     Psi 8 B D + G 8 B^2 (F only: 4 B^2); streamable to 12 B      B = 32, D = 2: 512 B + 4-8 KB; 12 B/anchor    REG (streamed)
                                           Gram matrix                                                                       per anchor                                                   streamed                                      -> SMEM (full G)
    postselection: Bell cup accepts w.p.   dropping array entries; a top-field slice is   index filter / pointer offset      0 extra for the top field (HO.6); 8 x 2^{W-q} for an          0 / 0 (top field)                             REG
    2^{-q} per cup (C8) TIMES the          a pointer offset                               (HO.6)-(HO.7)                      interior field copy; exact DFS over S sliced wires:
    contraction norm Z, E[Z] = 2^{-2 q_n}                                                                                    (S+1) x 8 x 2^W
    (E-1): expected acceptance per N TV N
    sentence P_post = Z 2^{-2 q_n}, E[P_post]
    = 2^{-4 q_n} = 1/16 at (1,1), 1/256 at
    (2,1), i.e. 4^{-q} per cup (Table A, A7)
    unitary-only evolution / no CPTP       a non-unitary map is a dense d x d gate on     small dense complex gemv on        + 8 d^2 B transient gate; single-Kraus on the pure state:     32 B / 128 B gate; doubled register W = 2:    REG (pure) ->
    without ancilla + discard              the ket (single Kraus) or a pair of gates on   strided blocks; Schur layer        no extra; doubled register 8 x 4^W + 8 d^2                   160 B, W = 4: 2.2 KB, W = 7: 128 KB           SMEM (doubled)
                                           the doubled register                          (NU.4)
    discard needs mid-circuit measurement  strided sum over the environment field         strided reduction (13.7 form)      accumulator 8 d^2 B                                          32 B / 128 B                                  REG
    no free non-CP maps (negation)         affine array op I - rho at word level          elementwise                        in place on 8 d^2 B                                          32 B / 128 B                                  REG
    controlled sub-circuit = many CNOTs    a predicate on the block index                 masked gemv (HO.2)                 0 extra (same gate)                                           0 / 0                                         REG
    parameter shift + shots per parameter  adjoint pass: three state vectors              butterflies run backwards (5.3)    24 x 2^W (gradient), 48 x 2^W (Hessian tangents);            W = 3: 192 / 384 B; W = 5: 768 B / 1.5 KB;    REG -> SMEM/VTCM
                                                                                          (11.27)-(11.30)                    + 8 x 2^{q_s} P (Jacobian) + 4 P^2 (metric)                  W = 12: 96 / 192 KB
    O(P^2) tests for the metric            Gram of the Jacobian                           J^H J, P x P solve                 8 x 2^{q_s} P + 4 P^2 B                                      P = 8: 128 + 256 B; P = 256: 256 KB           REG -> SMEM
    qubits only                            radix is a stride table                        mixed-radix butterfly (12.16')     8 D + 8 d_j^2 B, D = prod d_j                                (3,2) tree: 168 B; 2 wires d = 3: 72 B + 72 B REG
    no QRAM for amplitude encoding         a gathered table row                           batched GEMV with gathers          8 d_n per noun, 8 d_n^2 d_s per dense verb (or 8 (2 d_n r     16 + 64 B / 32 + 256 B per sentence;          REG (rows) /
                                                                                                                             + r^2 d_s) per MPO verb); table V x row in HBM / VTCM         50 K nouns at d_n = 16: 6.4 MB                HBM (table)
    one question per execution             the state is reused; all RDMs in one pass      strided Gram + WHT butterfly       + 4 d^2 B per wire + m Q x 4 B answers; WHT in place         m = 3, W = 3: 148 B; W = 6: 228 B             REG

Reading of the table. At (1,1) every primitive is register-resident for texts of up to W = 6-7 live qubits; at
(2,1) the state (8 x 2^W) and the doubled register (8 x 4^W) are what move a kernel out of the register tier, never
the per-wire objects (d^2 = 16 complex = 128 B). The doubled register is the only primitive with a hard ceiling
inside on-chip memory (8 (4^W + d^2) B: W = 3 in a CUDA thread, W = 4 in an HVX context or an 8 KB VTCM slice,
W = 5 in Adreno 32 KB local, W = 7 at 128 KB in CUDA shared with the > 48 KB dynamic opt-in; W = 5 / 6 miss the 8 KB /
32 KB tiers by the 32 B gate tile alone, Sec. 2.1 (4)), and it is needed only for fuzz / rank > 1 Kraus words applied
mid-text; single-Kraus updates, slices, controlled boxes and every readout functional cost nothing beyond the pure
state plus d^2 accumulators. Precision rules carry over unchanged: probabilities, RDMs, Gram entries and every
reduction accumulate in fp32 (C12); entropies clamp lam <= 1e-7 f and never run in fp16; postselection and
non-unitary traces T are carried as log sums and divided out once at readout (Sec. 4.5, Z_min guard of (5.1));
second-order training is fp32 only (Sec. 5.3). The kernel set is therefore the spec's existing one (Sec. 2.1/2.2
butterflies, Sec. 1.3 fused tensordot, Sec. 11.7 tree adjoint, Sec. 14.1 gate list, Sec. 14.3 MPS) plus four small
additions: a dense d x d complex gemv on strided blocks with an optional control predicate (HO.1), a strided
Hermitian outer-product accumulate (SF.1)/(T.1), a d x d complex Jacobi with the 2x2 closed form (SF.2)/(SF.3), and
a Hermitian GEMM over a batch (G.2). Nothing materialises a 2^W x 2^W matrix and nothing stores a unitary.

---------------------------------------------------------------------------------------------------------------

## 5. Sources

Verification status in this run: no arXiv / ACL / Springer / Quantinuum documentation could be fetched by any
worker; the lambeq source tree was fetched from raw.githubusercontent.com and a copy sits in the scratchpad
(lambeq_src/, lambeq_ansatz_*.py, lambeq_training_*.py); the "QNLP in practice" PDF was obtained through the CQ
mirror by one verifier (scratchpad/qnlp.pdf, qnlp.txt); abstracts of arXiv:2408.06061, 2409.08777, 1106.4058 and
ACL D11-1129 were confirmed via search snippets. Everything else is from memory: identifiers and specific claims
marked UNVERIFIED should be checked before publication.

DisCoCat and classical composition
- Coecke, Sadrzadeh, Clark 2010, "Mathematical foundations for a compositional distributional model of meaning",
  arXiv:1003.4394.
- Grefenstette, Sadrzadeh 2011, "Experimental support for a categorical compositional distributional model of
  meaning", EMNLP, ACL D11-1129 / arXiv:1106.4058 (abstract confirmed).
- Kartsaklis, Sadrzadeh, Pulman 2012; Kartsaklis, Sadrzadeh 2013; Milajevs et al. 2014 (evaluation of composition
  by cosine; from memory).
- Sadrzadeh, Clark, Coecke 2013/2014, Frobenius anatomy of relative pronouns (from memory).
- Fried, Polajnar, Clark 2015, low-rank verb tensors; Wijnholds et al. 2020 (from memory).
- Kartsaklis 2016, coordination in categorical compositional semantics (from memory).
Density matrices, entailment, negation
- Piedeleu, Kartsaklis, Coecke, Sadrzadeh 2015, "Open system categorical quantum semantics in natural language
  processing", arXiv:1502.00831.
- Blacoe, Kashefi, Lapata 2013 (quantum-theoretic word meaning, from memory).
- Balkir, Sadrzadeh, Coecke 2016, entailment with relative entropy, arXiv:1506.06534 (attribution per verifier
  correction, UNVERIFIED).
- Bankova, Coecke, Lewis, Marsden 2016/2019, graded hyponymy and the Loewner order, arXiv:1601.04908.
- Lewis 2019 (RANLP) k-hyponymy at sentence level; Lewis 2020 negation; De las Cuevas et al. 2020; Meyer & Lewis
  2020 (PyTorch density matrices); Rodatz, Shaikh, Yeh 2021 conversational negation (all from memory, UNVERIFIED
  exact forms).
- Coecke, Meichanetzidis 2020, "Meaning updating of density matrices", arXiv:2001.00862 (fuzz / phaser; UNVERIFIED
  wording).
- Eisinger, Gauderis, de Huybrecht, Wiggins 2025, syntactic ambiguity as density matrices, arXiv:2504.00040 (VUB).
Quantum-theory-inspired language models (QI-IR)
- Widdows, Peters 2003 (quantum logic negation); Widdows, Cohen (binary / real vector semantics; from memory).
- Sordoni, Nie, Bengio 2013, quantum language model (SIGIR).
- Zhang et al. 2018 NNQLM (AAAI); Li et al. 2019 CNM (NAACL); QPDN (from memory).
Circuit QNLP and hardware runs
- Zeng, Coecke 2016, "Quantum algorithms for compositional natural language processing", arXiv:1608.01406 (QRAM,
  closest-vector speedup).
- Coecke, de Felice, Meichanetzidis, Toumi 2020, "Foundations for near-term QNLP", arXiv:2012.03755.
- Meichanetzidis et al. 2020, "Grammar-aware QNLP", arXiv:2012.03756.
- Lorenz, Pearson, Meichanetzidis, Kartsaklis, Coecke 2021, "QNLP in practice", arXiv:2102.12846 / JAIR 2023 (PDF
  obtained; vocab 17 / ~115, MC = 130, 2^13 shots, SPSA, 100 iterations from the released code).
- Kartsaklis et al. 2021, "lambeq", arXiv:2110.04236 (source verified: NumpyModel, PytorchModel, PennyLaneModel,
  PytorchQuantumModel, TensorAnsatz / SpiderAnsatz / MPSAnsatz, to_tn(mixed=True)).
- Toumi, Yeung, de Felice 2021, diagrammatic differentiation, arXiv:2103.07960.
- Karamlou et al. 2022 (generative QNLP, tiny vocabulary; from memory).
- Castillo et al. 2025, classical ansatz comparison, arXiv:2502.20744 (from memory).
DisCoCirc
- Coecke 2019/2020, "The mathematics of text structure", arXiv:1904.03478.
- Wang-Mascianica, Liu, Coecke 2023, "Distilling text into circuits", arXiv:2301.10595.
- Laakkonen, Meichanetzidis, Coecke 2024, "Quantum algorithms for compositional text processing" (QDisCoCirc),
  arXiv:2408.06061 (abstract confirmed; BQP-completeness vs hardness UNVERIFIED).
- Duneau, Bruhn, Matos, Laakkonen, Saiti, Pearson, Meichanetzidis, Coecke 2024, "Scalable and interpretable QNLP:
  an implementation on trapped ions", arXiv:2409.08777 (abstract confirmed; 108 -> 20 qubit reuse and readout
  details UNVERIFIED).
- Duneau et al. 2025, interpretability of DisCoCirc, arXiv:2507.02940 (from memory).
- Krawchuk, Khatri, Ortega, Kartsaklis 2025, text-to-circuit pipeline, arXiv:2505.13208; lambeq Gen II blog (May
  2025) (search snippets).
- Waseem, Liu, Wang-Mascianica, Coecke 2022, DisCoCirc across languages, arXiv:2208.10281; arXiv:2504.09909
  (follow-up; search snippets).
Training, metrics, landscapes
- Stokes, Izaac, Killoran, Carleo 2020, "Quantum natural gradient", Quantum 4, 269.
- Mari, Bromley, Killoran 2021, second-order shift rules, PRA 103, 012405, arXiv:2008.06517.
- McClean et al. 2018, barren plateaus; Ragone et al. 2024, Nat. Commun., s41467-024-49909-3.
- arXiv:2608.16939 ("BuresQNG" QNLI posters, 2026): UNVERIFIED, not relied on.
Quantum information primitives cited
- Acharya et al., entropy estimation sample complexity, arXiv:1711.00814 (UNVERIFIED id).
- Brydges et al. 2019, randomized-measurement Renyi entropies; Miszczak et al. 2009, fidelity bounds (from memory).
- Chiribella et al. 2013, quantum switch (from memory).
- Buzek, Hillery, universal NOT (from memory).
- Reck et al. 1994 / Clements et al. 2016, U(d) decompositions (order UNVERIFIED in detail).
Classical structure learning referenced in Theme B
- Kim et al. 2017 (structured attention); Mena et al. 2018 (Gumbel-Sinkhorn); DIORA 2019 (from memory).

Reference programs (all fp64 plain C, in the scratchpad): nonunit/nu.c; spec/spec_test.c; jacobi.c; gram/gram.c;
ptrace/ptrace.c, grad.c, ptrace2.c; discocirc_ho/ho2.c; exact_train/et.c; wirerep/qd.c; lexicon/lex_test.c,
fid_test.c; parsestruct/ps.c; critic2/extra.c (T-W4, the (2,1) reference of Secs. 2.2 (7), 2.4 (7), 2.5 (7)). No
Python program is part of the reference set; chk.py and qng_check.py in the scratchpad are earlier verifier scratch
files, not cited by, and not needed to reproduce, any number in this report; the lambeq_*.py files and lambeq_src/ are
the fetched lambeq source that the historian READ for the tier-B/C provenance notes, never executed. The independent
recomputation used for the completeness audit is scratchpad/critic2/recompute.c and extra.c (fp64, plain C).
