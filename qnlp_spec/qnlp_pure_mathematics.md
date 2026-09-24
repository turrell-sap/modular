# Mathematics of compositional quantum-theoretic language models

Version 1.1 (pure mathematics companion to the specification, errata v1.1, addendum and hardware-limits documents). Version 1.1 corrects Proposition 4.37(c), completes the proofs of Proposition 4.37(b) and Corollary 7.7, removes every forward dependency from proofs, completes the notation table, and proves several engineering statements previously marked "asserted only" (Example 1.30', Corollary 2.32', Proposition 4.35', Corollary 5.29').

## Abstract

This document gives the mathematics under a programme that interprets natural-language sentences and texts as tensor contractions or parameterised circuits and evaluates them by classical simulation. It contains definitions, lemmas, propositions, theorems and proofs only, and it is self-contained: a reader needs no other document.

Chapter 1 treats pregroup grammars. Reductions are exactly the planar admissible matchings. A CKY-type chart decides grammaticality and counts parses exactly, and we give its exact cost. The stack reducer is sound and can emit every reduction, but it is incomplete, and we characterise when it succeeds.

Chapter 2 builds the functor from the free rigid monoidal category of a pregroup to finite-dimensional Hilbert spaces. It proves the component formula for sentence meanings and the exactness of the "bent-wire" circuit, and gives the Frobenius-algebra (spider) semantics of relative pronouns and coordination, the DisCoCirc text register and the completely-positive-map construction.

Chapter 3 fixes the Hilbert-space semantics. It covers index maps of gates, the gate set (periodicity, transposes, realness), postselection, and the exact law Z ~ Beta(d_s, d_V - d_s) of the sentence normaliser for a Haar-random verb, with E[Z] = 2^{-2 q_n}. It also covers fidelities, reduced states, entropies and the Loewner order.

Chapter 4 treats contraction schedules (fused pairwise contraction, word-tree peaks, cut width), Frobenius nodes, matrix product states and operators, and the product approximation.

Chapter 5 differentiates everything above: exact parameter-shift rules and their exactness criterion, the postselected quotient rule and its 1/D bound, adjoint (reverse-mode) differentiation on circuits and word trees, signs for transposed words, periodicity, SPSA with the corrected variance, second-order objects, and the saddle at the origin of IQP models.

Chapter 6 is numerical analysis: rounding models, linear error growth under unitary evolution, fixed-point safety from unitarity, non-expansive saturation, the tight postselection-amplification constant 1 (not 4), and floating-point summation.

Chapter 7 treats capacity: dimensions of state manifolds and ansatz images, the readout as a rational trigonometric function, multilinearity and the impossibility of switching inside one evaluation, word-order sensitivity, low-rank verbs and gradient variance.

Everything not proved is labelled Conjecture or UNVERIFIED and collected in Appendix B. Appendix A maps each result to the engineering equations it underpins and back. Appendix C lists the standard results cited.

## 0. Front matter

### 0.1 Conventions (stated once, used throughout)

- **(C1) Little-endian index.** A W-qubit register is C^N, N = 2^W. Its basis vector e_i = |i> has i = sum_k bit_k(i) 2^k, where bit_k(i) = floor(i / 2^k) mod 2. Qubit 0 is the least significant bit. [spec C1]
- **(C2) Kronecker order.** Only where the text says "Kronecker order", (A (x) B)[r_A dim B + r_B][c_A dim B + c_B] = A[r_A][c_A] B[r_B][c_B]. The LEFT factor carries the HIGH digits. [spec C2]
- **(C7-identification; default meaning of (x)).** Everywhere else, a tensor product of word spaces, word states or fields means the index identification of spec C7: the FIRST listed factor occupies the LOWEST bits. For example, a word with factor values v_0, ..., v_{m-1} has flat index sum_j v_j 2^{fo[j]} (Lemma 2.15). Operators written with qubit or field subscripts (H_a, <0|_K, |0>_a, Pz_ctl) name the qubits they act on and imply no order. This is not the C2 order. Linguistic order and Kronecker order are opposite. [spec C2, C7]
- **(C3) Brackets.** M[r][c] is row-major (row index first). T[x, y, z] with commas is a field-indexed tensor whose first index is in the lowest bits (C7).
- **(C4) Order of operators.** Operators act from the left, psi' = U psi. Products of operators are read right to left. A gate list g_1, ..., g_m (g_1 applied first) denotes the operator U = g_m ... g_1. [spec C3, C4]
- **(C5) Rotations.** R_G(theta) = exp(-i theta G/2) for Hermitian G. For a Pauli string P, R_P(theta) is the usual rotation. The letter P denotes a Pauli string only in R_P(.) and where a statement declares it (Lemmas 5.18, 6.11). Rx, Ry, Rz are R_{Px}, R_{Py}, R_{Pz}. [spec C5]
- **(C8) Cups.** The cup pairs bit r of one field with bit r of the other (component-wise pairing). The nested pairing (bit q-1-r with bit r) is an equally valid alternative (Proposition 2.11). Which one the reference implementation uses is UNVERIFIED. Every result holds for either choice after replacing k by bitreverse_q(k) in the second field. [spec C8]
- **(C9) Reserved letters.** D always means the postselection denominator D(theta) = sum_c N_c, never a gate count; the number of gates of a circuit is Ng. The letter u without argument or subscript always denotes a unit roundoff (Chapter 6 and Proposition 5.35); subscripted or argument-carrying symbols such as u_k, u_{rc}, u_{a0} and u(dd) are unrelated local symbols. [spec C9]
- **(C10) Ansatz identifiers.** 0 = IQP without closing H layer; 1 = IQP with closing H layer; 2 = Sim14; 3 = Sim15; 4 = Ry-only (Definition 3.17). [spec C10]
- **(C11, C13).** A word used as an effect is applied as the transposed gate list (Lemma 2.22). Controlled rotations have generators |1><1|_ctl (x) P_tgt (Lemma 3.13). [spec C11, C13]
- **(C17) DisCoCirc wires.** Noun wire j occupies field [q_n j, q_n (j+1)), and wires are numbered by first mention. A two-wire gate has local index m = a + d b, with a = subject (low) and b = object (high). [addendum C17]
- **Adjoint.** Written ^dag (conjugate transpose). ^T is the plain transpose. conj is entrywise complex conjugation.
- **Bit operations.** x >> s = floor(x / 2^s); x << s = x 2^s; &, | and xor are bitwise and, or and exclusive or on non-negative integers; ~ is bitwise complement within the register width; popcount(x) is the number of 1 bits. [P] is the Iverson bracket: 1 if P holds, else 0. delta_{ij} = [i = j].
- **Logarithms.** log2 is used for entropies stated in bits, and ln for natural units. Unsubscripted log in Chapter 5 is natural.

**Citation labels.** Engineering documents are cited as:

- [spec (x.y)], [spec Cn], [spec Tn], [spec Sec. x], [spec l.NNN] for the first-principles specification;
- [addendum (x.y)], [addendum Cn], [addendum Tn], [addendum Sec. x], [addendum l.NNN];
- [errata E-n] for errata v1.1, and [errata Sec. B] for its "no change" section;
- [review n] for the review of the companion v1.1;
- [limits Sec. x (k)] or [limits l.NNN] for the distilled hardware-limits document.

Line numbers refer to the file versions this document was checked against.

### 0.2 Global notation table

Each symbol in the table has one global meaning. A few letters also carry a declared local meaning inside a named statement; these are listed in the block "Local re-uses" at the end of the table, and no other re-use occurs. Where an engineering document uses a different letter, the table says so.

**Grammar (Chapter 1)**

| Symbol | Meaning |
|---|---|
| B | finite set of basic types, discrete order: B = {N, S} [spec Sec. 1.1] or {N, S, I, Q} [addendum C15] |
| (b, z) | simple type, b in B, z in the integers; b = (b,0), b^l = (b,-1), b^r = (b,+1), b^ll = (b,-2), b^rr = (b,+2) |
| n, s | the simple types (N,0), (S,0), inside type strings only; elsewhere s is the sentence (class) index and n a declared count or dimension |
| Typ, Typ*, 1 | the set of simple types; finite strings over Typ; the empty string |
| X = t_0 ... t_{m-1} | a type string of length m; t_i = (b_i, z_i); positions are 0-based |
| X[i..j] | substring of positions i..j inclusive; empty if i > j |
| match(i, j) | i < j, b_i = b_j, z_j = z_i + 1 (admissible pair) |
| cup (i, j) | a pair i < j of a matching; span(i, j) = j - i; it encloses k if i < k < j |
| M | a cup set (matching); Red(X, p) = reductions of X to survivor position p; Red(X -> 1) = reductions to 1 |
| Der(X, tau) | union over p with t_p = tau of Red(X, p), for tau in Typ; for a target t in B we write Der(X, (t,0)) |
| ->_C, ->_E, ->* | contraction step, expansion step, reflexive-transitive closure of both |
| #A, Cat_k | cardinality of a finite set A; the k-th Catalan number |
| R[i][j], C[i][j] | chart entries: reducibility bit and exact count (Definition 1.9) |
| Ptest(m) | number of partner tests of the chart (Theorem 1.11) |
| n_w | number of words of a sentence; M_lat = number of typed lattice positions (Proposition 1.15) |
| nd(M) | nesting depth: max over cups of (1 + number of enclosing cups) (Definition 1.27) |

**Spaces, words and sentences (Chapters 2-4)**

| Symbol | Meaning |
|---|---|
| q_n, q_s | qubits per noun wire and per sentence wire; q(b^{(k)}) = q(b) for every winding k |
| d = 2^{q_n}, d_s = 2^{q_s} | noun and sentence dimensions |
| Q_w | number of qubits of word w; Q_V = 2 q_n + q_s; d_V = 2^{Q_V} = d^2 d_s |
| fo_w[j] | bit offset of factor j (0-based) of word w: sum_{j' < j} q(t_{j'}) |
| off_w, start(p), width(p) | word offset in the register; start bit and width of type position p [spec C7] |
| phi_w | word state in C^{2^{Q_w}}, phi_w = U_w(theta_w)|0...0> |
| n1, n2, V[a,s,b] | noun states; verb tensor V[a,s,b] = phi_V[a + d s + d d_s b] (a = n^r low, b = n^l high) [spec (1.5)] |
| V_s | the d x d matrix V_s[a][b] = V[a,s,b] |
| Psi | the product state of all words of a sentence, Psi[i] = prod_w phi_w[(i >> off_w) & (2^{Q_w} - 1)] |
| sigma | the sentence vector in C^{d_s} (only meaning of sigma) |
| Z = sum_s abs(sigma[s])^2 | sentence norm square (only meaning of the letter Z alone) |
| N_c, D, p_c | N_c = abs(sigma[c])^2 (or the corresponding circuit quantity); D = sum_c N_c; p_c = N_c / D [spec (1.6), (5.1)]; p_c is the P_c of spec (5.1) |
| n_cl = 2^{q_s} | number of classes |
| <cup_q|, |cap_q> | sum_{k < 2^q} <k| (x) <k| and its transpose |
| bra_T(phi) | the unconjugated row vector sum_a phi[a] <a| (no complex conjugation) |
| K, M_post, Surv | postselected qubit set; mask OR_{k in K} 2^k; surviving index set {i : (i & M_post) = 0} |
| Pi_K (Pi when K is fixed) | orthogonal projector onto span{e_i : i in Surv} (Definition 3.24); the only meaning of Pi |
| F_S, f_S, b(c) | survivor field, its start bit, and the readout index b(c) = c << f_S |
| W, N = 2^W | register width in qubits and dimension (W_hw = width of the bent-wire register) |
| p_surv, P_post | survival probability of a unit state; postselection probability of the Bell-circuit form |
| wire_map_e | qubit relabelling of an effect word (Definition 2.23) |

**Categories (Chapter 2)**

| Symbol | Meaning |
|---|---|
| (Cat, (x), I) | strict monoidal category, unit object I; g o f = g after f |
| gamma_{A,B} | symmetry (swap) A (x) B -> B (x) A |
| cup_A, cap_A | counit A (x) A^* -> I and unit I -> A^* (x) A of a duality (always with object subscript) |
| mu, u_F, Delta, e_F | Frobenius multiplication, unit, comultiplication, counit (u_F = sum_i |i>, e_F = sum_i <i|) |
| delta^{(k)} | the k-legged spider tensor: 1 iff all k indices agree; delta^{(0)} = 1, delta^{(1)} = all-ones |
| Fr | the DisCoCat functor (Definition 2.16) |
| Dbl(f) | doubling f (x) conj(f); Dbl(phi) = |phi><phi| |
| Lin(A), Dens(A) | linear operators on A; density operators on A |
| Tr, Tr_R | trace; partial trace over subsystem R |

**Circuits and gates (Chapter 3)**

| Symbol | Meaning |
|---|---|
| Px, Py, Pz, I, H | Pauli matrices, identity, Hadamard (Px etc. avoid clash with Z and X) |
| Rx, Ry, Rz | R_{Px}, R_{Py}, R_{Pz} |
| co, si | cos(theta/2), sin(theta/2) in explicit gate matrices |
| ctl, tgt | control and target qubit of a controlled gate |
| U_k, U_{lo,hi} | a 2x2 matrix on qubit k; a 4x4 matrix on qubits lo < hi |
| m(i) | local index bit_lo(i) + 2 bit_hi(i) |
| ins(r, f, q, a) | bit-field insertion (Definition 3.5); i0, i00, i11, compact its instances |
| L, P_w | layer count; number of real angles of word w |
| Haar(n) | uniform probability measure on the unit sphere of C^n |
| Beta(a, b), Gamma(a, 1) | Beta and Gamma distributions |
| rho, tau | density operators |
| S(rho), S(A\|\|B), S_alpha(rho) | von Neumann entropy, relative entropy, Renyi entropy (S alone occurs only with an argument, or as the subscript of F_S, f_S) |
| h(x) | binary entropy -x ln x - (1-x) ln(1-x) |
| I(A:B) | mutual information S(rho_A) + S(rho_B) - S(rho_AB), in bits (log2) |
| k_max(A <= B) | sup{k >= 0 : k A <= B} (Definition 3.44) |
| F_root, F_sq, Fid | Uhlmann fidelity, its square, pure-state fidelity |
| T_dist | trace distance |
| A <= B | Loewner order |

**Circuits, observables and errors (Chapters 3, 5, 6)**

| Symbol | Meaning |
|---|---|
| Ng | number of gates of a circuit [spec C9] |
| n_cup, Sq | number of cups of a reduction; total cup width Sq = sum_e q_e |
| Pr | an orthogonal projector |
| Ob | a Hermitian observable |
| O_c, O_eff | the readout projector O_c = \|b(c)><b(c)\| of class c; O_eff = sum_c w_c O_c (Definition 5.1, Corollary 5.15) |
| Kr, Kr_k | Kraus operators |
| Pm | a permutation matrix (or permutation) |
| Vk | g_Ng ... g_{k+1}, the gates after gate k (Vk = I for k = Ng) |
| psi_post | normalised postselected state Pi psi / sqrt(p_surv) |
| W_B, N_B = 2^{W_B} | width and dimension of the Bell-form register, W_B = 2 Sq + q_s |

**Tensor networks (Chapter 4)**

| Symbol | Meaning |
|---|---|
| Gw = (Words, Cups) | word graph |
| cw(x) | cut width at boundary x |
| sr_cut(Psi), osr(G) | Schmidt rank across a cut; operator Schmidt rank |
| A_i[alpha][t][beta], chi_i | MPS site tensor and bond dimension |
| xi_0 >= xi_1 >= ... | singular (Schmidt) values, 0-based |
| eps_tr^2 | discarded weight sum_{k >= chi} xi_k^2 |
| r | tensor-train (MPO) rank |
| chi | bond dimension of an MPS, or the rank kept by a truncation |
| Trn_k | truncation map at step k (a map, not a trace) |
| Q_max, q_max, q_out(v) | largest word width max_w Q_w; largest merged edge width (including q_s); output field width of tree node v |
| N_e, T_cl, E_live | number of distinct nouns (wires) of a text; number of clauses; number of live entities |

**Differentiation (Chapter 5)**

| Symbol | Meaning |
|---|---|
| theta in R^{n_par} | parameter vector; n_par = number of parameters |
| alpha_k, s_k, p(k) | applied angle of gate k; alpha_k = s_k theta_{p(k)} with s_k in {+-1, +-1/2} |
| G_k, Gamma_k | generator of gate k; Gamma_k = -(i/2) G_k |
| psi_k, lambda_k | forward state after gate k; cotangent (Definition 5.12) |
| g_c, gbar, w_c | dLoss/dp_c; sum_c g_c p_c; (g_c - gbar)/D |
| Loss | the training loss (the letter L alone is the layer count, or a subsystem label as in H_L) |
| eps_reg | readout regulariser in spec (1.7) |
| Shift_h[f] | f(t + h) - f(t - h) |
| freq(f) | the set of frequencies of a trigonometric polynomial f |
| J_c[j], v_j | d sigma[c]/d theta_j; sum_c conj(sigma[c]) J_c[j] |
| F_FS, F_cl, Hess | Fubini-Study metric, classical Fisher matrix, Hessian of Loss |
| Delta_sp, c_sp, sigma_eta^2 | SPSA direction, SPSA step, evaluation-noise variance |

**Numerics (Chapter 6)**

| Symbol | Meaning |
|---|---|
| fl(x), u | rounded value (round to nearest, ties to even); unit roundoff |
| u32, u16, u_bf16 | 2^-24, 2^-11, 2^-8 |
| gamma_n | n u/(1 - n u) |
| Qa.b | fixed-point: integer x interpreted as x/2^b, a integer bits including sign |
| S15 | the pre-scale 1 - 2^-15 [errata E-6] |
| rnd_b(x) | round to nearest multiple of 2^-b (ties up) |
| sat | componentwise clamp onto the Q1.15 box [-1, 1 - 2^-15] |
| delta, err | error vector hat psi - psi and its norm err = \|\|delta\|\| |
| dU | error of stored gate entries |
| g_gate | per-gate error bound in state 2-norm |
| cg | per-gate error constant of Theorem 6.6 (error at most cg u per gate) |
| g_Q15(W), g_Q31(W) | per-gate error bounds of the Q1.15 and Q1.31 paths (Proposition 6.17) |
| eta_k, Lk | store error at step k and operator norm of the stored factor (Theorem 6.23) |
| ulp(x) | unit in the last place of the floating-point number x |
| tol, p_min, A_amp | tolerance on an output probability; survival floor; amplification factor (Corollary 6.28) |

**Capacity (Chapter 7)**

| Symbol | Meaning |
|---|---|
| d_Q = 2^Q | dimension of a Q-qubit space |
| CP^{n-1}, RP^{n-1} | complex and real projective spaces; pi_proj the quotient map |
| f_w | parameter map theta -> U_w(theta)|0> |
| rk_w(theta) | realised degrees of freedom (Definition 7.3) |
| A_w | linear map phi_w -> sigma with the other occurrences fixed |
| m_k | multiplicity of angle theta_k in the sentence circuit |
| Omega_g | frequency set of gate g |
| Var_psi(G) | <psi|G^2|psi> - <psi|G|psi>^2 |
| O_0 | traceless part O - (Tr O / N) I of an observable O |

**Local re-uses.** Each of the following letters has, besides its global meaning, exactly the local meanings listed, each confined to the statement named.

| Letter | Local meaning (statement) |
|---|---|
| K | number of merged arguments (Section 2.9); number of derivations Dv_1, ..., Dv_K (Definition 1.17) |
| L, R | subsystem labels of a bipartition, as in H_L, H_R, rho_L, I(L:R) (Theorems 3.39, 4.27; Proposition 4.28); R is also the output tensor of a contraction step (Definition 4.7, Propositions 4.11, 4.17) |
| n | a count or dimension declared in the statement (Lemma 6.4, Corollary 3.34, Theorem 7.23, Lemma 6.22) |
| P | a Pauli string, where declared (Lemmas 5.18, 6.11) |
| U, U', U'' | strings in the proof of Lemma 1.4 |

The following renames remove every other clash: strings of Lemma 1.4 are x_1, x_2 and Y'; the embedding strings of Propositions 1.28 and 1.29 are Rt_k and Ce_k; the name map of Remark 2.21 is Mn; the three-qubit W state of Proposition 4.37 is Wst; the gate of Theorem 7.31 is Wg(theta); the merged vectors of Section 2.9 are Lv_k; the uniform verb vector in the proof of Theorem 5.41 is uV.

**Letters with a local meaning.** The letters i, j, k, l, a, b, c, e, r, t, x, y are indices or dummy variables. Where a letter carries a local meaning, the defining statement fixes it for that statement only.

### 0.3 Dependency graph of chapters

An arrow X --> Y means that some result of Y cites a result of X in its statement or proof. The transitive reduction of the graph is the chain

```
   Ch1 --> Ch2 --> Ch3 --> Ch4 --> Ch5 --+--> Ch6
                                          |
                                          +--> Ch7
```

and its full edge set is the following table (row Y, column X, "x" if Y cites X). The table is strictly lower triangular, so the graph is acyclic.

```
             Ch1  Ch2  Ch3  Ch4  Ch5
     Ch2      x
     Ch3           x
     Ch4      x    x    x
     Ch5           x    x    x
     Ch6                x         x
     Ch7           x    x    x    x
```

As an adjacency list (prerequisites of each chapter):

- Ch2 <- Ch1.
- Ch3 <- Ch2.
- Ch4 <- Ch1 (Definition 1.27 in Theorem 4.18), Ch2, Ch3.
- Ch5 <- Ch2 (Lemma 2.22 in Proposition 5.19), Ch3, Ch4.
- Ch6 <- Ch3, Ch5 (Proposition 5.16 in Theorem 6.9). Section 6.5 works in the setting of the contraction chains of Chapter 4, but cites no Chapter 4 result.
- Ch7 <- Ch2 (Theorems 2.20, 2.24), Ch3, Ch4 (Lemma 4.32 in Proposition 7.30), Ch5.

The chapter graph is acyclic, and so is the citation graph of individual results: every citation used in a statement or proof points to an earlier result. A citation marked "(forward)" is a pointer only. Such pointers occur in remarks, in parenthetical asides and in a few definitions and statements (for example Definition 1.31), and no proof depends on them.

# Chapter 1. Pregroup grammars and recognition

## 1.1 The free pregroup

**Definition 1.1 (free pregroup, discrete case)** [spec (1.1), Sec. 1.1; addendum (11.1)-(11.3), C15]. The adjoints of a simple type are (b,z)^l = (b,z-1) and (b,z)^r = (b,z+1). For a string they are (t_1 ... t_r)^l = t_r^l ... t_1^l, and likewise for ^r. For x_1, x_2 in Typ* there are two rewrite steps:

    (C)  x_1 (b,z)(b,z+1) x_2  ->_C  x_1 x_2            (adjacent simple types; the left one has the smaller z)
    (E)  x_1 x_2               ->_E  x_1 (b,z+1)(b,z) x_2.

X ->* Y denotes the reflexive-transitive closure of ->_C together with ->_E. Rule (C) covers t^l t -> 1 (z = -1) and t t^r -> 1 (z = 0). A partially ordered B would need generalised contractions (Lambek 1999, UNVERIFIED citation detail). All engineering documents use the discrete case, and so does this document.

**Lemma 1.2 (string adjoints).** For every X: X^l X ->_C* 1 and X X^r ->_C* 1. Consequently X (X^r X X^l) X ->_C* X. Example: the VP-coordination type is (n^r s)^r (n^r s)(n^r s)^l = s^r n^rr n^r s s^l n [addendum Sec. 11.2]. That string is obtained by the adjoint rule; it is not an instance of the first two statements (X^r X does not reduce to 1 in general, since t^r t is an expansion pair).

*Proof.* Induct on |X|. For X = tY: X^l X = Y^l t^l t Y ->_C Y^l Y ->_C* 1, and X X^r = t Y Y^r t^r ->_C* t t^r ->_C 1. For the second claim, contract the leftmost X against X^r and the rightmost X^l against X; this leaves X. QED

**Definition 1.3 (lexicon, grammaticality)** [spec Sec. 1.1 table; addendum Sec. 11.2 table, C15]. A lexicon assigns each word a finite non-empty set of type strings. A sentence with one chosen string per word has as its type string X the concatenation of the chosen strings.

- Spec lexicon: noun n; adjective or determiner n n^l; intransitive verb n^r s; transitive verb n^r s n^l; post-verbal adverb s^r s.
- Addendum additions: ditransitive n^r s n^l n^l; noun-modifying preposition n^r n n^l; complementiser and sentence-initial adverb s s^l; sentential-complement verb n^r s s^l; noun coordination n^r n n^l; VP coordination s^r n^rr n^r s s^l n; subject relative pronoun n^r n s^l n; object relative pronoun n^r n n^ll s^l; wh-question q n^ll s^l.

X is grammatical for a target t in B iff X ->* (t,0).

**Lemma 1.4 (switching lemma; Lambek 1999, Buszkowski 2001; citation details UNVERIFIED)** [spec l.150; addendum (11.2)-(11.3); errata Sec. B; review 2.10]. If X ->* Y, then there is Y' with X ->_C* Y' ->_E* Y.

*Proof.* Consider two consecutive steps U ->_E U' ->_C U''. Here (E) inserts the pair P = (b,z+1)(b,z) at a boundary x_1 | x_2, so U' = x_1 P x_2, and (C) deletes an adjacent pair Dp of U'. There are three cases.

- (i) Dp is disjoint from P. Dp is adjacent in U', and P is a block of two consecutive types that does not meet Dp, so Dp is also adjacent in U. Doing (C) first and then (E) gives the same U''.
- (ii) Dp shares exactly one type with P. Either Dp = (last type of x_1)(b,z+1), which forces x_1 = x_1'(b,z) and U'' = x_1'(b,z)x_2 = U; or Dp = (b,z)(first type of x_2), which forces x_2 = (b,z+1)x_2' and U'' = x_1(b,z+1)x_2' = U. In both cases delete both steps.
- (iii) Dp = P is impossible, because (b,z+1)(b,z) is not an instance of (C).

Each move strictly reduces the number of pairs (E-step before C-step). So the rewriting terminates, with all contractions first. QED

**Corollary 1.5** [spec (1.1): "expansions are never needed"; addendum l.88]. For tau in Typ, X ->* tau iff X ->_C* tau. Also X ->* 1 iff X ->_C* 1.

*Proof.* By Lemma 1.4, X ->_C* Y' ->_E* tau. Contractions shorten a string by 2 and expansions lengthen it by 2, so |Y'| + 2k = |tau| = 1 with k expansions. This forces k = 0 and Y' = tau. The same argument works for the empty string 1. QED

Corollary 1.5 also shows that the pregroup is not commutative: n n^r ->_C 1, while n^r n has no contraction, so n^r n does not reduce to 1.

## 1.2 Reductions as non-crossing matchings

**Definition 1.6 (reduction)** [addendum l.88-93; errata E-12]. Let X have length m. A reduction of X to position p is a set M of cups with:

- (a) match(i,j) for every (i,j) in M;
- (b) M is a perfect matching of {0..m-1} \ {p};
- (c) no two cups cross, where (i,j) and (k,l) cross if i < k < j < l;
- (d) no cup encloses p.

A reduction of X to 1 satisfies (a)-(c) and is a perfect matching of all positions.

**Theorem 1.7 (derivations are planar matchings)** [errata E-12; addendum l.88-93; review 2.10, 5(a)].

- (i) Record the pairs deleted by a contraction derivation X ->_C* t_p by their original positions, where p is the surviving original position. They form an element of Red(X,p). Conversely, every M in Red(X,p) is the cup set of some contraction derivation. The same holds for Red(X -> 1).
- (ii) j - i is odd for every cup of a reduction. A string that reduces to one simple type has odd length.
- (iii) X ->* (t,0) iff Red(X,p) is non-empty for some p with t_p = (t,0). The unions over p are disjoint (a reduction determines its unique survivor), and Der(X,tau) is finite.

*Proof.* (i), forward direction. Types never change, so deleted pairs satisfy (a). Each position other than p is deleted exactly once, which gives (b). If (i,j) is deleted at step s, the two types were adjacent then, so every k strictly between them was deleted earlier. Hence p is never enclosed, which gives (d). Suppose (i,j) and (k,l) cross, i < k < j < l. If (i,j) is deleted first, k must already be gone; but k is deleted together with l, and l > j is not yet adjacent. Symmetrically, if (k,l) is deleted first, then j (strictly between k and l) must already be gone, but j is deleted together with i later. Either way we reach a contradiction, which gives (c).

(i), converse. Induct on |M|. Pick (i,j) in M of minimal span. Any k with i < k < j is not p, by (d), so k is matched to some l. The cup (k,l) cannot cross (i,j), so it lies inside (i,j) and has smaller span, which is a contradiction. So i and j are adjacent. Apply (C) to them; then M \ {(i,j)} is a reduction of the shorter string.

(ii) By (c) and (d), the positions strictly inside a cup are matched among themselves, so j - i - 1 is even. The m - 1 matched positions are paired off, so m - 1 is even.

(iii) Combine Corollary 1.5 with (i). There are finitely many cup sets. QED

**Lemma 1.8 (survivor-cup bijection)** [addendum (11.5), equivalent form]. Let tau = (b,z) in Typ. Let X' = X (b,z+1), with the new position labelled m, and X'' = (b,z-1) X, with the new position labelled -1. Then:

- M |-> M + {(p,m)} is a bijection from the disjoint union, over p with t_p = tau, of Red(X,p) onto Red(X' -> 1);
- M |-> M + {(-1,p)} is a bijection from the same disjoint union onto Red(X'' -> 1).

Hence #Der(X,tau) = #Red(X(b,z+1) -> 1) = #Red((b,z-1)X -> 1). Neither map uses (c) or (d) in an essential way, so the same bijections hold for matchings that are not required to be planar.

*Proof.* The new cup is admissible and completes the matching. It crosses no cup of M, because no cup of M encloses p. Conversely, take M' in Red(X' -> 1). Position m is matched to some p with t_p = tau. Deleting that cup leaves a matching of all positions but p, and no remaining cup encloses p (such a cup would cross (p,m)). The two maps are mutually inverse. The prepended case is symmetric. QED

## 1.3 Chart recognition

**Definition 1.9 (chart recurrence)** [addendum (11.4), (11.5); errata E-12]. Set R[a][b] = C[a][b] = 0 whenever a <= b and b - a is even. This convention is correct by Theorem 1.7(ii): such windows have odd length and never reduce to 1. Put inner(a,b) = 1 and count(a,b) = 1 if a > b (empty window); otherwise inner(a,b) = R[a][b] and count(a,b) = C[a][b]. For j - i odd:

    R[i][j] = OR over j' in {i+1, i+3, ..., j}:  match(i,j') and inner(i+1,j'-1) and inner(j'+1,j)          (11.4)
    C[i][j] = SUM over those j' for which the conjunction holds:  count(i+1,j'-1) * count(j'+1,j)
    accept(p,t) = [t_p = (t,0)] and inner(0,p-1) and inner(p+1,m-1)                                      (11.5)
    #parses(t) = SUM over p with accept(p,t):  count(0,p-1) * count(p+1,m-1).

Addendum (11.5) at l.100 omits the filter t_p = (t,0) and sums over all p. Its l.124 has the filter. The version above is the correct one.

**Theorem 1.10 (correctness and exact counting)** [addendum (11.4)-(11.5), l.95-126, T11; errata E-12; review 2.10, 5(a)]. The chart computes:

- R[i][j] = 1 iff X[i..j] reduces to 1;
- C[i][j] = #Red(X[i..j] -> 1);
- accept(p,t) = 1 iff t_p = (t,0) and Red(X,p) is non-empty;
- #parses(t) = #Der(X,(t,0)).

*Proof.* Induct on j - i. Take M in Red(X[i..j] -> 1). Position i has a unique partner j', and j' - i is odd by Theorem 1.7(ii). Every other cup lies either inside (i,j') or wholly to its right, since otherwise it would cross (i,j'). The inner cups form a reduction of X[i+1..j'-1] to 1, and the outer cups form a reduction of X[j'+1..j] to 1. Conversely, an admissible (i,j') together with a reduction of each part is a reduction of X[i..j], because no crossing can arise between the parts.

So M |-> (j', M_in, M_out) is a bijection onto the disjoint union, over admissible j', of product sets. This gives the OR and the SUM; an empty part contributes exactly one reduction, the empty set.

For accept and #parses: the survivor is not enclosed, so the cups split into a reduction of X[0..p-1] and a reduction of X[p+1..m-1], again bijectively. Disjointness over p is Theorem 1.7(iii). Lemma 1.8 gives the alternative single-cell form: #Der(X,(b,z)) = C[0][m] computed on X(b,z+1). QED

**Theorem 1.11 (exact cost)** [errata E-12; review 2.10, correcting addendum l.107 "~m^3/12"; addendum Sec. 16]. Fill (11.4) in order of increasing window length, testing each candidate j' once. The number of partner tests is exactly

    Ptest(m) = (m-1)(m+1)(m+3)/24   (m odd),        Ptest(m) = m(m+1)(m+2)/24   (m even).

For example Ptest(5) = 8, Ptest(9) = 40, Ptest(197) = 323,400 and Ptest(511) = 5,592,320. The number of non-trivial cells is floor(m/2) ceil(m/2) <= m^2/4.

*Proof.* Windows of length 2l (span 2l - 1), for 1 <= l <= h := floor(m/2), have m - 2l + 1 placements and l candidates each. So

    Ptest(m) = SUM_{l=1}^{h} l (m + 1 - 2l) = (m+1) h(h+1)/2 - h(h+1)(2h+1)/3.

For m = 2h + 1 this equals h(h+1)(h+2)/3 = (m-1)(m+1)(m+3)/24. For m = 2h it equals h(h+1)(2h+1)/6 = m(m+1)(m+2)/24. QED

The storage estimate of addendum l.103 (R as an m x m bit table; counts and first partners only on the odd-span cells) is an engineering consequence of the cell count and is not treated here. The ratio "m ~ 2.3 x words" is UNVERIFIED.

**Proposition 1.12 (enumeration of parses)** [addendum l.117-122; (11.25), (11.26)]. Assume the counts C are exact (no saturation). List the admissible partners j' of i in increasing order, with n_{j'} = count(i+1,j'-1) count(j'+1,j) and n_in = count(i+1,j'-1). Define kth(i,j,k) as follows:

1. Take the first j' with k < n_{j'}, subtracting the earlier n's from k along the way.
2. Emit the cup (i,j').
3. Recurse on the inner window with k mod n_in and on the outer window with k div n_in.

This is a bijection {0, ..., C[i][j] - 1} -> Red(X[i..j] -> 1). The call kth(., ., 0) returns the first-partner parse.

*Proof.* The blocks of sizes n_{j'} partition {0..C[i][j]-1}, because C[i][j] is their sum (Theorem 1.10). Within a block, k |-> (k mod n_in, k div n_in) is a bijection onto the product index set. By induction on span, the recursive calls are bijections onto the inner and outer reduction sets. Composing with the product bijection of Theorem 1.10 gives the claim. If a count is capped (saturated), the block sizes are wrong and the map is no longer a bijection. QED

**Theorem 1.13 (Buszkowski; cited)** [addendum Sec. 11.6 l.454; review 8]. Pregroup grammars generate exactly the epsilon-free context-free languages. Here a pregroup grammar means a finite lexicon over the free pregroup on a finite B, with a simple-type target.

Sources: W. Buszkowski, "Lambek grammars based on pregroups", LACL 2001, LNAI 2099, 95-109 (citation details UNVERIFIED); D. Bechet, Studia Logica 87 (2007) 199-224, for polynomial parsing.

We do not reproduce the proof, and nothing below depends on it. The specialisation this programme uses is Theorem 1.10, a CKY-type (Cocke-Kasami-Younger) recurrence for an implicit binary grammar with the single nonterminal "reduces to 1". Theorem 1.13 has one consequence: a non-context-free string language, such as the copy language of cross-serial dependencies, has no pregroup grammar. Theorem 1.7 gives a separate one: no type assignment yields a crossing cup set.

**Proposition 1.14 (crossing matchings)** [addendum Sec. 11.6, l.457-459]. Let G(X) be the graph whose vertices are the positions of X, with an edge {i,j} iff match(i,j) or match(j,i).

- G(X) is bipartite, with the parts "even z" and "odd z".
- X admits a (possibly crossing) admissible perfect matching of all positions except one survivor of type (b,z) iff G(X(b,z+1)) has a perfect matching.
- This is decidable by Hopcroft-Karp (SIAM J. Comput. 2 (1973) 225-231) in O(|E| sqrt|V|) = O(m^{2.5}) time.

*Proof.* z_j = z_i + 1 joins opposite parities, so G(X) is bipartite. The equivalence is Lemma 1.8 for non-planar matchings. The size bounds are |E| <= m^2/2 and |V| = m + 1. QED

*Remark.* Counting perfect matchings of general bipartite graphs is #P-complete (Valiant, Theor. Comput. Sci. 8 (1979) 189-201). The engineering claim that counting crossing cup sets is #P-hard needs hardness for the restricted class of graphs G(X). That is UNVERIFIED.

**Proposition 1.15 (lexical ambiguity lattice)** [errata E-12; review 2.10]. Suppose word i has k_i <= k alternative type strings.

Typed positions are triples x = (i, a, o): word, alternative, offset. There are M_lat <= k n_w m_max of them, where m_max is the maximal length of a single word's type string. The committed type strings are exactly the paths of the directed acyclic lattice with two kinds of arc:

- (i,a,o) -> (i,a,o+1);
- (i, a, last offset of (i,a)) -> (i+1, a', 0), for every a'.

Here succ(x) and pred(x) are the arc successors and predecessors of x; each set has at most k elements. Sources are the nodes (0,a,0) and sinks are the nodes (n_w - 1, a, last offset).

Let R^typ[x][y] = 1 iff some path from x to y carries a string that reduces to 1. Then

    R^typ[x][y] = OR over y' with match(x,y'):
                  [ y' in succ(x)  or  OR_{s in succ(x), r in pred(y')} R^typ[s][r] ]
              and [ y' = y         or  OR_{s' in succ(y')} R^typ[s'][y] ].

Evaluating x in reverse topological order and y in topological order computes every entry from entries already computed. There are O(M_lat^2) cells, O(M_lat) choices of y' per cell and at most k^2 pairs (s,r), for a total of O(k^2 M_lat^3) operations. By contrast, running Theorem 1.10 separately on each of up to k^{n_w} committed strings is exponential in n_w.

The accept rule is: accept at a typed position p of type (t,0) iff both of the following hold:

- p is a source, or OR over sources src and r in pred(p) of R^typ[src][r];
- p is a sink, or OR over s in succ(p) and sinks snk of R^typ[s][snk].

*Proof.* On a reducing path, the partner y' of x is unique. The sub-path strictly between x and y', and the sub-path strictly after y', carry strings that reduce to 1 (the argument of Theorem 1.10). Conversely, a concatenation of paths through y' is again a path. The accept rule is Theorem 1.10's accept applied to paths from a source to a sink through p. A brute-force comparison over all committed strings on 200,000 random lattices (n_w <= 4, k <= 2) found no mismatch; that check is consistent with the proof but is not part of it. QED

Errata E-12's cost O(M^3) for the lattice chart is this bound for fixed k; the factor k^2 comes from the pairs (s, r).

*Remark (longest parsable prefix)* [addendum l.493]. accept_prefix(j) = OR_p [t_p = (t,0) and inner(0,p-1) and inner(p+1,j)] is exactly the accept rule of Theorem 1.10 applied to X[0..j]. It is therefore exact and needs no extra cells.

## 1.4 Ambiguity: a finite, in general exponentially large, set of derivations

**Proposition 1.16 (Catalan ambiguity)** [addendum (11.25), (11.26), l.103-104, 124; errata E-12]. Let X_k = n (n^r n n^l n)^k. This is a noun with k prepositional phrases, of length m = 4k + 1. Then

    #Der(X_k, n) = Cat_k >= 2^{k-1} = 2^{(m-5)/4}     (k >= 1).

Prefixing n n^r s n^l (a subject and a transitive verb) gives a sentence of m + 4 types with Cat_k reductions to s.

*Proof.* Write o, r, l for (N,0), (N,1), (N,-1). Then X_k = o(rolo)^k. By Lemma 1.8, g(k) := #Der(X_k, o) = #Red(Str_k -> 1), where Str_k = o(rolo)^k r. The o's of Str_k sit at positions 0, 4i - 2 and 4i (1 <= i <= k).

The final r, at position 4k + 1, is matched to an o at some position c. By the splitting argument of Theorem 1.10, the number of reductions with that choice of c is #Red(Str_k[0..c-1] -> 1) times #Red(Str_k[c+1..4k] -> 1). Consider the possible c:

- c = 0 or c = 4i with i < k: the right window begins with an r that has no o to its left, so the count is 0.
- c = 4k: the left window ends with an l that has no o to its right, so the count is 0.
- c = 4i - 2: the right window is l X_{k-i}, which has g(k-i) reductions (Lemma 1.8, prepended form). The left window is Str_{i-1}, which has g(i-1) reductions.

So g(k) = SUM_{i=1}^{k} g(i-1) g(k-i) with g(0) = #Red(or -> 1) = 1. This is the Catalan recurrence. Since Cat_k / Cat_{k-1} = 2(2k-1)/(k+1) >= 2 for k >= 2 and Cat_1 = 1, we get Cat_k >= 2^{k-1}.

For the sentence: the cup (0,1) is forced, and Lemma 1.8 applied to l X_k starting at position 3 gives g(k). QED

**Definition 1.17 (ambiguity objects)** [limits B1-B3; addendum (11.25), (11.26)]. Der(X,(t,0)) = {Dv_1, ..., Dv_K} is finite (Theorem 1.7), and Proposition 1.12 enumerates it. Every semantic object indexed by it is therefore a finite object: weights, mixtures, superpositions and Gram matrices. Chapters 3 and 5 treat them.

**Proposition 1.18 (row-stochastic slot mixing is not a mixture of assignments)** [limits l.1463; addendum (11.25)].

Let s: V_1 x ... x V_m -> Out be multilinear in m noun slots, all V_a = C^d. Let A be an m x m row-stochastic matrix (non-negative, rows summing to 1). Let x_1, ..., x_m be noun vectors and xt_a = SUM_b A[a][b] x_b. Then

    s(xt_1, ..., xt_m) = SUM over all maps f: {1..m} -> {1..m} of ( PROD_a A[a][f(a)] ) s(x_{f(1)}, ..., x_{f(m)}),

a convex combination over m^m assignments.

- (i) The total weight on non-bijective assignments is 1 - perm(A). This is > 0 unless A is a permutation matrix.
- (ii) Suppose the m^m vectors s(x_{f(1)}, ..., x_{f(m)}) are linearly independent; this holds, for example, for the universal multilinear map s = tensor product with linearly independent x_b. Then s(xt) lies in the convex hull of the bijective assignments only if A is a permutation matrix.

Example: m = 2, A = [[h, 1-h], [1-h, h]]. Then s(xt_1, xt_2) = h^2 s(x_1,x_2) + (1-h)^2 s(x_2,x_1) + h(1-h)[s(x_1,x_1) + s(x_2,x_2)].

*Proof.* The expansion follows from multilinearity. The coefficients are non-negative and sum to PROD_a SUM_b A[a][b] = 1 (only the row sums are used).

(i) The bijective terms sum to the permanent perm(A). Suppose perm(A) = 1. Pick an f with positive weight; it is a bijection. If row a had a second non-zero entry at column k != f(a), then changing f(a) to k would give a positive-weight map. That map is not injective, because k = f(b) for some b != a. This is a contradiction. So every row has exactly one non-zero entry, and since f is a bijection, A is a permutation matrix.

(ii) By linear independence, the coefficients are uniquely determined, and (i) applies. Without independence the claim can fail. For example, s(x,y) = (x_1 + x_2)(y_1 + y_2) with x_1 = e_1 and x_2 = e_2 makes every term equal to 1. QED

## 1.5 The stack reducer: it can emit any reduction but cannot find one

**Definition 1.19 (runs, greedy run)** [spec l.162-174; errata E-12]. A run processes positions p = 0, ..., m-1 with a stack of positions. At p it either:

- contracts, which is allowed only if the top i of the stack satisfies match(i,p); it pops i and emits the cup (i,p); or
- shifts, pushing p.

A run accepts for target t if the final stack is {p} with t_p = (t,0). The greedy run contracts whenever contraction is allowed. For M in Red(X,p*), the M-run contracts at p iff p is a right end of a cup of M.

**Theorem 1.20 (accepting runs = reductions)** [errata E-12].

- (i) The cups emitted by an accepting run with final stack {p} form an element of Red(X,p).
- (ii) For M in Red(X,p*), the M-run is legal, emits exactly M, and accepts with stack {p*}.

*Proof.* (i) Emitted cups are admissible. Each position is either the right end of an emitted cup (and is never pushed), or is pushed once and popped at most once. So the cups form a matching of all positions except p.

Let (i,j) be emitted and i < k < j. There are two cases:

- k was pushed. It was pushed after i and popped before step j, because i is the top at step j. So k != p, and k's partner l satisfies l < j.
- k is a right end. Its partner l was the top at step k while i was still on the stack below it, so i < l < k.

In both cases (i,j) does not enclose p and does not cross the cup containing k.

(ii) Invariant after processing the positions < p:

- the emitted cups are the cups of M with both ends < p;
- the stack holds, in order, p* (if p* < p) followed by the left ends i < p whose partner is >= p.

At step p: if (i,p) is in M, then every k between i and p is matched inside (i,p), because M has no crossings and p* is not enclosed. So k has its partner < p and is off the stack. Hence i is the top, match(i,p) holds, and the contraction is legal. Otherwise p is a left end or p = p*, and shifting restores the invariant. QED

**Proposition 1.21 (exactness criterion for greedy)** [spec l.175-176; errata E-12; addendum l.129-139]. Let M be in Red(X,p*). Call an admissible pair (i,p) not in M *exposed* if i is the top of the M-run's stack just before step p. Then:

- greedy emits M iff M has no exposed pair;
- greedy accepts X for t iff some M in Der(X,(t,0)) has no exposed pair;
- greedy never accepts an ungrammatical X.

*Proof.* Greedy and the M-run agree until the first step p at which their decisions differ. If p is a right end of M, the top is its partner and both runs contract. So the two runs differ iff p is not a right end and match(top,p) holds, that is, iff (top,p) is exposed. With no exposed pair, the runs agree throughout and Theorem 1.20(ii) applies. With an exposed pair, greedy emits a cup outside M.

Conversely, suppose greedy accepts with cup set M. By Theorem 1.20(i), M is a reduction. Greedy contracts exactly at the right ends of M, so the greedy run is the M-run. At every p that is not a right end, greedy shifted, so match(top,p) failed there. Hence M has no exposed pair. Soundness is Theorem 1.20(i). QED

**Corollary 1.22 (degree-one condition).** If every position of X has at most one admissible partner (the graph G(X) of Proposition 1.14 is a matching), then greedy is exact on X, and the reduction, if one exists, is unique over all survivors.

*Proof.* Let M cover every position except p. Any edge at p has its other endpoint of degree 1, and that endpoint is matched in M, so it is matched to p. That is impossible, so p is isolated. Hence M = E(G(X)), which is unique. There are no admissible pairs outside M, so there are no exposed pairs, and Proposition 1.21 applies. QED

**Proposition 1.23 (the shape table)** [spec Sec. 1.1 table, l.176, Sec. 9 item 1 (l.1242); addendum l.129; review 7 P0]. Greedy is exact on each of the following strings, with the unique reduction shown:

    N IV            n . n^r s                              (0,1)                       s at 2
    N TV N          n . n^r s n^l . n                      (0,1)(3,4)                  s at 2
    ADJ N TV N      n n^l . n . n^r s n^l . n              (1,2)(0,3)(5,6)             s at 4
    N TV ADJ N      n . n^r s n^l . n n^l . n              (0,1)(3,4)(5,6)             s at 2
    ADJ N TV ADJ N  n n^l . n . n^r s n^l . n n^l . n      (1,2)(0,3)(5,6)(7,8)        s at 4
    N IV ADV        n . n^r s . s^r s                      (0,1)(2,3)                  s at 4
    ADJ N IV        n n^l . n . n^r s                      (1,2)(0,3)                  s at 4
    ADJ N IV ADV    n n^l . n . n^r s . s^r s              (1,2)(0,3)(4,5)             s at 6

The spec justifies this by saying that "each simple type has at most one admissible partner" (Corollary 1.22). That is literally false for the ADJ shapes. In ADJ N TV N, position 1 has the admissible partners 2 and 6, and position 3 has 0 and 2. Errata E-12 repeats the same justification. Proposition 1.21 replaces it.

*Proof.* Each listed M is admissible, perfect, non-crossing, and leaves the survivor unenclosed; all of this is checked by inspection.

The admissible pairs outside M, and why none is exposed:

- ADJ N TV N: (1,6) and (2,3). At step 6, position 1 is off the M-stack (its partner is 2). At step 3, position 2 is a right end and was never pushed. Neither pair is exposed.
- ADJ N TV ADJ N: (1,6), (1,8), (2,3), (5,8). The partners of 1, 2 and 5 are 2, 1 and 6, all earlier than the step in question. None is exposed.
- N TV ADJ N: (3,6). Position 3 is matched to 4, so it is off the stack at step 6.
- ADJ N IV and ADJ N IV ADV: (2,3). Position 2 is a right end, and the top at step 3 is 0.
- The remaining shapes have no admissible pair outside M.

Uniqueness: Theorem 1.10, evaluated on each of the eight strings (with m <= 9, so at most Ptest(9) = 40 partner tests each), gives #parses(s) = 1. This finite evaluation is the proof of uniqueness. QED

*Remark (P0 grammar)* [review 7 P0; addendum 15.3(a)]. Take the grammar S -> NP VP, NP -> N | ADJ N, VP -> TV NP | IV | IV ADV, with 24 N, 12 ADJ, 12 TV, 8 IV and 8 ADV. Then

- |NP| = 24 + 12 x 24 = 312;
- |VP| = 12 x 312 + 8 + 8 x 8 = 3,816;
- |S| = 312 x 3,816 = 1,190,592.

The grammar realises eight shapes (two NP forms times four VP forms), not six. Proposition 1.23 covers all eight.

**Proposition 1.24 (greedy is incomplete one word outside the table)** [errata E-12; spec l.162-163; review 2.10; addendum l.129-132, T11].

(i) "Alice loves man in park" = n | n^r s n^l | n | n^r n n^l | n (positions 0..8) has the unique reduction (0,1)(4,5)(3,6)(7,8), with s at 2. Greedy emits (0,1)(3,4)(7,8) and halts with stack {2,5,6}. Noun coordination n^r n n^l gives the same type string, so "Alice loves Bob and Carol" behaves in exactly the same way.

(ii) "Alice loves the man in park" = n | n^r s n^l | n n^l | n | n^r n n^l | n (positions 0..10) has exactly two reductions to s, both with s at 2. They are the two determiner-scope readings:

    M1 = (0,1)(5,6)(4,7)(3,8)(9,10),      M2 = (0,1)(3,4)(6,7)(5,8)(9,10).

Greedy halts with stack {2,7,8}.

(iii) "Alice sleeps and dreams" = n | n^r s | s^r n^rr n^r s s^l n | n^r s (positions 0..10) has exactly one reduction, (2,3)(1,4)(0,5)(8,9)(7,10), with s at 6. Greedy halts with stack {4,5,6}.

*Proof.* (i) The types are 0 (N,0), 1 (N,1), 2 (S,0), 3 (N,-1), 4 (N,0), 5 (N,1), 6 (N,0), 7 (N,-1), 8 (N,0). The admissible pairs are (0,1), (0,5), (3,4), (3,6), (3,8), (4,5) and (7,8). The pair (0,5) encloses the only (S,0) type, at position 2, so it is excluded by (d). The survivor must be 2, so (0,1) is forced, and positions 3..8 must be perfectly matched:

- (3,4) leaves 5 = (N,1) with no partner;
- (3,8) needs 4..7 to be matched, but (6,7) is inadmissible;
- (3,6), with (4,5) inside and (7,8) outside, works.

The pair (3,4) is exposed, since 3's partner is 6 and nothing lies between 3 and 4. The greedy trace is: push 0; (0,1); push 2; push 3; (3,4); push 5; push 6 (match(5,6) would need z_6 = 2); push 7; (7,8).

(ii) Both M1 and M2 are admissible and nested, and neither encloses 2. Theorem 1.10 gives #parses = 2. For M1, the pair (3,4) is exposed (3's partner is 8). For M2, the M2-run reaches step 6 with top 5 while 5's partner is 8, so (5,6) is exposed. By Proposition 1.21 greedy emits neither reduction, and since there are only two, it rejects. The greedy trace is: push 0; (0,1); push 2; push 3; (3,4); push 5; (5,6); push 7; push 8 (match(7,8) would need z_8 = 2); push 9; (9,10). Final stack {2,7,8}.

(iii) The types are 0 (N,0), 1 (N,1), 2 (S,0), 3 (S,1), 4 (N,2), 5 (N,1), 6 (S,0), 7 (S,-1), 8 (N,0), 9 (N,1), 10 (S,0). The listed cups are admissible and nested, and 6 is unenclosed. Theorem 1.10 gives #parses = 1. The pair (0,1) is exposed (0's partner is 5). The greedy trace is: push 0; (0,1); push 2; (2,3); push 4; push 5; push 6; push 7; push 8; (8,9); (7,10). Final stack {4,5,6}. QED

**Proposition 1.25 (plain backtracking is exponential)** [errata E-12; spec l.176-179]. Let Y_k = ((N,0)(N,1))^k (N,1), of length m = 2k + 1, with target S. Depth-first search over all runs, without lookahead, explores exactly 2^k complete runs, and all of them reject.

*Proof.* No (S,0) occurs, so no run accepts.

Invariant: before each even position, the stack is empty or its top is an (N,1). This holds initially. At an even position 2i < 2k, the type is (N,0). An (N,1) top cannot match it (that would need an (N,2)), so the run shifts. At the odd position 2i+1, the top is the (N,0) just pushed, and match holds. The run then offers a binary choice: contract, which restores the previous top (an (N,1) or empty); or shift the (N,1). Either way the invariant is restored. At position 2k the type is (N,1) and the top is (N,1) or empty, so the run shifts.

So each of the k odd positions offers exactly one binary choice, independently of the earlier choices, giving 2^k runs. QED

*Remark.* Pruning defeats this family. No lower bound is claimed for every backtracking scheme (UNVERIFIED). The rejection rates quoted at addendum l.133-134 are statistics of the sentence generator, not mathematics.

**Conjecture 1.26 (characterisation of greedy failure)** [errata E-12; addendum l.130-132; review 2.10]. Take the addendum lexicon WITHOUT the VP-coordination type, and let the word types n^r n n^l (noun-modifying preposition and noun coordination) be the "post-nominal" words. Conjecture: on a grammatical string, greedy fails iff some noun phrase that contains a post-nominal word is the argument of an n^l belonging to a word outside that noun phrase (the object of a verb or the object of a preposition).

The VP-coordination type is a separate failure class: Proposition 1.24(iii) shows that it fails with no post-nominal word present.

Known instances:

- Proposition 1.24(i) and (ii) fail, and so does "Alice loves Bob and Carol".
- "the man in the park sleeps" = n n^l | n | n^r n n^l | n n^l | n | n^r s passes, with greedy cups (1,2)(0,3)(5,6)(7,8)(4,9) and s at 10.
- "Alice and Bob sleep" passes.
- "Alice whom Bob loves sleeps" passes with the object-relative type n^r n n^ll s^l.

Under the subject-relative type n^r n s^l n, the same string is ungrammatical: Red is empty for every p, because the n^l of loves has no (N,0) to its right. So every recogniser rejects it, and it is not evidence about greedy. The earlier formulation "a noun followed by post-nominal n^r n" [addendum l.130-131] is refuted by "the man in the park sleeps". Sufficiency and necessity of the restricted statement are UNVERIFIED.

## 1.6 Nesting depth and embedding

**Definition 1.27 (nesting depth)** [addendum (11.21), (11.23)]. The depth of a cup c in M is 1 plus the number of cups of M enclosing c. nd(M) is the maximum depth over the cups of M. This is the level count used by the addendum.

*Remark (long-range dependency)* [addendum l.180]. A cup (i,j) may have arbitrarily large j - i, since any string reducing to 1 may sit between i and j (Theorem 1.10). Only nd(M) is a structural cost (Theorem 4.18, forward).

**Proposition 1.28 (right embedding: constant depth)** [addendum l.183; (11.21), (11.23)]. Let

    Rt_k = n (n^r s s^l) [(s s^l) n (n^r s s^l)]^{k-1} (s s^l) n (n^r s),

with k >= 1 sentential-complement verbs and length m = 6k + 3 ("Alice believes that Bob thinks that ... sleeps"). Take the cup set consisting of:

- each adjacent (subject n, verb n^r);
- each adjacent (verb s^l, complementiser s);
- each (complementiser s^l, next verb's s).

This is a reduction to the first verb's s, and nd = 2 for every k. For k = 2 it is (0,1)(3,4)(6,7)(5,8)(9,10)(12,13)(11,14), with s at 2.

*Proof.* The three kinds of cup have types (N,0)(N,1), (S,-1)(S,0) and (S,-1)(S,0), so all are admissible. There are k + 1 subject cups, k verb-complementiser cups and k complementiser-verb cups: 3k + 1 in all. They cover the 6k + 2 non-survivor positions exactly once each.

A complementiser-verb cup encloses exactly the noun and the n^r between its ends, which form one adjacent cup. The adjacent cups are enclosed by at most that one cup, and the complementiser-verb cups are pairwise disjoint. So there is no crossing, position 2 is unenclosed, and nd = 2. QED

**Proposition 1.29 (centre embedding: +3 per object relative)** [addendum l.183]. Let

    Ce_k = n (n^r n n^ll s^l n)^k (n^r s n^l)^k (n^r s),

of length m = 8k + 3 (k = 2: "Alice whom Bob whom Carol loves loves sleeps"). The positions are:

- relative pronoun j (j = 1..k) at 5j-4 (n^r), 5j-3 (n), 5j-2 (n^ll), 5j-1 (s^l), with its noun at 5j;
- verb i (i = 1..k) at 5k+3i-2 (n^r), 5k+3i-1 (s), 5k+3i (n^l);
- the final verb at 8k+1 and 8k+2.

The cups

    (5(j-1), 5j-4) for j = 1..k;   (5k, 5k+1);
    for j = k, ..., 1 with i = k+1-j:   (5j-1, 5k+3i-1),  (5j-2, 5k+3i),  (5j-3, 5k+3i+1)

form a reduction to 8k+2 with nd = 3k + 1. So nd = 4 for k = 1 and nd = 7 for k = 2.

*Proof.* Admissibility: the cup types are (N,0)(N,1); (N,0)(N,1); (S,-1)(S,0); (N,-2)(N,-1); and (N,0)(N,1), where position 5k+3i+1 is the n^r of verb i+1 or of the final verb. There are 4k + 1 cups covering 8k + 2 distinct positions.

The 3k + 1 "chain" cups (5k,5k+1), (5j-1,.), (5j-2,.), (5j-3,.) have strictly decreasing left ends and strictly increasing right ends in the listed order. So they form a chain of enclosures of length 3k + 1.

The subject cup (5(j-1), 5j-4) lies strictly between the left ends of the chain cups of blocks j-1 and j. It is enclosed exactly by the 3(j-1) chain cups of blocks 1..j-1, so its depth is 3j - 2 <= 3k - 2 < 3k + 1. It is disjoint from, and to the left of, the chain cups of blocks >= j, so it crosses nothing. Position 8k + 2 is unenclosed. QED

**Conjecture 1.30 (uniqueness of the embedding reductions for general k)** [addendum Sec. 19 item 9]. For every k >= 1, Rt_k and Ce_k have exactly one reduction to s, and each subject-relative level adds 2 to the nesting depth.

What is proved: for k = 1, 2, 3, Theorem 1.10 evaluated on Rt_k (m = 9, 15, 21) and Ce_k (m = 11, 19, 27) gives #parses(s) = 1 in each case, with nd = 2 for Rt_k and nd = 4, 7, 10 for Ce_k. The general-k uniqueness and the subject-relative depth law remain Conjecture.

**Example 1.30' (aux-inverted question)** [addendum Sec. 19 item 9]. With whom = q n^ll q^l, does = q i^l n^l, Alice = n and love = i n^l, the string of "whom does Alice love",

    (Q,0)(N,-2)(Q,-1) | (Q,0)(I,-1)(N,-1) | (N,0) | (I,0)(N,-1)      (positions 0..8),

has exactly one reduction to (Q,0), namely {(1,8), (2,3), (4,7), (5,6)} with survivor 0. It has no reduction to any other survivor.

*Proof.* The four cups have types (N,-2)(N,-1), (Q,-1)(Q,0), (I,-1)(I,0) and (N,-1)(N,0), so they are admissible. They are nested ((5,6) inside (4,7); (2,3) and (4,7) inside (1,8)), and position 0 is unenclosed. Uniqueness is Theorem 1.10 evaluated on these 9 positions (at most Ptest(9) = 40 partner tests), which gives #parses(Q) = 1 and accept(p, t) = 0 for every p != 0. The attribution of this type assignment to Lambek (2008) is UNVERIFIED. QED

## 1.7 Contraction along a reduction

**Definition 1.31 (meaning along a reduction)** [spec (1.5); addendum C15, (11.10)-(11.13')]. Give each b in B the space C^{d_b}, d_b = 2^{q(b)}, with its standard basis. A word of type string t_0 ... t_{r-1} is a tensor with legs indexed by its factors in C7 order (Definition 2.14, forward). Duals are identified with the space itself through the basis, so adjoints keep the space.

For a reduction M with survivor p, the meaning sigma in C^{d_{b_p}} is obtained as follows:

- take the product of all the word entries;
- tie the two legs of every cup to the same index (the component-wise pairing of C8);
- sum over one index per cup.

Theorem 2.20 (forward) proves that this is the functorial semantics of Chapter 2. Spider words (relative pronouns and coordinators) are Definition 2.31 (forward).

## 1.8 A type code and a worked example

**Lemma 1.32 (an injective type code)** [spec Sec. 6.3 l.1121-1122 (byte form; its end-marker value 255 lies outside the range of enc), C7, T10; addendum C15]. Let base(N) = 0, base(S) = 1, base(I) = 2, base(Q) = 3. For z in {-2, ..., 2}, let a(z) = 0 if z = 0, 2|z| - 1 if z < 0, and 2z if z > 0; so a takes the values 0..4. Put enc(b,z) = base(b) + 4 a(z).

Then enc: B x {-2..2} -> {0, ..., 19} is injective. Its inverse is base(b) = e mod 4 and a = floor(e/4), followed by z = 0 if a = 0, z = -(a+1)/2 if a is odd, and z = a/2 if a is even and > 0.

Examples: n^r s n^l -> 8, 1, 4; n^r s -> 8, 1; n n^l -> 0, 4; s^r s -> 9, 1.

*Proof.* a is a bijection from {-2..2} onto {0..4}, with the stated inverse (negatives map to odd values, positives to even values). Since 0 <= base(b) <= 3, division with remainder by 4 recovers base(b) and a(z) from enc(b,z). The largest value is 3 + 4 x 4 = 19. QED

**Example 1.33 ("Alice loves Bob")** [spec Sec. 8 l.1184, (1.1), C7]. X = n | n^r s n^l | n has the types 0 (N,0), 1 (N,1), 2 (S,0), 3 (N,-1), 4 (N,0). Its only admissible pairs are (0,1) and (3,4), so Corollary 1.22 applies.

The greedy run is: push 0; (0,1); push 2; push 3; (3,4). The final stack is {2}, of type (S,0). So the string is grammatical, with cups {(0,1),(3,4)}, s at 2, and the reduction is unique. The chart agrees: R[0][1] = R[3][4] = 1, accept(2,S) = 1 and #parses = 1.

Grammaticality is not invariant under permutation. The permutation n^r s n^l n n of X does not reduce to s, because the (N,1) at position 0 would need a partner to its left. The semantic sensitivity to word order is Proposition 7.28 (forward) [review 5(a)].

# Chapter 2. Rigid and compact closed categories, the DisCoCat functor and Frobenius algebras

## 2.1 Monoidal, rigid and compact closed categories

**Definition 2.1 (strict monoidal category; symmetry).** A strict monoidal category is a category Cat with a bifunctor (x): Cat x Cat -> Cat and an object I such that (A (x) B) (x) C = A (x) (B (x) C) and I (x) A = A = A (x) I, and likewise on morphisms. Bifunctoriality is the *interchange law*

    (g_1 (x) g_2) o (f_1 (x) f_2) = (g_1 o f_1) (x) (g_2 o f_2).                                       (2.1.1)

Cat is *symmetric* if it has natural isomorphisms gamma_{A,B}: A (x) B -> B (x) A with gamma_{B,A} o gamma_{A,B} = id and the hexagon law gamma_{A,B(x)C} = (id_B (x) gamma_{A,C}) o (gamma_{A,B} (x) id_C).

**Lemma 2.2 (disjoint operations commute).** For f: A -> A' and g: B -> B',

    (f (x) id_{B'}) o (id_A (x) g) = f (x) g = (id_{A'} (x) g) o (f (x) id_B).

*Proof.* Apply (2.1.1) with (g_1, g_2, f_1, f_2) = (f, id, id, g), and again with (id, g, f, id). QED

**Definition 2.3 (rigid monoidal category).** A monoidal category is *rigid* (autonomous) if every object A has two duals.

- A *right dual* A^r, with cup^r_A: A (x) A^r -> I and cap^r_A: I -> A^r (x) A. These satisfy
      (cup^r_A (x) id_A) o (id_A (x) cap^r_A) = id_A   and   (id_{A^r} (x) cup^r_A) o (cap^r_A (x) id_{A^r}) = id_{A^r}.
- A *left dual* A^l, with cup^l_A: A^l (x) A -> I and cap^l_A: I -> A (x) A^l. These satisfy the mirror-image snake equations.

No symmetry is assumed.

**Definition 2.4 (compact closed category).** A compact closed category is a symmetric rigid monoidal category. In such a category A^l and A^r are isomorphic. We write A^*, cup_A: A (x) A^* -> I and cap_A: I -> A^* (x) A.

**Lemma 2.5 (name/coname bijection; the bent wire).** In a rigid category the maps

    name(f)    := (id_{A^r} (x) f) o cap^r_A  : I -> A^r (x) B,
    unname(phi) := (cup^r_A (x) id_B) o (id_A (x) phi) : A -> B

are mutually inverse bijections between Hom(A, B) and Hom(I, A^r (x) B).

*Proof.* First, unname(name(f)) = (cup^r_A (x) id_B)(id_A (x) id_{A^r} (x) f)(id_A (x) cap^r_A). By Lemma 2.2 this equals f o (cup^r_A (x) id_A)(id_A (x) cap^r_A), which is f by the first snake equation.

Second, the interchange law (the unit is strict) gives (id_{A^r (x) A} (x) phi) o cap^r_A = (cap^r_A (x) id_{A^r (x) B}) o phi. Hence

    name(unname(phi)) = (id_{A^r} (x) cup^r_A (x) id_B)(cap^r_A (x) id_{A^r} (x) id_B) phi = phi

by the second snake equation tensored with id_B. QED

In a compact closed category, the same argument composed with gamma turns effects A^* (x) B -> I into morphisms B -> A. This lemma is the categorical content of the "bend-the-wire" rewrite [spec Sec. 1.4]: a state on a cupped wire and an effect on the partner wire are the same morphism.

## 2.2 The pregroup as a posetal rigid category, and the free rigid category

**Definition 2.6 (pregroup).** A pregroup (Pg, <=, ., 1, (-)^l, (-)^r) is a partially ordered monoid in which every element t has a left adjoint t^l and a right adjoint t^r satisfying

    t^l t <= 1 <= t t^l,       t t^r <= 1 <= t^r t.                                                    (2.2.1)

A reduction of a type string X to s is a chain X <= ... <= s that uses only the contractions t^l t <= 1 and t t^r <= 1 [spec (1.1)].

**Proposition 2.7 (Lambek).** Regard Pg as a category with exactly one morphism a -> b iff a <= b, with tensor = concatenation and unit = 1. Then Pg is a posetal rigid monoidal category, in which:

- the right dual of t is t^r, with cup^r = (t t^r <= 1) and cap^r = (1 <= t^r t);
- the left dual of t is t^l, with cup^l = (t^l t <= 1) and cap^l = (1 <= t t^l).

In general Pg is not symmetric (n s <= s n fails), so it is not compact closed in the sense of Definition 2.4.

*Proof.* Multiplication is monotone in both arguments, and this is functoriality of the tensor. Associativity and the unit law hold strictly. The cups and caps are the inequalities (2.2.1). The snake equations hold because any two parallel morphisms of a poset are equal. QED

**Definition 2.8 (free rigid monoidal category on B).** RigFree(B) is the free rigid monoidal category generated by the basic types (Preller and Lambek, "Free compact 2-categories", Math. Struct. Comp. Sci. 17 (2007); Joyal and Street; citation details UNVERIFIED). Its objects are the strings Typ*. Its morphisms are planar string diagrams built from cups and caps, taken modulo the monoidal axioms and the snake equations. Its universal property is that a strong monoidal functor into a rigid category is determined by its values on the generators and on their cups and caps.

A reduction M in Red(X,p) (Definition 1.6) defines a morphism rho_M: X -> t_p. It is the tensor of the cups of M with the identity on the survivor, arranged planarly as in the proof of Theorem 1.7. The posetal pregroup of Proposition 2.7 is the preorder collapse of RigFree(B), and it only answers the question "is there a parse". Distinct reductions are distinct morphisms of RigFree(B).

Example: s s^l s s^r s has two reductions to s. One is {(1,2),(0,3)} with survivor 4. The other is {(2,3),(1,4)} with survivor 0. Proposition 2.18 (forward) shows they give different linear maps. Take q_s = 1 and the basis vector whose five field values are (0,0,0,0,1). The first map sends it to |1>. The second sends it to 0, because the cup (1,4) compares 0 with 1.

## 2.3 FHilb: the self-dual structure, pairings and circuits

**Definition 2.9 (self-dual structure on C^{2^q}).** Put (C^{2^q})^* := C^{2^q} and

    <cup_q| := sum_{k=0}^{2^q-1} <k| (x) <k| ,       |cap_q> := sum_{k=0}^{2^q-1} |k> (x) |k>.            (2.3.1)

These serve as both the left and the right cup and cap. FHilb, the category of finite-dimensional complex Hilbert spaces and linear maps, is compact closed with this structure.

**Lemma 2.10 (snake equations in FHilb).** (2.3.1) satisfies the snake equations. More generally, for any permutation Pm of {0..2^q-1}, the pair eps_Pm := sum_j <j| (x) <Pm j| and eta_Pm := sum_k |Pm k> (x) |k> satisfies the snake equations.

*Proof.* (id (x) eps_Pm)(eta_Pm (x) id)|a> = sum_k |Pm k> sum_j <j|k> <Pm j|a> = sum_k |Pm k><Pm k|a> = |a>. Also (eps_Pm (x) id)(id (x) eta_Pm)|a> = sum_{k,j} <j|a> <Pm j|Pm k> |k> = |a>. Taking Pm = id gives (2.3.1). QED

**Proposition 2.11 (normalisation and pairing of the cup)** [spec C8].

- (i) <cup_q| = 2^{q/2} <Phi+_q|, where |Phi+_q> = 2^{-q/2} sum_k |k>|k> is the normalised Bell state.
- (ii) In little-endian order, <cup_q| is the product over r = 0..q-1 of <cup_1| on the bit pair (a_r, b_r). This is the component-wise pairing.
- (iii) The nested pairing a_{q-1-r} <-> b_r is eps_Pm with Pm = bitreverse_q. It is a valid compact structure (Lemma 2.10), and it equals (ii) iff q = 1.

*Proof.* (i) is the definition of |Phi+_q>. (ii) For k = sum_r k_r 2^r, <k| (x) <k| factorises as prod_r <k_r|_{a_r} <k_r|_{b_r}. Summing over k is the same as summing over all bit tuples. (iii) The nested pairing evaluates to sum_k <k|_A <bitreverse(k)|_B. Also bitreverse_1 = id. For q >= 2 the two tensors differ: at the input |1>|1> the component-wise pairing gives 1 and the nested pairing gives 0. QED

**Lemma 2.12 (Bell-circuit realisation of the cup)** [spec C8, (1.4), (2.6)]. For one qubit pair (a, b),

    (<0|_a <0|_b) H_a CNOT(a -> b) = 2^{-1/2} <cup_1|.

Hence, on q pairs, (<0|^{(x)2q}) prod_r (H_{a_r} CNOT(a_r -> b_r)) = 2^{-q/2} <cup_q|. The direct functional <00| + <11| equals <cup_1| exactly.

*Proof.* CNOT(a -> b) maps |x>_a |y>_b to |x>_a |x xor y>_b [spec (0.5)]. We have <0|H|x> = 2^{-1/2} for x in {0,1}, and <0|x xor y> = delta_{xy}. So the amplitude is 2^{-1/2} delta_{xy}, which is 2^{-1/2} <cup_1| evaluated on |x>|y>. The q-pair statement follows from Proposition 2.11(ii). QED

If the cups of widths q_e are realised this way, the postselected amplitude is 2^{-sum_e q_e / 2} sigma, and P_post = Z / 2^{sum_e q_e} [spec (1.4)]. For N TV N at q_n = 1 this is Z/4 [spec Sec. 8].

**Lemma 2.13 (the factor 2^{-q/2} is optimal).** Let A be the set of live qubits (two fields of q qubits each) and B an ancilla register. Suppose c <cup_q| = (<0|_{A,B}) Vu (id_A (x) |0>_B) for some unitary Vu and scalar c. Then |c| <= 2^{-q/2}. Lemma 2.12 attains this bound.

*Proof.* The right-hand side is a row vector of Euclidean norm at most ||Vu||_op = 1. The vector <cup_q| has norm sqrt(2^q). QED

## 2.4 Word fields, the functor and the sentence morphism

**Definition 2.14 (word fields)** [spec C7, (0.9)]. Let w be a word of type t_0 ... t_{m-1}. Factors are indexed from 0. Put q(n^{(k)}) = q_n and q(s^{(k)}) = q_s for every winding k, fo_w[j] = sum_{j' < j} q(t_{j'}), and Q_w = sum_j q(t_j). The tensor view of the word is phi_w[v_0 + 2^{fo_w[1]} v_1 + ... + 2^{fo_w[m-1]} v_{m-1}].

In a register that holds the words in order, off_w = sum_{w' < w} Q_{w'}. Type position p has start(p) = off_{word(p)} + fo_{word(p)}[factor(p)] and width(p) = q(b_p).

**Lemma 2.15 (the C7 index map is an isomorphism)** [spec C7, Sec. 1.1]. The map (v_0, ..., v_{m-1}) |-> sum_j v_j 2^{fo_w[j]}, with 0 <= v_j < 2^{q(t_j)}, is a bijection onto {0, ..., 2^{Q_w} - 1}. It induces a unitary from the tensor product of the factor spaces onto C^{2^{Q_w}}, with the first factor in the lowest bits. For a cup (p, p'), width(p) = width(p').

*Proof.* Since fo_w[j+1] = fo_w[j] + q(t_j), the bit ranges [fo_w[j], fo_w[j+1]) are disjoint and tile [0, Q_w). The value v_j is exactly the content of bits [fo_w[j], fo_w[j+1]) of the image, so the map is the binary mixed-radix decomposition and is a bijection. It maps basis vectors to basis vectors, so the induced map is unitary. A cup joins b^l b or b b^r for a single base type b, and q is constant on the windings of b. QED

**Definition 2.16 (DisCoCat functor)** [spec Sec. 1.2, C7]. Fr: RigFree({N, S}) -> FHilb is the strict monoidal functor with:

- Fr(n^{(k)}) = C^{2^{q_n}} and Fr(s^{(k)}) = C^{2^{q_s}};
- Fr on strings realised through Lemma 2.15, and Fr(1) = C;
- every cup of type b sent to <cup_{q(b)}| and every cap sent to |cap_{q(b)}>.

**Proposition 2.17 (Fr is well defined and preserves the rigid structure).** The functor Fr exists and is unique, and Fr(X Y) = Fr(X) (x) Fr(Y). It sends the cups and caps to morphisms that satisfy the snake equations. It does NOT factor through the posetal pregroup: the example of Definition 2.8 exhibits two parallel morphisms of RigFree with different images.

*Proof.* Existence and uniqueness come from the universal property of RigFree (Definition 2.8, cited), because the chosen images of the cups and caps satisfy the snake equations (Lemma 2.10). Strictness of the tensor is Lemma 2.15. QED

*Remark (what the rest of the document needs).* Theorem 2.20 (forward) and every later result use Fr only on morphisms rho_M for planar reductions M. On those, Fr(rho_M) is defined directly by the cup index-sum of Proposition 2.18 (forward), which does not depend on the order in which the cups are applied. The universal property, whose citation details are UNVERIFIED, is needed only for the claim that Fr is a functor on all of RigFree(B), and no later result uses that claim.

**Proposition 2.18 (independence of cup order).** Let the cups (A_e, B_e), e = 1..n_cup, of a reduction be applied to a state Psi, one at a time and in any order. The result is always

    Res[s] = sum_{k_1..k_{n_cup}} Psi[ fields A_e, B_e set to k_e; survivor field set to s ].

*Proof.* Each application of <cup_q| to the fields (A_e, B_e) replaces the current tensor by the sum over k_e of its entries with both fields equal to k_e. After all the cups have been applied, the result is the stated finite sum, independent of the order, because finite sums commute. This covers nested cups too, for which Lemma 2.2 alone does not apply. QED

**Definition 2.19 (word meaning; sentence morphism; normaliser)** [spec (1.2), (1.4)]. The meaning of word w is the state phi_w = U_w(theta_w)|0>^{(x)Q_w}. For a sentence w_1 ... w_{n_w} with reduction rho, the sentence meaning is

    sigma := Fr(rho) o (phi_{w_1} (x) ... (x) phi_{w_{n_w}}) in C^{2^{q_s}},        Z := sum_s abs(sigma[s])^2.   (2.4.1)

**Theorem 2.20 (component formula)** [spec (1.3), (1.5)]. With Psi[i] = prod_w phi_w[(i >> off_w) & (2^{Q_w} - 1)],

    sigma[s] = sum_{k_1..k_{n_cup}} Psi[ sum_e k_e (2^{start(A_e)} + 2^{start(B_e)}) + s 2^{start(F_S)} ],     (2.4.2)

where start(F_S) is the start of the survivor field F_S. For N TV N this gives sigma[s] = sum_{a,b} n1[a] V[a,s,b] n2[b], with no complex conjugation.

*Proof.* By Lemma 2.15 applied to the whole register (widths Q_w, offsets off_w), the product state has components Psi[i]. By Proposition 2.18, Fr(rho) acts by the index sum. The disjoint fields combine additively in the flat index. The cup is bilinear, not sesquilinear, so no conjugation occurs. N TV N has two cups: (n1.n, V.n^r) on field a and (V.n^l, n2.n) on field b. QED

**Remark 2.21 (frames and the Choi correspondence)** [addendum Sec. 14.1, (14.3)-(14.4), 13.2(a)]. Using the symmetry gamma to move the s leg to the output, the verb state V is the name (Lemma 2.5) of a frame f: C^d (x) C^d -> C^{d_s}, f[s; a, b] = V[a,s,b].

Now let V[a,b] be a unit vector in C^d (x) C^d (the case d_s = 1, or s fixed). It is the name of Mn: C^d -> C^d with Mn[b][a] = V[a,b]. Its reduced state on the first factor is rho_1 = (Mn^dag Mn)^T. So sqrt(d) Mn is unitary iff rho_1 = I/d, that is, iff V is maximally entangled. This fails for generic V. So the DisCoCat word state U_V|0> and the DisCoCirc gate U_V (Section 2.7) are different mathematical objects.

## 2.5 The bent-wire rewrite

**Lemma 2.22 (state to effect)** [spec Sec. 1.4, C8, C11]. For phi in C^{2^q}, <cup_q| (phi (x) id) = bra_T(phi) = sum_a phi[a] <a|. If phi = U|0>, then bra_T(phi) = <0| U^T. If U is the gate list g_1, ..., g_m (so U = g_m ... g_1), then U^T = g_1^T ... g_m^T, which is the gate list g_m^T, ..., g_1^T: reversed order, each gate transposed. A gate on a subset of qubits transposes factorwise. The transposes of the specific gates are listed in Lemma 3.18 (forward).

*Proof.* sum_k <k|<k| (phi (x) id) = sum_k phi[k] <k|. Also phi[a] = U[a][0] = (U^T)[0][a], so bra_T(phi) = <0|U^T. The list rule is (g_m ... g_1)^T = g_1^T ... g_m^T. In a Kronecker product, (A (x) B)^T = A^T (x) B^T. QED

**Definition 2.23 (bent-wire assignment)** [spec Sec. 1.4]. Given a parse, a *state/effect assignment* colours each word STATE or EFFECT so that:

- (a) the word owning the survivor is STATE;
- (b) every cup joins an EFFECT factor to a STATE factor.

For an EFFECT word e, wire_map_e sends local qubit fo_e[j] + r to start(partner of factor j) + r under the component-wise pairing, or to start + q - 1 - r under the nested pairing. K is the union of the images of all wire maps. STATE words occupy consecutive fields in sentence order, and W_hw = sum over STATE words of Q_w.

**Theorem 2.24 (the bent-wire circuit is exact)** [spec Sec. 1.4, (1.3), (1.4), Sec. 8, T5, T6]. Take an assignment as in Definition 2.23. Let Psi_St be the tensor product over STATE words of phi_w, on W_hw qubits, and let U_e^T act on the qubits wire_map_e(0..Q_e - 1). Then

    sigma[s] = ( <0|_K (x) <s|_{F_S} ) ( prod_e U_e^T ) Psi_St                                           (2.5.1)

exactly, with no 2^{-q/2} factor, and the order of the product is irrelevant. If every word state is a unit vector, then p_surv := sum_s abs((<0|_K (x) <s|_{F_S})(prod_e U_e^T) Psi_St)^2 = Z.

*Proof.* By Proposition 2.18 we may apply the cups word by word. Fix an EFFECT word e whose factors j cup STATE fields P_j. Applying its cups to phi_e (x) (rest) gives the effect bra_T(phi_e) on the tuple of fields (P_j), read in C7 order (Lemmas 2.15 and 2.22). That effect is <0|U_e^T transported to those fields. Transport is conjugation by the permutation of qubits given by wire_map_e, which is unitary. So U_e^T on the mapped qubits is the gate list of Lemma 2.22 with its qubit indices relabelled.

Distinct EFFECT words act on disjoint sets of qubits, because each position is cupped at most once, so by Lemma 2.2 they commute. After all the effects, the only field left uncontracted is the survivor field, and (2.5.1) is its component s. Finally p_surv = sum_s abs(sigma[s])^2 = Z. QED

**Proposition 2.25 (the assignment is forced; odd cycles)** [spec Sec. 1.4; addendum (11.22); spec (2.6)]. Let the word graph have one node per word and one edge per cup; multi-edges are allowed and do not create odd cycles.

- (i) A word can be an EFFECT only if it is a bare state U_w|0> and every one of its factors cups a STATE factor.
- (ii) Given (a) of Definition 2.23, the colouring is determined on the connected component of the survivor word by colour(neighbour) = not colour(word). On every other component it is unique up to a global flip. A valid assignment exists iff every component is bipartite.
- (iii) For the eight shapes of Proposition 1.23 the word graph is a tree and the assignment is unique. W_hw equals:
  - N IV: q_n + q_s
  - N TV N: 2 q_n + q_s
  - ADJ N TV N and N TV ADJ N: 3 q_n + q_s
  - ADJ N TV ADJ N: 4 q_n + q_s
  - N IV ADV: q_n + 2 q_s
  - ADJ N IV: 2 q_n + q_s
  - ADJ N IV ADV: 2 q_n + 2 q_s
- (iv) If a component contains an odd cycle, some cup joins two words of the same colour. Both words must then be prepared as STATES, and that cup realised on two live fields as the Bell functional of Lemma 2.12. That realisation has amplitude factor 2^{-q/2}, and by Lemma 2.13 no circuit with one unitary, ancillas and postselection does better.
- (v) A contracted intermediate (for example the vector ADJ.n) is in general not a unit vector, so it is not of the form U|0> for a unitary U.

*Proof.* (i) Lemma 2.22 needs phi_e = U_e|0> and a partner state on each factor. (ii) Condition (b) forces adjacent words to have opposite colours. On a connected graph this propagates uniquely from any fixed node, and it is consistent iff there is no odd cycle. (iii) The cup sets of Proposition 1.23 give trees. Colour them from the survivor word and add up Q_w over the STATE words. For example, ADJ N TV N has STATE = {N, TV}, giving q_n + (2 q_n + q_s). (iv) An odd cycle cannot be 2-coloured, so some edge is monochromatic. By (b) of Definition 2.23, an EFFECT needs a live STATE partner, so both ends of that edge are STATE, and the cup must be applied to two live fields. (v) ||sum_b ADJ[a,b] n[b]||^2 is a polynomial in the angles and is not identically 1. For example, ADJ = |0>|0> gives ||.||^2 = abs(n[0])^2. QED

**Corollary 2.26 (wire maps at (q_n, q_s) = (1, 1))** [spec Sec. 1.4; T5-T7]. Number the STATE-word qubits from left to right. Then:

- N IV: IV on q0 = n^r, q1 = s. N^T on [0]. K = {0}, survivor qubit 1.
- N TV N: TV on q0..q2. N1^T on [0], N2^T on [2]. K = {0,2}, survivor qubit 1.
- ADJ N TV N: N on q0, TV on q1..q3. ADJ^T on [1,0], N2^T on [3]. K = {0,1,3}, survivor qubit 2.
- N TV ADJ N: TV on q0..q2, N2 on q3. N1^T on [0], ADJ^T on [2,3]. K = {0,2,3}, survivor qubit 1.
- ADJ N TV ADJ N: N1 on q0, TV on q1..q3, N2 on q4. ADJ1^T on [1,0], ADJ2^T on [3,4]. K = {0,1,3,4}, survivor qubit 2.
- N IV ADV: N on q0, ADV on q1 = s^r and q2 = s. IV^T on [0,1]. K = {0,1}, survivor qubit 2.
- ADJ N IV: N on q0, IV on q1 = n^r and q2 = s. ADJ^T on [1,0]. K = {0,1}, survivor qubit 2, W = 3 = 2 q_n + q_s.
- ADJ N IV ADV: ADJ on q0 = n and q1 = n^l, ADV on q2 = s^r and q3 = s. N^T on [1], IV^T on [0,2]. K = {0,1,2}, survivor qubit 3, W = 4 = 2 q_n + 2 q_s.

In each case M_post = OR_{k in K} 2^k and b(c) = c << f_S. For general (q_n, q_s), each qubit becomes a field of width q(b).

*Proof.* Apply Definition 2.23 to the cups of each shape. For example, in ADJ N TV N the adjective's n (factor 0) cups TV.n^r (qubit 1), and its n^l (factor 1) cups N.n (qubit 0), so wire_map = [1, 0]. The other entries are read off in the same way from the unique reductions of Proposition 1.23. For ADJ N IV the reduction is (1,2)(0,3): the tree is IV (root) - ADJ - N, so STATE = {N, IV}, and ADJ's n cups IV.n^r (qubit 1) and its n^l cups N.n (qubit 0). For ADJ N IV ADV the reduction is (1,2)(0,3)(4,5): the tree is ADV (root) - IV - ADJ - N, so STATE = {ADJ, ADV}; N's n cups ADJ.n^l (qubit 1), and IV's n^r and s cup ADJ.n (qubit 0) and ADV.s^r (qubit 2). Relabelling physical qubits is conjugation by a permutation unitary and leaves (2.5.1) unchanged. QED

The worked example of spec Sec. 8 (W = 3, K = {0,2}, survivor qubit 1, p_surv = Z; the Bell form on W = 5 gives sigma/2 and Z/4) is the N TV N instance of Theorem 2.24 and Lemma 2.12. Its decimals are numerically checked in the source and are not re-derived here.

## 2.6 Frobenius algebras and spiders

**Definition 2.27 (special commutative dagger-Frobenius algebra).** In a symmetric monoidal category, a Frobenius algebra on A is a quadruple (mu: A (x) A -> A, u_F: I -> A, Delta: A -> A (x) A, e_F: A -> I) such that:

- (mu, u_F) is an associative unital monoid;
- (Delta, e_F) is a coassociative counital comonoid;
- the Frobenius law holds: (id (x) mu)(Delta (x) id) = Delta mu = (mu (x) id)(id (x) Delta).

It is *special* if mu Delta = id, *commutative* if mu gamma = mu, and *dagger* (in FHilb) if Delta = mu^dag and u_F = e_F^dag.

**Proposition 2.28 (the basis spider)** [addendum (11.6)-(11.9)]. On C^{2^q} define

    mu|i>|j> = delta_{ij}|i>,   u_F = sum_i |i>,   Delta|i> = |i>|i>,   e_F|i> = 1.                      (2.6.1)

Equivalently, (mu(va (x) vb))[i] = va[i] vb[i], Delta(va) = sum_i va[i]|i>|i>, and e_F(va) = sum_i va[i]. This is a special commutative dagger-Frobenius algebra. All of its structure maps are components of the spider tensors delta^{(k)}. In bits, "all indices equal" means that all q bit pairs are equal (component-wise, C8).

*Proof.*
- Associativity: mu(mu (x) id)|ijk> = delta_{ij} delta_{ik}|i> = mu(id (x) mu)|ijk>.
- Unit: mu(u_F (x) id)|j> = sum_i delta_{ij}|j> = |j>.
- Frobenius law: (id (x) mu)(Delta (x) id)|ij> = (id (x) mu)|iij> = delta_{ij}|ii> = Delta mu|ij>. The other equality is symmetric.
- Special: mu Delta|i> = |i>.
- Commutative: delta_{ij} is symmetric in i and j.
- Dagger: <jk|Delta|i> = delta_{ij}delta_{ik} = <i|mu|jk>^*, so Delta = mu^dag.
- Coassociativity and counit are the daggers of associativity and unit.

Components: mu[k][i,j] = delta^{(3)}[i,j,k], Delta[j,k][i] = delta^{(3)}[i,j,k], and e_F[i] = sum_{j,k} delta^{(3)}[i,j,k] = 1. QED

Coecke, Pavlovic and Vicary (*A new description of orthogonal bases*, Math. Struct. Comp. Sci. 2013) show that every special commutative dagger-Frobenius algebra on a finite-dimensional Hilbert space has the form (2.6.1) for a unique orthonormal basis. We use only the direction proved above. The *spider theorem* (Lack 2004; Coecke, Paquette and Pavlovic, "Classical and quantum structuralism"; citation details UNVERIFIED) says that any connected diagram built from (2.6.1) with a inputs and b outputs equals delta^{(a+b)}. It is cited, and every tensor used below is verified directly.

**Lemma 2.29 (spiders induce the cup)** [addendum (11.6); spec C8]. Under the component-wise pairing, e_F o mu = <cup_q| and Delta o u_F = |cap_q>. So a cup is a two-legged spider, and a spider carries no state/effect colour. Under the nested pairing, the cup is e_F o mu o (id (x) Pm), with Pm the permutation of Proposition 2.11(iii).

*Proof.* e_F mu|ij> = delta_{ij} = <cup_q| evaluated on |ij>, and Delta u_F = sum_i |ii>. QED

**Lemma 2.30 (non-unitarity and circuit realisation)** [addendum (11.7)-(11.9); spec (1.4), (1.6), (2.6); addendum T13]. Let q >= 1.

- (i) mu: C^{2^{2q}} -> C^{2^q} is surjective, with a kernel of dimension 4^q - 2^q >= 2. No isometry equals it.
- (ii) Restricted to product inputs va (x) vb, the map (id (x) <0|^{(x)q}) o prod_r CNOT(x_r -> y_r) equals mu(va (x) vb) exactly, with amplitude factor 1.
- (iii) Delta = prod_r CNOT(x_r -> y_r) o (id (x) |0>^{(x)q}), and Delta is an isometry.
- (iv) e_F = 2^{q/2} <0|^{(x)q} H^{(x)q}. So a deleted q-qubit wire costs 2^{-q/2} in amplitude and 2^{-q} in probability, and this factor cancels in p_c = N_c / D.

*Proof.* (i) mu|ii> = |i> gives surjectivity, so the rank is 2^q on a 4^q-dimensional domain. Isometries are injective. (ii) The CNOT ladder maps |i>|j> to |i>|i xor j>, which gives sum_{ij} va_i vb_j |i>|i xor j>. Projecting the second register onto |0> keeps only j = i, leaving sum_i va_i vb_i |i>. (iii) |i>|0> |-> |i>|i> = Delta|i>, and Delta^dag Delta|i> = sum_j <jj|ii> |j> = |i>. (iv) <0|H|b> = 2^{-1/2} for b in {0,1}. A global scalar cancels in a ratio. QED

**Definition 2.31 (relative pronoun and coordination tensors)** [addendum (11.10)-(11.13'), C15]. In C7 factor order (first factor in the lowest bits), and with an unconstrained leg carrying the all-ones factor delta^{(1)}:

    who  [a (n^r), b (n), c (s^l), e (n)]         = delta^{(3)}[a,b,e],    index a + 2^{q_n} b + 2^{2q_n} c + 2^{2q_n+q_s} e,
    whom [a (n^r), b (n), e (n^ll), c (s^l)]      = delta^{(3)}[a,b,e],    index a + 2^{q_n} b + 2^{2q_n} e + 2^{3q_n} c,
    and_n[a (n^r), b (n), c (n^l)]                = delta^{(3)}[a,b,c],
    and_vp[x (s^r), y (n^rr), y' (n^r), z (s), x' (s^l), y'' (n)] = delta^{(3)}[y,y',y''] delta^{(3)}[x,z,x'],
    and_s[x (s^r), z (s), x' (s^l)]               = delta^{(3)}[x,z,x'],   index x + 2^{q_s} z + 2^{2q_s} x'.

These tensors follow Sadrzadeh, Clark and Coecke, "The Frobenius anatomy of word meanings I/II", J. Logic Comput. 2013/2016 (citation details UNVERIFIED). Each is a spider word: it has no parameters, and its entries are products over basic types of "all legs of that type are equal", with a leg permutation as needed when the basic types interleave.

**Proposition 2.32 (sentence formulas)** [addendum (11.14)-(11.17'), T12, T13]. Let V[a,s,b] be as in Theorem 2.20 and IV[a,s] = phi[a + 2^{q_n} s]. Then:

    "Alice who loves Bob sleeps":  sigma[s] = sum_a Alice[a] ( sum_c sum_b V[a,c,b] Bob[b] ) IV[a,s],
    "Alice whom Bob loves sleeps": sigma[s] = sum_a Alice[a] ( sum_c sum_b Bob[b] V[b,c,a] ) IV[a,s],
    "Alice and Bob sleep":         sigma[s] = sum_a Alice[a] Bob[a] IV[a,s],
    "Alice sleeps and dreams":     sigma[s] = sum_a Alice[a] IV1[a,s] IV2[a,s],
    "[S1] and [S2]":               sigma[s] = sigma_1[s] sigma_2[s].

*Proof.* Each formula is Theorem 2.20 with the tensors of Definition 2.31 inserted.

"Alice who loves Bob sleeps" has positions Alice n(0); who n^r(1) n(2) s^l(3) n(4); loves n^r(5) s(6) n^l(7); Bob n(8); sleeps n^r(9) s(10). Its cups are (0,1), (2,9), (3,6), (4,5), (7,8). Contracting who = delta^{(3)}[a,b,e] against Alice, IV, V and Bob identifies Alice's index, the subject leg of IV and the subject leg of V. The s^l leg sums the sentence leg c of V freely.

"Alice whom Bob loves sleeps" (cups (0,1), (5,6), (4,7), (3,8), (2,9), with s at 10): whom's n^ll cups loves.n^l, so the pronoun ties V's high (object) field to Alice's index, and Bob enters V's low field.

"Alice and Bob sleep": and_n ties Alice, Bob and the subject leg of IV. "Alice sleeps and dreams": and_vp ties Alice's noun to both n^r legs and both sentence outputs to the survivor. "[S1] and [S2]": and_s applies mu to the two sentence vectors (Proposition 2.28).

The orientation V[a,c,b] versus V[b,c,a] matters: for generic V the two sums differ. Addendum T12 checks both relative-clause orientations against brute force. QED

**Corollary 2.32' (complementiser orientation)** [addendum (11.14'), T14]. In "Alice believes that Bob sleeps" = n | n^r s s^l | s s^l | n | n^r s (positions 0..8), the reduction is (0,1)(3,4)(6,7)(5,8) with s at 2. The complementiser that = s s^l has factor 0 (s, low bits) cupped to believes.s^l and factor 1 (s^l, high bits) cupped to sleeps.s. Put t_sleeps[c'] = sum_b Bob[b] IV_sleeps[b, c'] and Mth[c][c'] = phi_that[c + 2^{q_s} c']. Then the vector that enters the cup with believes.s^l is

    t_that[c] = sum_{c'} phi_that[c + 2^{q_s} c'] t_sleeps[c'] = (Mth t_sleeps)[c],

a contraction over the HIGH index of phi_that. The transposed form sum_{c'} phi_that[c' + 2^{q_s} c] t_sleeps[c'] = (Mth^T t_sleeps)[c] equals it for every t_sleeps iff Mth is symmetric. Example: at q_s = 1 with phi_that = e_1 and t_sleeps = e_0, the correct form gives e_1 and the transposed form gives 0.

*Proof.* The cups are admissible, nested and leave position 2 unenclosed; the reduction is unique by Theorem 1.10 (m = 9). By Lemma 2.15, factor j of that occupies the bits [j q_s, (j+1) q_s) of its index, so its s^l leg is the high digit c'. The cup (5,8) ties c' to the output leg of the contracted sub-sentence "Bob sleeps", which by Theorem 2.20 is t_sleeps; the cup (3,4) leaves c open towards believes. The two forms are the maps t -> Mth t and t -> Mth^T t, which agree on all t iff Mth = Mth^T. In the example Mth = |1><0|, so Mth e_0 = e_1 and Mth^T e_0 = 0. QED

**Proposition 2.33 (sum semantics for coordination)** [addendum (11.12'), text after (11.17')]. Let and_n[a,b,c] = delta_{ab} + delta_{bc}, with the free index summed against the all-ones vector. Then "Alice and Bob sleep" gives sigma[s] = sum_b (Alice[b] T_Bob + T_Alice Bob[b]) IV[b,s], where T_x = sum_i x[i]. With and_s = delta_{xz} + delta_{zx'}, "[S1] and [S2]" gives sigma = sigma_1 T_2 + T_1 sigma_2.

*Proof.* The contraction is bilinear. The term delta_{ab} contracts Alice with the verb index and sums Bob against the all-ones vector, which gives Alice[b] T_Bob. The second term is symmetric. QED

A fully learned "and" is an ordinary word of type n^r n n^l, and Theorem 2.20 covers it without any Frobenius structure.

**Conjecture 2.34 (spiders in the bent-wire circuit)** [addendum (11.22), T18; spec (2.6), Sec. 1.4]. For every grammatical sentence of the addendum lexicon there is a colouring of the unitary words together with a realisation of each spider, built from Lemma 2.30(ii)-(iv), Theorem 2.24 and Lemma 2.12, whose postselected amplitude is

    sigma[s] x 2^{-q/2} per e_F leg x 2^{-q/2} per Bell cup.

All factors cancel in p_c [spec (1.6)].

What is proved: the building-block identities of Lemma 2.30, Theorem 2.24 and Lemma 2.12. What is checked: the instance "Alice who loves Bob sleeps" with STATES {loves, sleeps} and EFFECTS {Alice, Bob} (addendum T18). A general colouring rule, including e_F legs attached to EFFECT words, is UNVERIFIED.

## 2.7 DisCoCirc: texts as composition in a symmetric monoidal category

**Definition 2.35 (text register)** [addendum C17, (13.5), (13.6), (14.1), (14.2)]. A text with N_e distinct nouns is interpreted on (C^d)^{(x)N_e} = C^{2^W}, where W = q_n N_e. Noun wire j is the field [q_n j, q_n (j+1)), and wires are numbered by first mention.

The initial state is Psi_0 = (x)_j phi_{n_j}, so Psi_0[i] = prod_j phi_{n_j}[(i >> q_n j) & (d-1)]. Each clause t contributes a map G_t on C^{2^W}, equal to a gate on the mentioned fields tensored with the identity elsewhere. The text state is Psi_{T_cl} = G_{T_cl} ... G_1 Psi_0, where T_cl is the number of clauses and G_1 comes first (text order).

A two-wire verb gate is a C10 ansatz on 2 q_n qubits. Its local qubits 0..q_n - 1 are the subject and q_n..2q_n - 1 the object. It is transported by wire_map(j) = q_n i_subj + j for j < q_n, and q_n i_obj + (j - q_n) for j >= q_n.

**Lemma 2.36 (index formula and field swap)** [addendum (14.1), C17].

- (i) Psi_0[i] as above is the component formula of (x)_j phi_{n_j} under Lemma 2.15.
- (ii) Let G be a dense verb gate on the local index m = a + d b (a = subject). When the subject field lies ABOVE the object field, G is applied as G' = Pm G Pm^T, where Pm|a + d b> = |b + d a>. Equivalently, G'[pi(m_out)][pi(m_in)] = G[m_out][m_in] with pi(m) = (m >> q_n) | ((m & (d-1)) << q_n).

*Proof.* (i) is Lemma 2.15. (ii) pi(a + d b) = b + d a, so Pm|m> = |pi(m)>, and (Pm G Pm^T)[pi(m_out)][pi(m_in)] = G[m_out][m_in]. A gate given as a gate list is defined by bit tests on the mapped qubits, so relabelling the qubits already does the transport. QED

**Proposition 2.37 (Frobenius bending is diagonal and commutative)** [addendum (14.3), (11.6), (11.8)]. Let f: (C^d)^{(x)k} -> C^{d_s} be a frame and ef a functional on C^{d_s} (for example ef = 2^{-q_s/2}(1, ..., 1), or a learnt one). Define G_F by: copy each input wire with Delta, feed one copy of every wire to f, and close with ef. Then

    G_F[a'; a] = prod_j delta(a'_j, a_j) v[a],    where v[a] = sum_s ef[s] f[s; a].

G_F is diagonal, any two such gates commute, and G_F is unitary iff abs(v[a]) = 1 for all a.

*Proof.* Delta|a_j> = |a_j>|a_j>. Feeding the second copies to f and closing with ef multiplies |a> by v[a] and leaves the first copies unchanged. So G_F = diag(v). Diagonal matrices commute, and a diagonal matrix is unitary iff all its entries have modulus 1. QED

So a text built from Frobenius-bent sentences depends only on its multiset of sentences, not their order. Unitary bending [addendum (14.4)] takes G_V = U_ansatz(theta_V) in U(d^2), so p_surv = 1. Which gate set the DisCoCirc literature uses is UNVERIFIED.

## 2.8 The CPM construction; density operators; CPTP maps

**Definition 2.38 (CPM(FHilb); Selinger, "Dagger compact closed categories and completely positive maps", ENTCS 170 (2007)).** CPM(FHilb) has the objects of FHilb. A morphism A -> B is a linear map Lin(A) -> Lin(B) of the form rho |-> Tr_E[Kr rho Kr^dag] for some Kr: A -> B (x) E, that is, a completely positive (CP) map. The doubling Dbl(f)(rho) = f rho f^dag embeds FHilb in CPM(FHilb).

**Theorem 2.39 (Selinger; Choi 1975; cited).** CPM(FHilb) is a dagger compact closed category, with Dbl(cup) and Dbl(cap) as its cup and cap. A map is CP iff it has a Kraus form Phi(rho) = sum_k Kr_k rho Kr_k^dag.

**Proposition 2.40 (specialisations)** [limits l.184, l.206, l.638; addendum (11.7)-(11.9), (13.7)].

- (i) Dbl(phi) = |phi><phi|. The doubled sentence morphism is |sigma><sigma|, and Z = Tr |sigma><sigma|.
- (ii) Discarding a wire is the FHilb cup that pairs the ket and bra indices of the doubled wire: Tr(rho) = sum_k <k|rho|k>. It is CP, with Kraus operators <k|. The partial trace is Tr_R := id_{Lin(A)} (x) Tr_{Lin(R)}. Tr is the unique linear functional on Lin(A) that equals 1 on every density operator. (The limits document calls this map "the counit (cap)"; it is the cup of this document.)
- (iii) The doubled Frobenius counit Dbl(e_F)(rho) = sum_{a,a'} rho[a][a'] = <u_F|rho|u_F> is postselection on the unnormalised all-ones vector. It is a different effect from Tr.
- (iv) Dbl(mu)(rho (x) tau) = rho .* tau (entrywise product) is CP. The doubled copy Dbl(Delta)(rho) places rho[a][a'] at ((a,a),(a',a')). On diagonal inputs these give the classical copy and merge.
- (v) The DisCoCirc update rho_{t+1} = Tr_{anc}[U_t (rho_t (x) |0><0|_{anc}) U_t^dag] is CPTP, with Kraus operators Kr_k = (id (x) <k|_{anc}) U_t (id (x) |0>_{anc}). The postselected update keeps Kr_0 alone. It is CP, but not trace preserving before renormalisation.

*Proof.* (i) Dbl(phi) = phi (x) conj(phi) = |phi><phi|, and Tr|sigma><sigma| = Z.

(ii) The Kraus form is Tr(rho) = sum_k <k|rho|k>. Uniqueness: density operators span Lin(A), so a linear functional that equals 1 on all of them is fixed, and it coincides with Tr.

(iii) Take q = 1. For rho = (|0> - |1>)(<0| - <1|)/2, Dbl(e_F) gives 0 while Tr gives 1.

(iv) Dbl(mu) has the single Kraus operator mu, and (mu (rho (x) tau) mu^dag)[a][a'] = rho[a][a'] tau[a][a']. Dbl(Delta)(rho) = Delta rho Delta^dag has entries <aa|Delta rho Delta^dag|a'a'> = rho[a][a'].

(v) sum_k Kr_k^dag Kr_k = (id (x) <0|) U_t^dag (id (x) sum_k |k><k|) U_t (id (x) |0>) = id. The trace of the retained branch is Tr(Kr_0 rho Kr_0^dag) = p_t, which is < 1 in general. QED

**Definition 2.41 (words as CP maps; higher-order boxes)** [limits l.184, l.810, HO.2-HO.4]. In CPM(FHilb) a DisCoCirc word acting on the wires it mentions is a CP map Phi(rho) = sum_{k < r_K} Kr_k rho Kr_k^dag of Kraus rank r_K. The canonical noun updates are parametrised by a positive operator omega = Uo diag(r) Uo^dag = sum_i r_i Pr_i (after Coecke and Meichanetzidis 2020, arXiv:2001.00862; wording UNVERIFIED).

A *higher-order box* is a linear map Fb: Lin((C^d)^{(x)K}) -> Lin((C^d)^{(x)K'}), that is, a superoperator acting on a sentence box. Special cases are the sandwich B' = V1 B V2 with unitaries V1, V2; the controlled frame; the hedge B' = (1 - h) I + h B; and the reversal B^dag. These are definitions only.

**Remark 2.42 (qudit and mixed-radix version)** [addendum Sec. 12.5, (12.16'); limits l.1110]. Replace C^{2^q} by C^{d_t} with arbitrary integers d_n and d_s. The following results carry over verbatim:

- Sections 2.1-2.2;
- Lemma 2.15, with mixed-radix strides prod_{j' < j} d(t_{j'}) (Lemma 4.2, forward);
- Definition 2.16, Propositions 2.17-2.18 and Theorem 2.20;
- Definition 2.27 through Proposition 2.33, as algebra;
- Section 2.8.

Over the reals (SO(d) rotations, real spiders) the same proofs apply.

The qubit-circuit results do NOT carry over verbatim: Proposition 2.11(ii)-(iii), Lemmas 2.12, 2.22 (as gate lists) and 2.30(ii)-(iv), and Corollary 2.26. Their qudit analogues replace CNOT by SUM: |x>|y> -> |x>|x + y mod d>, and H by the Fourier matrix F_d, with <0|F_d|x> = d^{-1/2}. The factors 2^{-1/2} become d^{-1/2}, and the kernel of mu has dimension d^2 - d. These analogues are UNVERIFIED here.

## 2.9 The commutative case: permutation-invariant K-ary composition

**Proposition 2.43 (order invariance of the Frobenius merge)** [addendum 11.3; review 5(a)]. The K-fold iterate J_mu(Lv_1, ..., Lv_K)[i] = prod_k Lv_k[i] of the multiplication mu is independent of the order and bracketing of its arguments. Stating this invariance requires the symmetry gamma: mu o gamma = mu, and gamma is not among the generators of a planar (non-symmetric) rigid category.

Addition J_+(Lv_1, ..., Lv_K) = sum_k Lv_k is also invariant under permutations of its arguments. However, it is not a linear map on (C^d)^{(x)K}, because it is not multilinear: (2va) (x) vb = va (x) (2vb), but 2va + vb != va + 2vb. It is the codiagonal of the biproduct (C^d)^{(+)K} -> C^d, which lies outside the tensor calculus of Sections 2.1-2.6. It is not a spider.

*Proof.* Entrywise products and sums of complex numbers are commutative and associative. Any permutation is a product of adjacent transpositions, and any bracketing can be reduced by associativity. QED

**Proposition 2.44 (norm collapse of the merge; growth of the sum)** [review 5(a); spec l.217]. Let Lv_1, ..., Lv_K be independent random unit vectors in C^d, each with a distribution invariant under permutations of coordinates and under Lv -> -Lv. Then

    E ||prod_k Lv_k||^2 = d^{-(K-1)},       E ||sum_k Lv_k||^2 = K.                                   (2.9.1)

By Markov's inequality, P(||prod_k Lv_k||^2 >= t) <= d^{-(K-1)} / t.

*Proof.* Permutation invariance and sum_i abs(Lv_k[i])^2 = 1 give E abs(Lv_k[i])^2 = 1/d. By independence, E prod_k abs(Lv_k[i])^2 = d^{-K}, and summing over i gives d^{1-K}. For the sum, E||sum_k Lv_k||^2 = K + sum_{k != l} <E Lv_k, E Lv_l> = K, because invariance under Lv -> -Lv gives E Lv_k = 0. QED

For example, at d = 64 and K = 8 the expected squared norm is 64^{-7} = 2.3e-13. This concerns squared norms and expectations only.
# Chapter 3. Hilbert-space semantics: gates, measurement and normalisation

## 3.1 The statevector and its index maps

**Definition 3.1 (register)** [spec C1]. A W-qubit state is a vector psi in C^N with N = 2^W. Qubit k is bit k of the index. The ket |b_{W-1} ... b_1 b_0> denotes e_i with bit_k(i) = b_k, so qubit 0 is the least significant and fastest-varying index.

**Definition 3.2 (Kronecker order)** [spec C2]. (A (x)_Kr B)[r_A dim B + r_B][c_A dim B + c_B] = A[r_A][c_A] B[r_B][c_B]. Here the left factor carries the high digits, and the subscript Kr marks this order.

**Lemma 3.3 (basis vectors factor).** e_i = e_{bit_{W-1}(i)} (x)_Kr ... (x)_Kr e_{bit_0(i)}. The factor for qubit k stands W - 1 - k places from the left.

*Proof.* Induction on W; the case W = 1 is trivial. For W > 1 write i = 2 i' + b_0 with i' < 2^{W-1}. By Definition 3.2 with dim B = 2, (e_{i'} (x)_Kr e_{b_0})[2r + c] = e_{i'}[r] e_{b_0}[c]. This equals 1 exactly when r = i' and c = b_0, that is, at the index i. Now apply the hypothesis to e_{i'}. QED

**Corollary 3.4 (a word adjoined in the high bits)** [spec (2.7), C2]. If psi is in C^{2^{W_live}} and phi is in C^{2^Q}, then

    (phi (x)_Kr psi)[x] = phi[x >> W_live] psi[x & (2^{W_live} - 1)]     for x < 2^{W_live + Q}.

The source index x & (2^{W_live} - 1) is at most x.

*Proof.* Apply Definition 3.2 with dim B = 2^{W_live}. The inequality holds because masking removes bits. QED

(The inequality is what makes an in-place evaluation in descending x possible. That use is engineering and is not treated here.)

**Definition 3.5 (bit-field insertion)** [limits T.2; spec C7]. Take a field of q bits starting at bit f, with f + q <= W. For r < 2^{W-q} and a < 2^q, put

    ins(r, f, q, a) = (r & (2^f - 1)) | (a << f) | ((r >> f) << (f + q)).

Three instances are named.

- For k <= W - 1: i0(j,k) = ins(j, k, 1, 0) [spec (2.2)].
- For lo < hi <= W - 1: tt = ins(j, lo, 1, 0), i00(j) = ins(tt, hi, 1, 0) [spec (2.3)], and i11(j) = i00(j) | 2^lo | 2^hi.
- The compaction map [spec (2.4)]:
      compact(i) = ((i >> (hi+1)) << (hi-1)) | (((i >> (lo+1)) & (2^{hi-lo-1} - 1)) << lo) | (i & (2^lo - 1)).

**Lemma 3.6 (bit-insertion enumeration)** [spec (2.2)-(2.5), T1].

- (a) For fixed f, q, a, the map r -> ins(r, f, q, a) is a strictly increasing bijection from [0, 2^{W-q}) onto {i < 2^W : (i >> f) & (2^q - 1) = a}. Its inverse is i -> (i & (2^f - 1)) | ((i >> (f+q)) << f).
- (b) i0(., k) is a strictly increasing bijection of [0, 2^{W-1}) onto {i : bit_k(i) = 0}. Moreover i0(j,k) >= j, with equality iff j < 2^k.
- (c) i00 is a strictly increasing bijection of [0, 2^{W-2}) onto {i : bit_lo(i) = bit_hi(i) = 0}. We have compact(i00(j)) = j, and i00(j) >= j with equality iff j < 2^lo. The four indices idx[m] = i00(j) | (bit_0(m) << lo) | (bit_1(m) << hi), m = 0..3, are exactly the indices that agree with i00(j) outside bits lo and hi, and m(idx[m]) = m.
- (d) i11(j) > i00(j) >= j.

*Proof.* (a) Write r = r_hi 2^f + r_lo with r_lo < 2^f. Then ins(r,f,q,a) = r_lo + a 2^f + r_hi 2^{f+q}, whose digits are r_lo in bits [0,f), a in bits [f, f+q) and r_hi above. So the image lies in the target set, and each element of the target set arises from exactly one pair (r_lo, r_hi). For monotonicity, let r < r'. Either r_hi < r'_hi, and then the images differ by at least 2^{f+q} - (2^f - 1) > 0; or r_hi = r'_hi and r_lo < r'_lo. The stated inverse deletes the field digits.

(b) is (a) with q = 1, a = 0. Also i0(j,k) - j = (j >> k) 2^k >= 0, which is zero iff j >> k = 0.

(c) i00 is a composition of two maps of type (a). The second insertion (at hi > lo) does not disturb the first. So i00(j) >= tt >= j, with equality iff j < 2^lo. compact removes the two zero bits by applying the inverse in (a) twice. The claim about idx[m] is Definition 3.1.

(d) is immediate. QED

For example, W = 4 and k = 1 give j = 0..7 -> 0, 1, 4, 5, 8, 9, 12, 13. W = 4, lo = 1, hi = 3, j = 2 give tt = 4, i00 = 4, idx = 4, 6, 12, 14 and compact(4) = 2 [spec (2.2), (2.4)].

## 3.2 The gate set

**Definition 3.7 (fixed matrices)** [spec (0.10), (0.4)]. I = [[1,0],[0,1]], Px = [[0,1],[1,0]], Py = [[0,-i],[i,0]], Pz = [[1,0],[0,-1]], H = (1/sqrt2)[[1,1],[1,-1]]. For Hermitian G, R_G(theta) := exp(-i theta G/2).

**Lemma 3.8 (rotation matrices)** [spec C5, (0.1)-(0.3), C13]. If G is Hermitian with G^2 = I, then R_G(theta) = co I - i si G, where co = cos(theta/2) and si = sin(theta/2). Hence

    Rx = [[co, -i si], [-i si, co]],   Ry = [[co, -si], [si, co]],   Rz = diag(e^{-i theta/2}, e^{+i theta/2}).

Rz(theta) = e^{-i theta/2} diag(1, e^{i theta}) differs from diag(1, e^{i theta}) by a global phase.

*Proof.* In the exponential series, the even powers of G equal I and the odd powers equal G. QED

**Definition 3.9 (local gates)** [spec C13, T2]. For a 2x2 matrix U and a qubit k, U_k has entries M[i_out][i_in] = U[bit_k(i_out)][bit_k(i_in)] when i_out and i_in agree off bit k, and 0 otherwise. For a 4x4 matrix U4 and qubits lo < hi, put mask = 2^lo + 2^hi; then U_{lo,hi} has entries M[i_out][i_in] = U4[m(i_out)][m(i_in)] when (i_out & ~mask) = (i_in & ~mask), and 0 otherwise.

**Lemma 3.10 (butterfly)** [spec (2.1), C13]. Write u_{rc} = U[r][c]. For every j < 2^{W-1}, let i0 = i0(j,k) and i1 = i0 | 2^k. Then psi' = U_k psi satisfies psi'[i0] = u_00 psi[i0] + u_01 psi[i1] and psi'[i1] = u_10 psi[i0] + u_11 psi[i1]. The matrix of Definition 3.9 is I (x)_Kr ... (x)_Kr U (x)_Kr ... (x)_Kr I, with U in the position of qubit k given by Lemma 3.3.

*Proof.* By Lemma 3.3 and multilinearity, (I (x) ... U ... (x) I) e_{i_in} = sum_b U[b][bit_k(i_in)] e_{i'}, where i' agrees with i_in except that bit_k(i') = b. This is Definition 3.9. Each row has two non-zero entries, at i_in in {i0, i1}. By Lemma 3.6(b), the pairs (i0, i1) partition [0, N). QED

**Lemma 3.11 (quad update; lifts)** [spec (2.3), (0.12), C6]. For every j < 2^{W-2}, with v[m] = psi[idx[m]], U_{lo,hi} psi = psi' satisfies psi'[idx[m_out]] = sum_{m_in} U4[m_out][m_in] v[m_in].

Define lift_lo(G)[m_out][m_in] = G[bit_0(m_out)][bit_0(m_in)] [bit_1(m_out) = bit_1(m_in)]; this is I (x)_Kr G. If U4 = lift_lo(G), then U_{lo,hi} = G_lo. Symmetrically, lift_hi(G) = G (x)_Kr I gives G_hi.

*Proof.* The first claim is Definition 3.9 read column by column, with the blocks enumerated by Lemma 3.6(c). For the lifts: bit_0(m(i)) = bit_lo(i) and bit_1(m(i)) = bit_hi(i). So the bracket together with the block condition says that i_out and i_in agree off bit lo, and the remaining factor is G[bit_lo(i_out)][bit_lo(i_in)]. QED

**Lemma 3.12 (controlled gates: bit-test definitions equal the dense forms)** [spec (0.5)-(0.8), (0.11), C6]. Let ctl != tgt, lo = min(ctl, tgt) and hi = max(ctl, tgt). The four bit-test rules below each equal U_{lo,hi} with the stated U4, written in the local order m = bit_lo + 2 bit_hi.

- CNOT(ctl -> tgt): psi'[i] = psi[i xor 2^tgt] if bit_ctl(i) = 1, and psi'[i] = psi[i] otherwise.
- CZ: psi'[i] = (-1)^{bit_ctl(i) bit_tgt(i)} psi[i].
- CRz(theta)(ctl -> tgt): psi'[i] = exp(i (2 bit_tgt(i) - 1) theta/2) psi[i] if bit_ctl(i) = 1, and psi'[i] = psi[i] otherwise.
- CRx(theta)(ctl -> tgt): apply Rx(theta) (Lemma 3.8) to the pair (psi[i], psi[i | 2^tgt]) for each i with bit_ctl(i) = 1 and bit_tgt(i) = 0.

The matrices are:

    CNOT(hi->lo) = [[1,0,0,0],[0,1,0,0],[0,0,0,1],[0,0,1,0]],   CNOT(lo->hi) = [[1,0,0,0],[0,0,0,1],[0,0,1,0],[0,1,0,0]],
    CZ = diag(1,1,1,-1),   CRz(hi->lo) = diag(1, 1, e^{-i theta/2}, e^{i theta/2}),   CRz(lo->hi) = diag(1, e^{-i theta/2}, 1, e^{i theta/2}),
    CRx(hi->lo): identity on m = 0,1 and Rx on the pair (2,3);   CRx(lo->hi): identity on m = 0,2 and Rx on the pair (1,3).

*Proof.* Each rule touches only the amplitudes inside one block {idx[m]} of Lemma 3.6(c), and it acts identically on every block. So U4 is found by evaluating the rule on the four local basis vectors.

- CNOT(hi->lo): the control is bit_1(m) and the target is bit_0(m), so the rule swaps m = 2 and m = 3.
- CNOT(lo->hi): the rule swaps m = 1 and m = 3.
- CZ: the rule negates m = 3.
- CRz and CRx: the rule acts only inside the control = 1 half, as stated. QED

**Lemma 3.13 (controlled rotations from their generators)** [spec C13, (0.7), (0.8)]. Let Gc = |1><1|_ctl (x) Pa_tgt with Pa in {Px, Pz}. Then

    R_{Gc}(theta) = |0><0|_ctl (x) I + |1><1|_ctl (x) R_{Pa}(theta).

For Pa = Pz this is CRz(theta), and for Pa = Px it is CRx(theta). Gc has spectrum {0, +1, -1}, and Gc^2 = |1><1|_ctl (x) I != I.

*Proof.* Gc = 0 (+) Pa in the decomposition by control value, and the exponential acts block by block. Compare with Lemma 3.12. QED

**Remark 3.14 (relabelled layouts)** [spec (3.1), Sec. 3.1-3.2, Sec. 2.4; limits HO.1, HO.2]. Let pi be a bijection of [0, N) that permutes bit positions. Then the operator of Lemma 3.10 or 3.11, conjugated by the permutation matrix of pi, is again a butterfly (or quad update) on the image bits. Every storage layout in the engineering documents is of this form: splits of the index into a high part and a low part, pairs of amplitudes, several sentences per block. The proofs above therefore cover every such layout.

Two further cases:

- A K-wire gate with local index m = sum_r bit_{w[r]}(i) 2^r is the block decomposition of Lemma 3.6 with K inserted bits. Adding a control predicate (i & ctrl_mask) = ctrl_val gives Pr_ctl (x) B + (I - Pr_ctl) (x) I, where Pr_ctl is the projector onto the indices that satisfy the predicate.
- Mixed-radix registers (Lemma 4.2, forward) need the mixed-radix analogue of Lemma 3.6, which we now prove. Let dr[0..m-1] be radices with strides stride[j] = prod_{j' < j} dr[j'], fix a position j and a digit a < dr[j], and put

      ins_j(r, a) := (r mod stride[j]) + a stride[j] + (r div stride[j]) stride[j+1]      for 0 <= r < prod_{j' != j} dr[j'].

  This is a strictly increasing bijection onto {i : digit_j(i) = a}, and the digits of ins_j(r, a) are those of r (in the radices dr with position j removed) with a inserted at position j. *Proof.* Write r = r_hi stride[j] + r_lo with r_lo < stride[j]. The three terms of ins_j occupy the digit positions < j, = j and > j respectively; the expansion i = sum_j v_j stride[j] with 0 <= v_j < dr[j] is unique, because sum_{j' < j} v_{j'} stride[j'] < stride[j], and this gives the digit statement and hence bijectivity onto the stated set. For r < r', either r_hi < r'_hi, and then the images differ by at least stride[j+1] - (stride[j] - 1) > 0, or r_hi = r'_hi and r_lo < r'_lo. QED

**Lemma 3.15 (flip masks versus CNOT)** [spec Sec. 2.4, (0.5)].

- Px_k is the permutation i -> i xor 2^k of the index set. A product of such gates is i -> i xor mk for a mask mk. The operators are related by psi_true[i] = psi_stored[i xor mk]. A later gate U_k with bit_k(mk) = 1 acts on the stored vector as Px U Px, since Px is its own inverse.
- CNOT(ctl -> tgt) is the permutation i -> i xor (bit_ctl(i) 2^tgt). This map is linear over GF(2) but is not a translation, so it cannot be absorbed into a mask.
- For W = 3, lo = 0, hi = 1: CNOT(2 -> 0) maps the set {i : bit_0 = bit_1 = 0} = {0, 4} to {0, 5}, which is not a translate of {0, 4}. If {ctl, tgt} is disjoint from {lo, hi}, the set is mapped to itself.

*Proof.* The permutation descriptions follow from Lemma 3.12, and Px_k = i Rx(pi) by Lemma 3.8. The translations i -> i xor mk form a group. i -> i xor (bit_ctl(i) 2^tgt) fixes 0 but moves 2^ctl, so it is not a translation. The two set claims can be checked by direct evaluation. QED

**Lemma 3.16 (2 pi versus 4 pi periodicity)** [spec C5, Sec. 5.6, T6].

- (i) If G^2 = I, then R_G(theta + 2 pi) = -R_G(theta) and R_G(theta + 4 pi) = R_G(theta).
- (ii) For a controlled rotation, CR_{Pa}(theta + 2 pi) = Pz_ctl CR_{Pa}(theta) exactly. This is not a scalar multiple, so CR_{Pa} is 4 pi-periodic but not 2 pi-periodic.
- (iii) Consider the computational-basis Born probabilities of a word state U|0>. Shifting an uncontrolled angle by 2 pi never changes them. Shifting a controlled angle by 2 pi changes them only if some non-diagonal gate acts on the control wire after the shifted gate. Examples:
  - ansatz 0 with L = 1 and Q = 2 gives probabilities (1/4, 1/4, 1/4, 1/4) at theta = 0 and at theta = 2 pi;
  - ansatz 1 with Q = 2 gives (1,0,0,0) at theta = 0 and (0,1,0,0) at theta = 2 pi.
- (iv) diag(1,1,1,e^{i theta}) = e^{i theta/4} Rz_ctl(theta/2) CRz(theta)(ctl -> tgt).

*Proof.* (i) By Lemma 3.8, co and si change sign under theta -> theta + 2 pi. (ii) By Lemma 3.13, the control = 1 block acquires the factor -1 while the control = 0 block does not; that is exactly Pz_ctl. (iii) A global sign, or a diagonal +-1 matrix, leaves every abs(psi[i])^2 unchanged. Diagonal gates commute with Pz_ctl. So Pz_ctl can be moved to the end, where it does not change the probabilities, unless a non-diagonal gate on the control wire stands in the way. For ansatz 1 with Q = 2 at theta = 2 pi, the state is (H (x) H) Pz_0 (H (x) H)|00> = Px_0|00> = e_1. (iv) Multiply the diagonals entrywise: e^{i theta/4} (e^{-i theta/4}, e^{-i theta/4}, e^{i theta/4} e^{-i theta/2}, e^{i theta/4} e^{i theta/2}) = (1,1,1,e^{i theta}). QED

**Definition 3.17 (ansatze)** [spec C10, Sec. 5.5]. A word on Q qubits with parameters theta_w is the state U_w(theta_w)|0...0>, where U_w is one of the following gate lists.

- Q = 1, all ansatze except 4: Rx(t0), Rz(t1), Rx(t2), so P_w = 3.
- Ansatz 4 (Ry-only): at Q = 1, Ry(t0); for Q >= 2, Ry on each qubit followed by CNOT(0->1), CNOT(1->2), ..., CNOT(Q-2 -> Q-1). P_w = Q.
- Ansatz 0 (IQP), for Q >= 2, layer l = 0..L-1: H on every qubit, then CRz(theta_w[l(Q-1) + j])(j -> j+1) for j = 0..Q-2. P_w = L(Q-1).
- Ansatz 1: ansatz 0 followed by a closing H layer.
- Ansatz 3 (Sim15), for Q >= 2, per layer: Ry on all qubits, CNOT(j -> j-1 mod Q) for all j, Ry on all qubits, CNOT(j -> j+1 mod Q) for all j. P_w = 2QL.
- Ansatz 2 (Sim14): as Sim15 but with CRx(angle) in place of each CNOT. P_w = 4QL.

The ring directions of Sim14 and Sim15 are UNVERIFIED against external libraries; any fixed choice defines a valid ansatz.

**Lemma 3.18 (unitarity, transposes, conjugates, realness)** [spec C11, Sec. 1.4, T3; limits NU.5].

- (i) Every gate of Definitions 3.7-3.17 is unitary. So a gate list defines a unitary U = g_m ... g_1.
- (ii) U^T = g_1^T ... g_m^T and conj(U) = conj(g_m) ... conj(g_1).
- (iii) Rx, Rz, H, CNOT, CZ, CRz and CRx are symmetric, and Ry(theta)^T = Ry(-theta).
- (iv) Under conjugation, Rx, Rz, CRz and CRx map to the same gate at -theta. Ry, H, CNOT and CZ are real.
- (v) For Q >= 2, Sim15 and Ry-only words are real. Sim15 unitaries (Q >= 2) and Ry-only unitaries with Q = 1 or Q >= 3 lie in SO(2^Q). The Ry-only unitary at Q = 2 has determinant -1, so it lies in O(4) \ SO(4). At Q = 1, Sim15 words use Rx Rz Rx and are complex.

*Proof.* (i) R_G^dag = R_G^{-1}. H is Hermitian with H^2 = I. The controlled gates are block diagonal with unitary blocks.

(ii) Use (AB)^T = B^T A^T and conj(AB) = conj(A) conj(B).

(iii) H is real symmetric. Rx is complex symmetric: both off-diagonal entries are -i si. Rz, CRz and CZ are diagonal. CNOT is a permutation matrix of an involution, so it is symmetric. CRx is block diagonal with the symmetric block Rx. Ry has off-diagonal entries -si and si.

(iv) Conjugation flips the sign of i.

(v) Ry and CNOT are real, and |0> is real. det Ry = co^2 + si^2 = 1. On W qubits, CNOT swaps 2^{W-2} disjoint pairs, so its determinant is (-1)^{2^{W-2}}. That is +1 for W >= 3 and -1 for W = 2. At Q = 2, a Sim15 layer contains four CNOTs, so its determinant is +1. The Ry-only list at Q = 2 contains exactly one CNOT, so its determinant is -1. A numerical check at (t0, t1) = (0.7, -1.3) gives det = -1.000000. QED

**Remark 3.19 (dimension counts; see Chapter 7)** [addendum (12.1), (12.2), (14.4)]. dim SO(4) = 6 and dim SU(4) = 15. So a Sim15 gate on two qubits cannot cover SU(4). In particular it cannot implement:

- CZ, because det CZ = -1 while Sim15 unitaries have determinant 1 (Lemma 3.18(v));
- diag(1,1,1,e^{i phi}) for phi not in {0, pi}, because that matrix is not real.

Which layer count makes Sim15 generate SO(d^2), or Sim14 generate SU(d^2), is UNVERIFIED (Conjecture 7.13, forward).

**Lemma 3.20 (gate identities)** [spec (5.5), T3]. In operator order:

- (a) CRz(theta)(ctl->tgt) = Rz_tgt(theta/2) CNOT(ctl->tgt) Rz_tgt(-theta/2) CNOT(ctl->tgt);
- (b) CRx(theta)(ctl->tgt) = Rx_tgt(theta/2) CZ Rx_tgt(-theta/2) CZ;
- (c) Rx_tgt(theta/2) CNOT Rx_tgt(-theta/2) CNOT = I. So the CNOT sandwich does not produce CRx.

*Proof.* Decompose along the control. On the control = 0 block, CNOT and CZ act as I, and each identity reduces to R(theta/2) R(-theta/2) = I. On the control = 1 block, CNOT acts as Px_tgt and CZ acts as Pz_tgt. Now:

- Px Rz(a) Px = Rz(-a), since Px anticommutes with Pz. This gives Rz(theta) in (a).
- Pz Rx(a) Pz = Rx(-a). This gives Rx(theta) in (b).
- Px commutes with Rx. This gives I in (c). QED

**Lemma 3.21 (the Q = 1 noun)** [spec Sec. 8, C10, T4]. Let c_j = cos(t_j/2) and s_j = sin(t_j/2). Then Rx(t2) Rz(t1) Rx(t0)|0> = (nv[0], nv[1]), where

    nv[0] = c0 c2 e^{-i t1/2} - s0 s2 e^{+i t1/2},     nv[1] = -i (c0 s2 e^{-i t1/2} + s0 c2 e^{+i t1/2}).

*Proof.* Rx(t0)|0> = (c0, -i s0). Rz(t1) multiplies this by (e^{-i t1/2}, e^{i t1/2}). Finally Rx(t2) maps (alpha, beta) to (c2 alpha - i s2 beta, -i s2 alpha + c2 beta). QED

**Theorem 3.22 (IQP fixed-modulus lemma)** [spec C10, Sec. 8, T4; addendum (12.4); review 3 item 15]. Take ansatz 0 with L = 1 on Q >= 2 qubits, and write b_j = bit_j(k). Then

    V[k] = 2^{-Q/2} exp( i sum_{j=0}^{Q-2} b_j (2 b_{j+1} - 1) theta_j / 2 ),

so every amplitude has modulus 2^{-Q/2}. For Q = 3 at (t0, t1) = (0.8, -0.6), the phases for k = 0..7 are 0, -0.4, 0.3, 0.7, 0, -0.4, -0.3, 0.1.

*Proof.* H^{(x)Q}|0...0> = 2^{-Q/2} sum_k |k>. Each CRz(theta_j)(j -> j+1) is diagonal (Lemma 3.12). It multiplies |k> by exp(i b_j (2 b_{j+1} - 1) theta_j/2). The diagonal factors commute and their phases add. The numerical case is direct evaluation. QED

**Remark 3.23 (angle generation of arbitrary vectors)** [addendum Sec. 12.5, l.819-821; limits l.1139]. Let Ry_{0,q}(t) act on coordinates 0 and q by out[0] = co in[0] - si in[q] and out[q] = si in[0] + co in[q]. Apply Ry_{0,1}(t_1), Ry_{0,2}(t_2), ..., Ry_{0,d-1}(t_{d-1}) in that order to e_0, and then the phases P_q(ph_q): coordinate q -> e^{i ph_q} coordinate q. The result is

    v[0] = prod_{q=1}^{d-1} cos(t_q/2),       v[q] = e^{i ph_q} sin(t_q/2) prod_{q' < q} cos(t_{q'}/2)   (q >= 1).

Proof by induction on q: before Ry_{0,q} is applied, coordinate q is 0, and coordinate 0 equals prod_{q'<q} cos(t_{q'}/2). Every unit vector with v[0] >= 0 arises this way; this is the standard hyperspherical parametrisation, with t_q in [0, 2 pi]. Multiplying by a global phase then reaches every vector. That products of d(d-1)/2 Givens-phase pairs and d output phases parametrise U(d) is Reck et al. (1994) and Clements et al. (2016). It is cited, with details UNVERIFIED.

## 3.3 Postselection, survival probability and readout

**Definition 3.24 (postselection and compaction)** [spec (2.5), (2.5'), (2.6), Sec. 4.5, Sec. 5.1, (5.1), (1.6)]. Let K be a set of qubits, M_post = OR_{k in K} 2^k and Surv = {i : (i & M_post) = 0}. Let Pi_K be the orthogonal projector onto span{e_i : i in Surv}; when K is fixed we write Pi.

Compaction removes the projected qubits. For one qubit k, psi'[j] = psi[i0(j,k)]. For two qubits lo < hi, psi'[j] = psi[i00(j)]. For a general K, apply the one-qubit rule repeatedly (Lemma 3.25(c), forward).

For a unit vector psi, the survival probability is p_surv = ||Pi_K psi||^2. Readout of class c reads N_c = abs(psi[b(c)])^2, where b(c) = ins(0, f_S, q_s, c) = c << f_S. Put D = sum_c N_c and p_c = N_c / D.

*Standing hypothesis (H-readout)*: W = |K| + q_s, and the qubits outside K form the survivor field. Then Surv = {b(c) : c < 2^{q_s}} [spec Sec. 5.1].

**Lemma 3.25 (compaction; the denominator)** [spec (2.5), (2.5'), (2.2)-(2.4)].

- (a) Compaction restricted to range(Pi_K) is a unitary onto C^{2^{W - |K|}}, so p_surv = ||psi'||^2. In general D <= p_surv, with equality under (H-readout).
- (b) The source index of compaction is at least the target index: i0(j,k) >= j and i00(j) >= j.
- (c) Removing the qubits of K one at a time in descending order leaves the numbers of the qubits still to be removed unchanged.
- (d) N_c and D can be read from the uncompacted vector at the indices b(c).

*Proof.* (a) By Lemma 3.6, compaction reindexes a basis of range(Pi_K) bijectively. D sums abs(psi[i])^2 over the subset {b(c)} of Surv. (b) is Lemma 3.6(b),(c). (c) Removing bit q shifts down only the bits above q. (d) follows from Lemma 3.6(a). QED

**Lemma 3.26 (the Bell cup in index form)** [spec C8, (1.4), (2.6), Sec. 1.4; errata Sec. B (l.208); review 2.11]. Use the component-wise pairing (C8); for the nested pairing replace k by bitreverse_q(k) on the second field.

- (a) On two qubits a != b, (<0|_a <0|_b) H_a CNOT(a->b) = 2^{-1/2}(<0|_a<0|_b + <1|_a<1|_b). So for a cup on (a, b), the postselected and compacted amplitudes are z_j = 2^{-1/2}(psi[i00(j)] + psi[i11(j)]), where i00 and i11 are defined with {lo, hi} = {a, b}. Dropping the factor 2^{-1/2} multiplies the circuit amplitude by sqrt2 per qubit pair, which recovers sigma. Any global factor cancels in N_c / D.
- (b) Plain projection onto <0|_a<0|_b is a different map: on psi = e_{i11(0)} it gives 0, while the cup gives 2^{-1/2}.
- (c) If cups are realised as Bell effects, the postselected amplitude carries 2^{-q_n/2} per noun cup. For N TV N that is sigma[s] 2^{-q_n} in total, and P_post(Bell) = Z 2^{-2 q_n} (Z/4 at q_n = 1) [spec (1.4), Sec. 8].

*Proof.* (a) Write two-qubit kets as |x_a x_b>. This labelling is not the C1 order. CNOT maps |00>, |01>, |10>, |11> to |00>, |01>, |11>, |10>, and then H_a gives the component 2^{-1/2} along |00> exactly for the inputs |00> and |11>. The index statement follows from Lemma 3.6(c),(d).

(b) is direct evaluation.

(c) follows from Lemma 2.12 and Proposition 2.11(i). QED

Under the bent-wire convention (Theorem 2.24), or with the unnormalised cup, D = Z. With normalised Bell effects, D = p_surv = Z 2^{-sum_e q_e}.

**Lemma 3.27 (sharp normalisation inequality)** [errata E-11; spec (4.7), (4.9)]. For non-zero x, y in a Hilbert space,

    || x/||x|| - y/||y|| || <= 2 ||x - y|| / (||x|| + ||y||),

with equality iff ||x|| = ||y|| or y is a negative multiple of x. The bound 2||x - y|| / ||x|| used in spec (4.7) is looser by the factor (||x|| + ||y||)/||x||, which is about 2 when ||y|| is about ||x||.

*Proof.* Put al = ||x||, be = ||y||, dd = ||x - y|| and tt = abs(al - be). Then ||x/al - y/be||^2 = (dd^2 - tt^2)/(al be), because dd^2 = al^2 + be^2 - 2 Re<x,y>.

The claimed inequality (dd^2 - tt^2)/(al be) <= 4 dd^2/(al + be)^2 is, after clearing denominators, equivalent to dd^2 (al - be)^2 <= (al - be)^2 (al + be)^2. This holds because dd <= al + be. Equality holds iff al = be or dd = al + be (antiparallel vectors). QED

**Lemma 3.28 (pure-state trace distance)** [errata E-11; review 2.9]. For unit vectors a and b,

    max over orthogonal projectors Pr of abs(<a|Pr|a> - <b|Pr|b>) = sqrt(1 - abs(<a|b>)^2) <= ||a - b||.

*Proof.* Delta := |a><a| - |b><b| is Hermitian, traceless and supported on span{a, b}. In an orthonormal basis {a, b_perp} with b = al a + be b_perp, it is Delta = [[abs(be)^2, -al conj(be)], [-conj(al) be, -abs(be)^2]]. So det Delta = -abs(be)^2, and the eigenvalues of Delta are +-abs(be) = +-sqrt(1 - abs(<a|b>)^2). The quantity Tr(Pr Delta) is maximised by the projector onto the positive eigenvector.

For the last inequality, let tt = abs(<a|b>). Then ||a - b||^2 = 2 - 2 Re<a|b> >= 2(1 - tt) >= (1 - tt)(1 + tt). QED

The postselection-amplification theorem built on Lemmas 3.27-3.28 is Theorem 6.27 (forward).

## 3.4 The expected normaliser

**Definition 3.29 (sentence contraction)** [spec (1.5), C7, l.285; review 2.1]. For nouns n1, n2 in C^d and a verb state phi_V in C^{d_V} with V[a,s,b] = phi_V[a + d s + d d_s b], put

    sigma[s] = sum_{a,b} n1[a] V[a,s,b] n2[b],      Z = sum_s abs(sigma[s])^2,      p[k] = abs(sigma[k])^2 / Z,

with no conjugation of the nouns [spec (1.6)].

**Lemma 3.30 (sigma is a projection coordinate vector)** [errata E-1; spec (4.8); review 2.1]. Define uv_s in C^{d_V} by uv_s[a + d s' + d d_s b] = conj(n1[a]) delta_{s s'} conj(n2[b]). If ||n1|| = ||n2|| = 1, then:

- the vectors uv_s are orthonormal;
- sigma[s] = <uv_s|phi_V>;
- Z <= ||phi_V||^2, with equality iff phi_V lies in span{uv_s}.

*Proof.* <uv_s|uv_{s'}> = sum_{a,b} abs(n1[a])^2 abs(n2[b])^2 delta_{ss'} = delta_{ss'}, and <uv_s|phi_V> = sigma[s]. The inequality is Bessel's inequality. QED

**Lemma 3.31 (Haar second moment).** If phi ~ Haar(n), then E[phi_i conj(phi_j)] = delta_{ij}/n. Equivalently, E[phi phi^dag] = I/n.

*Proof.* Haar(n) is invariant under every unitary. Take the diagonal unitary with e^{i alpha} in position i. It multiplies E[phi_i conj(phi_j)], for j != i, by e^{i alpha}, for every alpha, so that expectation is 0. Invariance under permutations makes all E abs(phi_i)^2 equal, and they sum to 1. QED

**Theorem 3.32 (law of Z for a Haar-random verb)** [errata E-1, replacing spec l.218; spec l.285, l.858, l.970, T6 (l.1272); addendum l.705-712, l.786-788; review 2.1]. Let q_n >= 1. Fix unit nouns n1 and n2, deterministic or independent of the verb, and let phi_V ~ Haar(d_V). Then

    Z ~ Beta(d_s, d_V - d_s),     E[Z] = d_s / d_V = 1/d^2 = 2^{-2 q_n},     Var Z = d_s (d_V - d_s) / (d_V^2 (d_V + 1)).

Numerically:

- E[Z] = 1/4, 1/16 and 1/256 at q_n = 1, 2 and 4;
- sd(Z) = 0.1443, 0.0421 and 0.00195 at (q_n, q_s) = (1,1), (2,1) and (4,2).

The large-d_V form sd ~ sqrt(d_s)/d_V exceeds the true sd by the factor sqrt((d_V + 1)/(d_V - d_s)), which is 1.2247 at (1,1): 22.5 % high.

*Proof.* By Lemma 3.30, Z = ||Pr phi_V||^2, where Pr is the orthogonal projector onto the fixed d_s-dimensional subspace span{uv_s}. Map that subspace to span{e_0, ..., e_{d_s - 1}} by a unitary. By invariance, Z then has the law of sum_{i < d_s} abs(phi_i)^2.

Write phi = g/||g||, where g has independent standard complex Gaussian entries (real and imaginary parts independent N(0, 1/2)). The law of g/||g|| is unitarily invariant and lives on the sphere, so it is Haar. The abs(g_i)^2 are independent Exp(1) = Gamma(1,1). So X = sum_{i<d_s} abs(g_i)^2 ~ Gamma(d_s, 1) and Y = sum_{i >= d_s} abs(g_i)^2 ~ Gamma(d_V - d_s, 1) are independent. This needs d_V > d_s, which holds since q_n >= 1.

By the Gamma-Beta relation (Appendix C), X/(X+Y) ~ Beta(d_s, d_V - d_s). The mean a/(a+b) and variance ab/((a+b)^2(a+b+1)) of Beta(a,b) give the formulas.

The mean also follows from Lemma 3.31: E Z = sum_s <uv_s|(I/d_V)|uv_s> = d_s/d_V. At (1,1), Var = 12/(64 x 9) and sd = 0.1443. QED

**Corollary 3.33 (general forms)** [limits l.518, l.832, l.1218; errata E-1].

- (a) Fix all occurrences except one, w, and let phi_w ~ Haar(2^{Q_w}). Then E[Z] = ||A_w||_F^2 / 2^{Q_w}, where sigma = A_w phi_w (Theorem 2.20: with the other occurrences fixed, sigma is a fixed linear map A_w of phi_w). For N TV N, ||A_V||_F^2 = d_s.
- (b) With qudit radices d_n and d_s, E[Z] = 1/d_n^2.
- (c) Let every word of a sentence be independently Haar, or let the whole register be Haar. Let Pr be an orthogonal projector of rank rk on the register C^{2^W}. Then E<Psi|Pr|Psi> = rk / 2^W. In particular, n_cup Bell effects on pairs of q-qubit wires, each a rank-1 projector on 2q qubits, give E[P_post] = 2^{-2 q n_cup}. Postselecting n_sw single q-qubit wires onto one state each gives 2^{-q n_sw}.
- (d) For N TV N with a Haar verb, E[P_post] = E[Z] 2^{-2 q_n} = 2^{-4 q_n}. For two sentences with independent Haar verbs, the product of the two acceptance probabilities has mean 2^{-8 q_n}.
- (e) Z = 0 is attained, for example by a verb orthogonal to span{uv_s}. It has probability 0 under the Haar law, since the Beta law has no atom.

*Proof.* (a) E[Z] = E ||A_w phi_w||^2 = Tr(A_w A_w^dag)/2^{Q_w} by Lemma 3.31. For N TV N, A_V has the orthonormal rows uv_s^dag.

(b) Repeat Theorem 3.32 with the given radices.

(c) Only first moments are needed. E[|Psi><Psi|] = (x)_w E[|phi_w><phi_w|] = (x)_w I/2^{Q_w} = I/2^W, by independence and multilinearity, or by Lemma 3.31 for a Haar register. Then take the trace against Pr.

(d) follows from Lemma 2.12 and independence.

(e) follows from the absolute continuity of the Beta law. QED

**Corollary 3.34 (distribution of Z relative to the p_min floors)** [spec Sec. 4.5 rule (3), l.863-864; errata E-1, E-11]. For Beta(2, n), F(x) = 1 - (1-x)^{n+1} - (n+1) x (1-x)^n. For untrained Haar verbs:

- at the original floors of spec (4.7): P(Z < 0.31) = 0.691 at (1,1) (n = 6), and P(Z < 0.05) = 0.463 at (2,1) (n = 30). Errata E-1's "47 %" should read 46 % (exact 0.4634);
- at the floors implied by the tight constant of errata E-11, the thresholds x = 0.020 and x = 0.0033 (these are the floors p_min = (err/tol + err/2)^2 of Corollary 6.28 (forward); here they are used only as numbers): P(Z < 0.020) = 0.008 at (1,1), and P(Z < 0.0033) = 0.005 at (2,1).

So the original floors are not tail events, and the corrected floors are.

*Proof.* Beta(2, n) is the law of the second smallest of n + 1 independent uniforms. So F(x) is the probability that at least two of them are < x. Evaluating F gives the four values. QED

**Remark 3.35 (what is and is not proved about Z)** [errata E-1, E-10; review 2.1; limits L.1-L.2; addendum (13.10'), (11.25), (11.26)].

- (i) Theorem 3.32 is exact for a Haar verb with any unit nouns. Monte Carlo values in errata E-1 are consistent with it, and are not used in any proof.
- (ii) For ansatz words (IQP, L = 1, uniform angles), errata E-1 reports that the mean matches the Haar value at (1,1) but equals 0.041 at (2,1), with values of Z as small as 1e-12. A structural explanation via Theorem 3.22 is UNVERIFIED.
- (iii) That training raises Z above 2^{-2 q_n} is a Conjecture.
- (iv) The parse superposition sum_p wt_p sigma_p and the parse mixture sum_p wt_p |sigma_p><sigma_p| / Z_p are exact linear constructions.
- (v) For mixed words rho_w = sum_i pr_i |w_i><w_i|, multilinearity gives N_c = sum_i (prod_j pr_{i_j}) abs(sigma_c^{(i)})^2 and D = sum_i (prod_j pr_{i_j}) Z^{(i)}. That a Monte Carlo estimate of their ratio from n_MC samples has bias O(1/n_MC) is the delta method; it requires D bounded away from 0 and finite variances, and is UNVERIFIED here.

## 3.5 Fidelity, reduced states, entropy and the Loewner order

**Definition 3.36 (pure-state fidelity; Gram matrix)** [spec Sec. 1.5, Sec. 9 item 7; limits G.1-G.3]. For normalised a and b, Fid(a, b) = abs(<a|b>)^2. For unnormalised sigma_1 and sigma_2, Fid = abs(<sigma_1|sigma_2>)^2 / (Z_1 Z_2).

For a batch psi_a := sigma_a / sqrt(Z_a), the Gram matrix is G_ab := <psi_a|psi_b> = sum_k conj(psi_a[k]) psi_b[k]. If Psi_b is the matrix with rows psi_a, then G = conj(Psi_b) Psi_b^T. This corrects limits (G.2), whose matrix form Psi_b Psi_b^dag equals conj(G) = G^T; F_ab = abs(G_ab)^2 is unaffected. The pure-state trace distance is sqrt(1 - F_ab) (Lemma 3.42(a), forward), and arg G_ab depends on the global phases of the two states.

**Definition 3.37 (density operators; partial trace)** [spec Sec. 1.5; addendum (13.10), (14.19); limits T.1, SF.1]. A density operator on C^n is Hermitian, positive semidefinite and of trace 1. It has the matrix rho[a][a'], and vec(rho)[a + n a'] = rho[a][a'].

Write C^{2^W} = H_Af (x) H_R, where Af is a field of q bits starting at f. The partial trace over R is

    rho_Af[a][a'] = sum_{r < 2^{W-q}} Psi[ins(r,f,q,a)] conj(Psi[ins(r,f,q,a')]),

that is, rho_Af = Pm Pm^dag with Pm[a][r] = Psi[ins(r,f,q,a)].

**Lemma 3.38 (properties of the partial trace)** [limits T.1, HO.7, HO.8; addendum (14.18)].

- (a) rho_Af is PSD and Tr rho_Af = ||Psi||^2. On mixed inputs, Tr_R is linear, CP and trace preserving.
- (b) (Sum over slices.) Let Psi_v be the slice of Psi at value v of a field Bf disjoint from Af. The field start of Af is relocated when f_B < f_A. Then rho_Af(Psi) = sum_v rho_Af(Psi_v).
- (c) Tr_Bf[(I_Bf (x) G) rho (I_Bf (x) G)^dag] = G (Tr_Bf rho) G^dag.
- (d) The map rho -> Ao rho Bo^dag acts on vec(rho) as conj(Bo) (x)_Kr Ao.
- (e) A convex combination of two gate placements, rho' = h G_1 rho G_1^dag + (1-h) G_2 rho G_2^dag, is CPTP of Kraus rank <= 2. In general it is not a unitary conjugation.

*Proof.* (a) Pm Pm^dag is PSD, and its trace is sum abs(Pm[a][r])^2 = ||Psi||^2 by Lemma 3.6(a). Tr_R is CP because it has the Kraus operators <r|_R.

(b) Split the sum over r according to the digits of r in field Bf.

(c) In the Gram form, G multiplies Pm on the left, and r is untouched.

(d) vec(Ao rho Bo^dag)[a + n a'] = sum_{x,y} Ao[a][x] rho[x][y] conj(Bo[a'][y]), which is Definition 3.2.

(e) The Kraus operators sqrt(h) G_1 and sqrt(1-h) G_2 satisfy the completeness relation. A unitary conjugation maps pure states to pure states, whereas rho' is mixed whenever G_1 rho G_1^dag != G_2 rho G_2^dag for a pure rho. QED

**Theorem 3.39 (Schmidt decomposition; entropy; mutual information)** [addendum (14.15), (14.16); limits SF.6, T-W4]. Let Psi be a unit vector in H_L (x) H_R, and let M_Psi[l][r] be its coefficient matrix.

- (a) There are orthonormal families {u_k} and {v_k} and numbers xi_0 >= xi_1 >= ... > 0 with Psi = sum_{k < chi} xi_k u_k (x) conj(v_k), where chi = rank M_Psi = sr_cut(Psi) <= min(dim H_L, dim H_R). The reduced states rho_L and rho_R have the same non-zero spectrum {xi_k^2}.
- (b) S(rho_L) = -sum_k xi_k^2 log2 xi_k^2 <= log2 chi, with equality iff all xi_k^2 = 1/chi.
- (c) I(L:R) := S(rho_L) + S(rho_R) - S(rho_LR) = 2 S(rho_L) <= 2 log2 chi.
- (d) For subsystems a of L and b of R, I(a:b) <= I(L:R).
- (e) If Psi = Op Phi, where Phi is a product state across the cut and Op = sum_{j < rr} Ao_j (x) Bo_j has operator Schmidt rank rr across the cut, then chi <= rr. In particular one CRz across the cut, applied to a product state, gives chi <= 2.

*Proof.* (a) Take the singular value decomposition M_Psi = Um diag(xi) Vm^dag. Then rho_L = M_Psi M_Psi^dag and rho_R = (M_Psi^dag M_Psi)^T share the non-zero values xi_k^2.

(b) The Shannon entropy of a distribution on chi points is at most log2 chi (Gibbs' inequality), with equality iff the distribution is uniform.

(c) S(rho_LR) = 0 for a pure state, and S(rho_R) = S(rho_L) by (a).

(d) This is monotonicity of mutual information under partial trace. It is equivalent to strong subadditivity (Lieb and Ruskai 1973; Appendix C), which is cited.

(e) Psi = sum_j (Ao_j phi_L) (x) (Bo_j phi_R) has at most rr terms. CRz = |0><0| (x) I + |1><1| (x) Rz has operator Schmidt rank 2 across the control|target cut. QED

**Definition 3.40 (spectral functionals)** [limits SF.4, SF.5; addendum Sec. 14.6]. Let rho have eigenvalues ev_l. Define:

- the entropy S(rho) = -sum_l ev_l ln ev_l, omitting terms with ev_l = 0;
- the Renyi entropy S_alpha(rho) = ln(sum_l ev_l^alpha)/(1 - alpha);
- the purity Tr rho^2;
- the relative entropy S(A||B) = Tr A(ln A - ln B), which is +infinity unless supp A is contained in supp B;
- the trace distance T_dist(rho, tau) = (1/2) sum_l abs(ev_l(rho - tau));
- the fidelity F_root(rho, tau) = Tr sqrt(sqrt(rho) tau sqrt(rho)), and F_sq = F_root^2;
- the Bures angle arccos F_root, and the squared Bures distance 2(1 - F_root).

For a function g, g(rho) = Vm diag(g(ev)) Vm^dag.

**Theorem 3.41 (Uhlmann fidelity for qubits)** [spec Sec. 1.5; addendum Sec. 14.6; limits SF.5]. For 2 x 2 density operators, F_sq(rho, tau) = Tr(rho tau) + 2 sqrt(det rho det tau).

*Proof.* Mq := sqrt(rho) tau sqrt(rho) is PSD with eigenvalues mu_1 and mu_2. Then (Tr sqrt(Mq))^2 = mu_1 + mu_2 + 2 sqrt(mu_1 mu_2) = Tr Mq + 2 sqrt(det Mq). By cyclicity Tr Mq = Tr(rho tau), and det Mq = det rho det tau. QED

**Lemma 3.42 (pure states; Fuchs-van de Graaf)** [limits G.3, T-SF2].

- (a) For pure rho = |a><a| and tau = |b><b|: F_sq = abs(<a|b>)^2 and T_dist = sqrt(1 - F_sq).
- (b) In general, 1 - F_root <= T_dist <= sqrt(1 - F_sq) (Fuchs and van de Graaf 1999; cited).
- (c) For 2 x 2 density operators, Delta = rho - tau has eigenvalues +-ev, and T_dist = ev.
- (d) For 2 x 2 density operators with Bloch vectors rv and tv of equal length, F_sq = 1 - abs(rv - tv)^2/4 and T_dist = abs(rv - tv)/2. So the upper bound in (b) is attained.

*Proof.* (a) This is the eigenvalue computation in Lemma 3.28.

(c) Delta has trace zero.

(d) By Theorem 3.41, F_sq = (1 + rv.tv)/2 + 2 sqrt((1 - abs(rv)^2)(1 - abs(tv)^2))/4. With abs(rv) = abs(tv) this equals 1 - (abs(rv)^2 - rv.tv)/2 = 1 - abs(rv - tv)^2/4. Also Delta = (rv - tv).(Px, Py, Pz)/2 has eigenvalues +-abs(rv - tv)/2. QED

**Lemma 3.43 (entropy examples)** [limits T-SF3]. For A = |0><0| and B = I/2, in natural units: S(A) = 0, S(B) = ln 2, T_dist(A, B) = 1/2, F_root = 1/sqrt2, S(A||B) = ln 2 and S(B||A) = +infinity.

*Proof.* The spectra are {1, 0} and {1/2, 1/2}. A - B = diag(1/2, -1/2). sqrt(A) B sqrt(A) = A/2. supp B is not contained in supp A. QED

**Definition 3.44 (Loewner order; graded hyponymy)** [limits SF.4, SF.5; Bankova et al. arXiv:1601.04908; Balkir et al. arXiv:1506.06534]. For Hermitian matrices, A <= B iff B - A is PSD. For PSD A != 0 and PSD B, A is a k-hyponym of B iff k A <= B. Put k_max(A <= B) := sup{k >= 0 : k A <= B}.

**Proposition 3.45 (computing k_max)** [limits SF.5]. Let A != 0 and B be PSD.

- (a) If supp A is not contained in supp B, then k_max = 0. Otherwise k_max = 1/ev_max(B^{+1/2} A B^{+1/2}), computed on supp B, and k_max > 0.
- (b) If d = 2 and B > 0, then k_max = 2/(tt + sqrt(tt^2 - 4 det A / det B)), where tt = (Tr A Tr B - Tr(AB))/det B.
- (c) If A and B are 2 x 2 with the same spectrum {pp, 1 - pp}, 0 < pp < 1, then k_max(A <= B) = k_max(B <= A) and S(A||B) = S(B||A).
- (d) (c) fails for d = 3. Take A = diag(0.5, 0.3, 0.2) and B = diag(0.2, 0.5, 0.3). Then S(A||B) = 0.2238 and S(B||A) = 0.1938 (natural logarithms), while k_max(A <= B) = 0.4 and k_max(B <= A) = 0.6.

*Proof.* (a) If some unit vector v has <v|B|v> = 0 but <v|A|v> > 0, then k A <= B fails for every k > 0. Otherwise, on supp B, k A <= B iff k B^{-1/2} A B^{-1/2} <= I.

(b) The non-zero eigenvalues of B^{-1/2} A B^{-1/2} are those of B^{-1} A. For 2 x 2 matrices, B^{-1} = (Tr B I - B)/det B. The larger root of x^2 - tt x + det A/det B is the one required.

(c) Equal spectra give det(B^{-1} A) = 1, so the eigenvalues of B^{-1} A are ev and 1/ev, and those of A^{-1} B are the same pair. Write A = sum_i pr_i |a_i><a_i| and B = sum_j pr_j |b_j><b_j|. The overlap matrix cc_ij = abs(<a_i|b_j>)^2 is 2 x 2 and doubly stochastic, hence symmetric. Then S(A||B) = -h(pp) - sum_{ij} pr_i cc_ij ln pr_j, where h(pp) is the binary entropy of (pp, 1 - pp), and exchanging A and B transposes cc, which does not change it.

(d) For commuting operators, S(A||B) = sum_i a_i ln(a_i/b_i) and k_max(A <= B) = min_i b_i/a_i. Evaluate. QED

**Lemma 3.46 (Z-basis correlators by the Walsh-Hadamard transform)** [limits T.3, T.4; spec Sec. 2.1]. Let pr[x] = abs(Psi[x])^2.

- (a) zc[Sb] := sum_x (-1)^{popcount(x & Sb)} pr[x] is the expectation of prod_{k in Sb} Pz_k. The vector zc is obtained from pr by applying the butterfly of Lemma 3.10 with [[1,1],[1,-1]] on every qubit.
- (b) For a subset Af of qubits and a bit pattern bb on Af, the marginal is pr_Af(bb) = 2^{-|Af|} sum_{Tb subset Af} (-1)^{popcount(bb & Tb)} zc[Tb].
- (c) If Psi = Dg Phi with Phi a product state and Dg a diagonal unitary, then pr is a product distribution, even though Psi may be entangled.

*Proof.* (a) prod Pz_k is diagonal with entries (-1)^{popcount(x & Sb)}, and Lemma 3.3 identifies the tensor product of the [[1,1],[1,-1]] factors.

(b) The characters of Z_2^{|Af|} are orthogonal.

(c) Diagonal unitaries preserve the moduli abs(Psi[x]). QED

**Remark 3.47 (completely positive word updates)** [limits NU.1-NU.4]. Let omega = Uo diag(r) Uo^dag with r >= 0. The maps below all have the form rho -> Uo (Wgt .* (Uo^dag rho Uo)) Uo^dag, with Wgt as listed:

- fuzz(rho) = sum_i r_i Pr_i rho Pr_i: Wgt[a][a'] = r_a delta_{aa'};
- phaser(rho) = sqrt(omega) rho sqrt(omega): Wgt[a][a'] = sqrt(r_a r_a');
- projection onto supp(omega): Wgt[a][a'] = [r_a > 0][r_a' > 0];
- rho -> omega rho omega: Wgt[a][a'] = r_a r_a'.

Entrywise multiplication by omega (Mult) is the same construction with Uo = I and Wgt = omega.

For every PSD Wgt the map is CP. Writing Wgt = sum_mm wv_mm wv_mm^dag gives the explicit Kraus form Wgt .* X = sum_mm diag(wv_mm) X diag(wv_mm)^dag. The Choi matrix is sum_{aa'} Wgt[a][a'] |a><a'| (x) |a><a'|, which is PSD iff Wgt is. The Kraus rank of fuzz is the number of non-zero r_i. The family Wgt = sqrt(r_a r_a') (1 if a = a', else gam), for gam in [0,1], is a convex combination of the fuzz and phaser weights. For fuzz and phaser the pre-normalisation trace is tr_pre = Tr(omega rho).

The affine map neg(rho) = I - rho at d = 2 equals Tn(rho) := Py rho^T Py on density operators: for rho = (I + x Px + y Py + z Pz)/2, rho^T = (I + x Px - y Py + z Pz)/2, and conjugation by Py flips the signs of the Px and Pz components. The Choi matrix of Tn has a negative eigenvalue, so it is not CP. It has no postselected realisation either: there is no linear CP map Ep with Ep(rho)/Tr Ep(rho) = neg(rho) on an open set of density operators where Tr Ep > 0.

*Proof.* Tn is a linear bijection of Lin(C^2) (it is an involution). Its Choi matrix is (Py (x) I) Fsw (Py (x) I)^dag, where Fsw is the Choi matrix of the transpose, the swap; so it is unitarily similar to the swap and has the eigenvalue -1. Hence Tn is not CP.

Suppose Ep is linear and CP and Ep(rho) = c(rho) Tn(rho) with c(rho) = Tr Ep(rho) > 0 for all rho in an open set O of density operators (Tr Tn(rho) = 1). Put Lm := Tn^{-1} o Ep, a linear map with Lm(rho) = c(rho) rho on O. Take a ball Bb in O (in the affine space of trace-one Hermitian matrices). For rho_1 != rho_2 in Bb, the segment rho_t = (1-t) rho_1 + t rho_2 lies in Bb, and rho_1, rho_2 are linearly independent (distinct with equal trace). Linearity gives Lm(rho_t) = (1-t) c(rho_1) rho_1 + t c(rho_2) rho_2, while Lm(rho_t) = c(rho_t)((1-t) rho_1 + t rho_2). Comparing coefficients at any t in (0,1) gives c(rho_1) = c(rho_t) = c(rho_2). So c is a constant c0 > 0 on Bb. A ball in the trace-one hyperplane spans Herm(2) over R, and Herm(2) spans Lin(C^2) over C, so Lm = c0 id and Ep = c0 Tn. Then Ep is not CP, a contradiction. QED

Attributions of these operations to named papers (including the Buzek-Hillery optimal NOT, (2I - rho)/3) are UNVERIFIED.

**Remark 3.48 (further exact identities).**

- (i) Slot linearity [limits G.5, G.6]. Fixing all words but one, sigma = Ms w_v is linear in the open word w_v, and the score abs(<tv|Ms w_v>)^2 / <w_v|Ms^dag Ms|w_v> is a normalised quadratic form.
- (ii) Retained sentence wire [limits HO.5]. Adjoining a fresh |0>^{q_s} field is Corollary 3.4 with phi = e_0. Restricting to the top field is Lemma 3.6(a) with f = W - q.
- (iii) Early reuse of a finished wire [limits HO.7, HO.8]. By Lemma 3.38(b),(c), tracing a wire early and re-preparing it changes no later reduced state on the other wires.
- (iv) Trajectory sampling [limits HO.10]. Draw v with probability ||Psi_v||^2 and continue with Psi_v/||Psi_v||. Then E[rho_Af^{(traj)}] = sum_v rho_Af(Psi_v) = rho_Af, by Lemma 3.38(b).
- (v) Measure-and-reprepare [addendum (12.19)]. Replacing sigma_1 by sqrt(p_1) is not a linear map on the state.
- (vi) Unitary MPO verb [addendum (12.15)]. Let Ao be the first r columns of a unitary and Co the first r rows of one. Then ||Ao^T n1|| <= 1 and ||Co n2|| <= 1.
- (vii) Quantum switch [limits B3]. For unitaries V1, V2 and a unit vector Psi_0, ||(V2 V1 + V1 V2) Psi_0||^2 = 2(1 + Re<V2 V1 Psi_0|V1 V2 Psi_0>). The construction is trivial when V1 and V2 commute.
- (viii) Readout with sum aggregation and no cups [review 5(c)]. p = abs(sigma)^2 / Z is a normalised quadratic form of the inputs, so phases affect it only through interference between at least two terms.
# Chapter 4. Tensor networks: contraction, cut width, matrix product states and operators

In this chapter (x) is the abstract tensor product with the little-endian index map of Section 0.1. That is, Psi[l + d_L r] = M_Psi[l][r], with the left factor in the low digits. It is not the C2 Kronecker product. Storage statements count the number of complex entries that must exist simultaneously. No cup conjugates anything [spec (1.5), C8].

## 4.1 Tensors, fields and mixed-radix indexing

**Definition 4.1 (tensor with fields).** A field list [F_0, ..., F_{n-1}] has widths w_0, ..., w_{n-1}. Field F_j carries a type position pos(F_j), and its bit offset in the flat index is shift(F_j) = w_0 + ... + w_{j-1}, which is fixed by list order. A tensor with this field list is a map T: prod_j {0..2^{w_j}-1} -> C, stored as T[sum_j v_j 2^{shift(F_j)}]. For a word w the offsets are shift = fo_w[j] (Definition 2.14). #T denotes the number of entries of T.

Examples [spec (1.5), Sec. 1.3; addendum (11.14), (12.9)]:

- V[a,s,b] = phi_V[a + d s + d d_s b];
- IV[a,s] = phi[a + d s];
- ADJ[a,b] = phi[a + d b];
- ADV[b,s] = phi[b + d_s s].

For a list of fields Fs, scatter(x, Fs) is the integer whose bit group at the i-th field of Fs is the i-th consecutive group of bits of x, with all other bits 0.

**Lemma 4.2 (mixed radix)** [addendum (12.16'), T16]. Let dr[0..m-1] be radices, and put stride[j] = prod_{j' < j} dr[j']. The digit map i -> (v_j), v_j = floor(i / stride[j]) mod dr[j], and its inverse i = sum_j v_j stride[j], are mutually inverse bijections between {0, ..., prod_j dr[j] - 1} and prod_j {0, ..., dr[j] - 1}.

*Proof.* For 0 <= v_j < dr[j] we have sum_{j' < j} v_{j'} stride[j'] < stride[j]. So reducing i modulo stride[j+1] and dividing by stride[j] recovers v_j, which proves injectivity. The two sets have the same size, so the map is a bijection. As checks: radices (3,3,3) give (2,1,0) -> 5 and 17 -> (2,2,1); radices (3,2,3) give 13 -> (1,0,2). QED

**Remark 4.3 (scope of radix generality)** [spec Sec. 1.6; limits Sec. 2.7 (7) Test A]. Replacing 2^w by an arbitrary radix, Definition 4.1, Definition 4.4 (forward) and all results of Sections 4.2-4.8 hold verbatim with Lemma 4.2 in place of bit shifts. The exceptions are the qubit-circuit statements, namely Lemma 2.12 and its uses.

## 4.2 The sentence state

**Definition 4.4 (monolithic sentence state)** [spec (1.3), (1.4), (1.6), (1.7)]. Suppose the cups and the survivor field partition the type positions, as they do for a grammatical reduction. Let Psi be as in Theorem 2.20. Then sigma, Z and p are given by (2.4.2) and Definition 3.29. The regularised readout is

    p_K[k] = (abs(sigma[k])^2 + eps_reg) / (sum_{j < n_cl} abs(sigma[j])^2 + n_cl eps_reg),      Loss = -ln p_K[y].

By Proposition 2.18, sigma is the application of the tensor product of all cups and the identity on the survivor field to Psi. It has 2^{q_s} outputs, and each output is a sum over 2^{sum_e q_e} addresses.

**Proposition 4.5 (density-matrix model)** [spec Sec. 1.6; addendum (13.10)]. Let the words be density operators rho_w, and contract every cup on both the row index and the column index:

    Sg[s][s'] = sum rho_{n1}[a][a'] rho_V[(a,s,b)][(a',s',b')] rho_{n2}[b][b'].

For pure words this reduces to Sg = sigma sigma^dag.

*Proof.* For rho_w = phi_w phi_w^dag, the double sum factorises as sigma[s] conj(sigma[s']). QED

## 4.3 Pairwise contraction and the fused sweep

**Lemma 4.6 (contraction is a matrix product)** [spec (2.8)]. Let A have c_a bits, of which the k contracted bits are the LOW bits, and let B have c_b bits, of which the k contracted bits are the HIGH bits. Put f_a = c_a - k and f_b = c_b - k. Then

    out[(i_a << f_b) | i_b] = sum_{c < 2^k} A[(i_a << k) | c] B[(c << f_b) | i_b]

is the row-major product of A_mat (2^{f_a} x 2^k) and B_mat (2^k x 2^{f_b}). It costs 2^{c_a + c_b - k} complex multiply-adds. If the contracted bits sit elsewhere, one bit permutation of each operand brings them into position.

*Proof.* The index (i_a << k) | c is row i_a, column c of A_mat, and (c << f_b) | i_b is row c, column i_b of B_mat. Each of the 2^{f_a + f_b} outputs sums 2^k terms. QED

**Definition 4.7 (fused contraction step)** [spec Sec. 1.3]. Let T be an open tensor with fields FT, and phi an incoming word with fields FW. The cups pair the fields cupT of FT with the fields cupW of FW. The remaining fields are restT and restW, and q_cup is the total width of the cupped fields. The step is

    R[(rW << widthsum(restT)) | rT] = sum_{k < 2^{q_cup}} T[scatter(rT, restT) | scatter(k, cupT)] phi[scatter(rW, restW) | scatter(k, cupW)],

after which T := R and FT := restT ++ restW. A sweep processes the words in sentence order, starting from T = phi_1.

**Theorem 4.8 (the sweep computes sigma).** After a sweep over all n_w words, T = sigma with FT = [F_S]. The full product state Psi is never formed.

*Proof.* We maintain an invariant. After words 1..w have been processed:

- FT is the list of positions in words 1..w that are not yet cupped within {1..w};
- T[addr(v)] is the sum, over the digits of all cups internal to {1..w}, of prod_{w' <= w} phi_{w'}, with the open fields fixed at v.

The invariant holds for w = 1, where there are no internal cups.

For the step, the cups joining word w+1 to {1..w} are exactly the pairs (cupT, cupW). scatter(k, cupT) and scatter(k, cupW) deposit the same bit groups of k into paired fields, so the sum over k is the component-wise pairing (C8). By distributivity of finite sums, multiplying the invariant sum by phi_{w+1} and then summing over k gives the invariant sum for {1..w+1}. When the cup list is empty the step is the tensor product.

At w = n_w every cup is internal and FT = [F_S], so the invariant sum is (2.4.2). QED

**Proposition 4.9 (storage of a step).** A step needs #T + #phi + #R simultaneous entries, where #R = 2^{widthsum(restT) + widthsum(restW)}. It needs one entry fewer per output when the output overwrites an operand, which is only possible under the hypotheses of Lemma 4.10 (forward).

*Proof.* The three arrays are the only objects the step uses, and #R can be read off Definition 4.7. QED

**Lemma 4.10 (in-place slot rule, corrected)** [spec Sec. 1.3 l.258; errata E-13].

- (i) If restT is empty, write the output for rW into the incoming word's slot k = 0, that is, phi[scatter(rW, restW) | scatter(0, cupW)]. Every order that performs this write after the reads of {scatter(rW, restW) | scatter(k, cupW) : k} is correct.
- (ii) If restW is empty, write the output for rT into T's slot T[scatter(rT, restT) | scatter(0, cupT)], under the same read-before-write condition.
- (iii) If both restT and restW are non-empty, the slot of an incoming word depends only on rW. Distinct outputs then collide, and no in-place rule is claimed.

Spec l.258 states the incoming-word rule without the hypothesis of (i), and errata E-13 inherits the omission. Both need the qualifier.

*Proof.* (i) The output at rW depends only on the slot set of rW in phi and on all of T. Since restT is empty, T is a vector over the cupped fields alone, so it is never overwritten. Distinct rW have disjoint slot sets (Lemma 4.2). So a write to slot 0 of rW, performed after its reads, destroys nothing that another output needs.

(ii) Symmetric.

(iii) Counterexample, step 2 of N TV N at (1,1): T = R[s,b] = [[1,2],[3,4]] and phi = n2 = (3,-1). The true result is sigma = (1, 5). Writing both outputs into n2[0] destroys the value sigma[1] still needs and yields (1, -1). QED

**Proposition 4.11 (N TV N storage and cost)** [spec Sec. 1.3, Sec. 2.3, Sec. 7; addendum (12.11); limits L.3]. The sweep is T = n1, then R[s,b] = sum_a n1[a] V[a,s,b], then sigma[s] = sum_b R[s,b] n2[b].

- It costs d^2 d_s + d d_s multiply-adds.
- Its peak storage is 2^Q + 2^{q_n} + 2^{q_n + q_s}, plus 2^{q_n} if n2 is held throughout: 14 (16) at (1,1) and 44 (48) at (2,1).
- With Lemma 4.10, the peak is 2^Q + 2^{q_n}: 10 at (1,1) and 36 at (2,1). Step 1 is case (i), writing R into V's slots with a = 0. Step 2 is case (ii), writing sigma into R's slots with b = 0.

Forming n1 (x) V first needs at least 2^{3 q_n + q_s} + 2^{2 q_n + q_s} entries (24 / 160), and the naive product state needs 2^{4 q_n + q_s} (32 / 512). In general, a step that first forms T (x) phi holds 2^{ob + Q_w} entries, where ob is the number of open bits before the step. The fused step holds at most 2^{max(Q_w, oa)} plus operands, where oa is the number of open bits after it. The ratio of the largest single arrays is 2^{ob + Q_w - max(Q_w, oa)}. For step 2 of N TV N it is 2^{2 q_n}.

*Proof.* The sizes are #n1 = d, #V = d^2 d_s and #R = d d_s, and Proposition 4.9 gives the peaks. For the multiply-add counts apply Lemma 4.6: step 1 has A = V and B = n1 with k = q_n, and step 2 has A = R and B = n2 with k = q_n. The product T (x) phi has 2^{ob + Q_w} entries by definition. QED

**Remark 4.12 (fold sequences and the zero-angle value)** [spec Sec. 1.3, Sec. 8, T5, T6; limits G.5]. The fold sequences are:

- N IV: sigma[s] = sum_a n[a] IV[a,s];
- ADJ N TV N: n'[a] = sum_b ADJ[a,b] n[b], then as N TV N;
- N IV ADV: R[b] = sum_a n[a] IV[a,b], then sigma[s] = sum_b R[b] ADV[b,s].

Zero angles. At zero angles, with the nouns equal to basis vectors e_0 (for example Q = 1 words, which give Rx Rz Rx |0> = |0>) and the verb an ansatz-0 word, V = H^{(x)Q}|0> is uniform with entries 2^{-Q/2}. Then sigma[s] = V[0,s,0] = 2^{-Q/2} and Z = d_s 2^{-Q} = 2^{-2 q_n}. At (1,1) this is sigma = (2^{-3/2}, 2^{-3/2}) and Z = 1/4 [spec T6]. This deterministic value coincides numerically with the Haar mean E[Z] of Theorem 3.32 but is a different statement. The spec's decimal values for "Alice loves Bob" (Z = 0.14026553 and the others) are numerically checked there and are not re-derived here.

The slot matrix of "Alice loves [ ]" is the sweep stopped one word early.

## 4.4 Contraction order on the word graph

**Definition 4.13 (word graph; node operation)** [addendum C16, Sec. 11.7].

The word graph Gw has one node per word and one edge per cup. Parallel cups between the same two words are merged into one edge, whose width is the sum of their widths. The root is the word that owns the survivor. In this section Gw is assumed to be a tree; cycles are first collapsed by Proposition 4.17 (forward).

A node v has children k, each joined by an edge of width q_k, and it outputs a field of width q_out(v). For the root, the output is the survivor field, so q_out(root) := q_s. For a leaf, the reduced vector is its word tensor. The node operation is

    rv_v[o] = sum_{j : out(j) = o} phi_v[j] prod_k rv_k[f_k(j)],

where out(j) and f_k(j) are the digits of j in the output field and in the field toward child k. The node operation is linear in phi_v and in each rv_k.

**Theorem 4.14 (peak recurrence)** [addendum (11.18'), (11.19)]. Evaluate the tree in post-order, holding the results of earlier siblings while later subtrees are evaluated. The peak storage is

    peak(v) = max( max_k [ peak(child_k) + sum_{k' < k} 2^{q_{k'}} ],   2^{Q_v} + sum_k 2^{q_k} + 2^{q_out(v)} ).

Base case: for a leaf v (a non-root node without children), peak(v) := 2^{Q_v} = 2^{q_out(v)}. Every factor of a non-root leaf is cupped to its parent, its only neighbour, so its output field is the whole word, Q_v = q_out(v), and its reduced vector is its word tensor. The displayed formula applies to nodes with at least one child (and to a root without children, where it reads 2^{Q_v} + 2^{q_s}).

Under Lemma 4.10(i) the last term drops 2^{q_out(v)}. The fused pass costs 2^{Q_v} x (number of children) multiply-adds.

Examples: for N TV N the root term is 2^Q + 2 x 2^{q_n} + 2^{q_s}, giving 14 at (1,1) and 42 at (2,1). The addendum's T22 value for "Alice who loves Bob sees Carol who hates Dan", 16 at (1,1) (12 in place), is asserted there.

*Proof.* Induction over the tree from the leaves. At a leaf nothing is computed: the word tensor is the reduced vector, which gives the base case. At an inner node, while child k is being evaluated, the storage in use is its own evaluation plus the finished results of the children k' < k. During the node's own pass, the word tensor, all child results and the output are in use at once. QED

**Proposition 4.15 (optimal child order; corrects the rule of (11.18')).** Fix children with peaks Pk_k and output sizes rs_k = 2^{q_k}. The order that minimises max_k [Pk_k + sum_{k' < k} rs_{k'}] is the order of DECREASING Pk_k - rs_k. The addendum's rule, decreasing Pk_k, is optimal when all rs_k are equal, and can be strictly worse otherwise.

Example: take Pk_i = 10, rs_i = 9, Pk_j = 9, rs_j = 1, with prefix Sp. Peak-first (i, then j) gives Sp + max(10, 18) = Sp + 18. The rule above (j, then i) gives Sp + max(9, 11) = Sp + 11.

*Proof.* This is an exchange argument. Let i, j be adjacent, with i first and prefix Sp. Their two terms are max(Pk_i + Sp, Pk_j + Sp + rs_i). Swapped, they are max(Pk_j + Sp, Pk_i + Sp + rs_j). No other term changes.

Suppose Pk_i - rs_i >= Pk_j - rs_j. Then Pk_j + rs_i <= Pk_i + rs_j and Pk_i <= Pk_i + rs_j, so the unswapped maximum is no larger than the swapped one. Bubble sort into decreasing Pk - rs never increases the objective.

If all rs_k are equal, the two orders coincide. (Pk_k >= rs_k always holds, since a child's peak includes its output.) QED

**Proposition 4.16 (crude bound)** [addendum (11.20)]. Let q_max be the maximum edge width in Gw after merging, including q_s, and let Q_max = max_w Q_w. Then peak(root) <= 2^{Q_max} + n_w 2^{q_max}.

*Proof.* Induction on Theorem 4.14. Each held or output vector has at most 2^{q_max} entries, and each word contributes at most one of them. QED

**Proposition 4.17 (cycles)** [addendum Sec. 11.4, (11.18'')].

(a) Consider a wrapper word of type X s^l n. Its n cups a verb's n^r. Its s^l cups the s of the k-th post-verbal modifier of type s^r s, where modifier i cups its s^r to the s of its left neighbour. Then the wrapper closes a cycle of length k + 2 in Gw.

(b) Label the cycle members other than the wrapper v_1..v_mm, in order from the wrapper's a-side. Let the on-cycle edge widths be q_0 := q_a (wrapper to v_1), q_1, ..., q_mm := q_b (v_mm to wrapper). Assume each member, after absorbing its off-cycle subtrees, has exactly its two on-cycle fields open, so that it is a matrix M_i of shape 2^{q_{i-1}} x 2^{q_i}.

Contract Mp_i = M_1 ... M_i from v_1 onward. Then contract the wrapper over both open fields of Mp_mm. The peak storage is at most the maximum of three terms:

- the peaks of the off-cycle subtrees, plus the results held at the time (Theorem 4.14);
- max_i [2^{Q_{v_i}} + 2^{q_0 + q_{i-1}} + 2^{q_0 + q_i}], if each M_i is formed just before its chain step;
- 2^{Q_wrap} + 2^{q_0 + q_mm} + 2^{Q_wrap - q_a - q_b}.

For the wrapper type X = n^r s, the wrapper term is 16 + 4 + 4 = 24 at (1,1) (Q_wrap = 4, q_a = q_n, q_b = q_s) and 64 + 8 + 8 = 80 at (2,1). These equal the (11.18'') values when the other two terms are smaller. That is not automatic: a member with on-cycle widths (1,2) and Q_v = 4 gives a chain term of 16 + 4 + 8 = 28 > 24.

(c) Conjectures (UNVERIFIED):

- each wrapper has at most one back edge;
- the Sec. 11.2 lexicon produces no nested cycles;
- a spider wrapper adds no storage beyond the chain.

The treewidth of word graphs for general lexica is left open [spec Sec. 10 item 8].

*Proof.* (a) The edges wrapper-verb, verb-mod_1, mod_i-mod_{i+1} and mod_k-wrapper form a closed walk through k + 2 distinct words.

(b) Each chain step is Lemma 4.6, with the partial product Mp_{i-1} (2^{q_0 + q_{i-1}}), the word tensor of v_i and the new partial product Mp_i (2^{q_0 + q_i}) all present. The final step is a single fused contraction. QED

**Theorem 4.18 (cut-width bound for the sweep)** [addendum (11.21)]. Let M be a planar cup set with nesting depth nd = nd(M) (Definition 1.27).

- (i) The cups crossing any boundary x are pairwise nested.
- (ii) cw(x) <= nd.
- (iii) The open tensor at x has at most nd + 1 fields (the survivor included) and at most 2^{q_max (nd + 1)} entries.
- (iv) The sweep's peak storage is at most 3 x 2^{max(q_max (nd + 1), Q_max)}.

So (11.21) holds as stated in the addendum, whose D is nd.

*Proof.* (i) Two cups (i,j) and (i',j') with i < i' < x <= j and j' != j do not cross only if i < i' < j' < j, that is, if they are nested.

(ii) In a family of mm pairwise nested cups, the innermost cup has depth >= mm.

(iii) The fields of T at x are the left ends of the crossing cups, plus possibly the survivor.

(iv) Each of T, phi_w and R is bounded as in (iii) or by 2^{Q_max}.

Example: take "Alice who loves Bob believes that Carol sleeps", with positions Alice n(0); who (1-4); loves (5-7); Bob (8); believes (9-11); that (12,13); Carol (14); sleeps (15,16). Just before "loves" the crossing cups are (2,9), (3,6) and (4,5). These are nested, so cw = 3, and #T = 2^{2 q_n + q_s}. Both (4,5) and (3,6) join T to "loves", so R has the fields 2 and 7. The peak is 8 + 8 + 4 = 20 at (1,1) and 32 + 32 + 16 = 80 at (2,1), against 14 and 42 for the tree order. QED

## 4.5 Frobenius spiders as entrywise products

**Proposition 4.19 (spider node)** [addendum (11.7), (11.9), C15]. Let a spider on basic type b have legs to children k = 1..mm with reduced vectors rv_k in C^{d_b}, and either one output leg or none.

- With one output leg, Res[o] = prod_k rv_k[o].
- With no output leg and mm >= 2, Res = sum_o prod_k rv_k[o].
- With no output leg and mm = 1, the leg is closed by the counit e_F, and Res = sum_o rv_1[o].

The cost is mm d_b complex multiplications (plus d_b additions when closed), and the storage is d_b.

*Proof.* The spider tensor delta^{(mm+1)} is 1 iff all its indices are equal. Contracting it against rv_1 (x) ... (x) rv_mm leaves the single surviving index o with value prod_k rv_k[o]. Closing o sums over it. QED

Instances [addendum (11.14)-(11.17')]:

- who: Res[a] = Alice[a] sum_c R_loves[a + d c];
- and_n: Res[a] = Alice[a] Bob[a];
- and_vp: sigma[s] = sum_a Alice[a] IV1[a + d s] IV2[a + d s];
- and_s: sigma[s] = sigma_1[s] sigma_2[s].

The sum semantics of Proposition 2.33 gives sigma = sigma_1 T_2 + T_1 sigma_2 with T_x = sum_i x[i] [addendum Sec. 11.3, text after (11.17')].

## 4.6 Matrix product states

**Definition 4.20 (MPS)** [addendum (14.6), (14.1)]. Take N_s sites of local dimension d. The site tensors are A_i[alpha][t][beta], with alpha < chi_{i-1}, beta < chi_i and chi_{-1} = chi_{N_s - 1} = 1. The state is

    Psi[t_0 + d t_1 + ... + d^{N_s - 1} t_{N_s - 1}] = A_0[t_0] A_1[t_1] ... A_{N_s - 1}[t_{N_s - 1}]

(a product of matrices). A product state, such as the DisCoCirc initial register, is the MPS with every chi_i = 1 and A_i[0][t][0] = phi_{n_i}[t].

Site i is *left-normalised* if sum_{alpha,t} conj(A_i[alpha][t][beta]) A_i[alpha][t][beta'] = delta_{beta beta'}. It is *right-normalised* if sum_{t,beta} A_i[alpha][t][beta] conj(A_i[alpha'][t][beta]) = delta_{alpha alpha'}.

The *bond-centred form at bond i* has sites 0..i left-normalised, a diagonal matrix diag(xi) on bond i, and sites i+1..N_s-1 right-normalised. The *site-centred form at site c* has sites < c left-normalised and sites > c right-normalised.

**Proposition 4.21 (bond dimension)** [addendum (14.7)]. Let sr_i(Psi) be the Schmidt rank of Psi across cut i (sites 0..i versus the rest).

- Every Psi has an exact MPS with chi_i = sr_i(Psi) <= min(d^{i+1}, d^{N_s - 1 - i}).
- Every MPS with bonds chi_i has sr_i(Psi) <= chi_i.
- An MPS stores sum_i d chi_{i-1} chi_i <= N_s d chi^2 complex numbers.

*Proof.* Cut i gives a d^{i+1} x d^{N_s - 1 - i} coefficient matrix, whose rank is at most either dimension.

Existence: take the SVD at cut 0, set A_0 to the left singular vectors, and reshape the remainder xi Vm^dag to (chi_0 d) x d^{N_s - 2}. Iterate. At each cut the rank equals sr_i, because the left factors built so far are isometries and so do not change rank.

Conversely, (14.6) writes the matrix of cut i as the product of a d^{i+1} x chi_i matrix and a chi_i x d^{N_s - 1 - i} matrix. QED

**Theorem 4.22 (Eckart-Young-Mirsky; Eckart and Young 1936, Mirsky 1960).** Let Ma have singular values xi_0 >= xi_1 >= ... . For every B of rank at most chi,

    ||Ma - B||_F^2 >= sum_{k >= chi} xi_k^2,

with equality when B is the truncated SVD.

*Proof.* Weyl's inequality for singular values (Horn and Johnson, *Topics in Matrix Analysis*, Thm 3.3.16) gives xi_{k + chi}(Ma) <= xi_k(Ma - B) + xi_chi(B) = xi_k(Ma - B). Square and sum over k. The truncated SVD attains the bound, by Theorem 3.39(a). QED

**Theorem 4.23 (bond-centred truncation is optimal)** [addendum (14.10)-(14.12)]. Take an MPS in bond-centred form at bond i. Then:

- (i) the xi_k are the Schmidt coefficients of the whole state across cut i;
- (ii) keeping the chi largest of them gives the best approximation of Psi in norm among all states of Schmidt rank at most chi across cut i;
- (iii) ||Psi_tr - Psi||^2 = eps_tr^2 = sum_{k >= chi} xi_k^2, and ||Psi_tr||^2 = ||Psi||^2 - eps_tr^2.

*Proof.* Contracting A_0..A_i gives a map Lm: C^{chi_i} -> C^{d^{i+1}}. It is an isometry, by induction over the left-normalised sites: a matrix with orthonormal columns composed with a further left-normalised site still has orthonormal columns. The same holds on the right. So Psi = sum_k xi_k Lm(e_k) (x) Rm(e_k) with orthonormal images on both sides. This is a Schmidt decomposition, and the Schmidt coefficients are unique, which gives (i). (ii) is Theorem 4.22 applied to the matrix of cut i. (iii) is Pythagoras in the Schmidt basis. QED

**Proposition 4.24 (two-site gate update)** [addendum (14.8)-(14.12), C6, C17, T21]. Take the site-centred form at site i. The update applies G to sites i and i+1 as follows:

1. Form Theta = A_i A_{i+1}, contracted over the bond.
2. If G is a verb gate whose subject sits on site i+1, replace G by Pm G Pm^T (Lemma 2.36(ii)).
3. Form Theta' = G Theta on the merged physical index t + d t'.
4. Reshape Theta' to a (d chi_{i-1}) x (d chi_{i+1}) matrix and take its SVD Um diag(xi) Vm^dag.
5. Set A_i := the first chi_i columns of Um, and A_{i+1} := xi_k conj(Vm[.][k]) for k < chi_i. Here chi_i := min(rr, chi) and rr = #{k : xi_k > tol xi_0}, so zero singular values are dropped rather than divided by.

The result is in site-centred form at site i+1. It equals G Psi exactly when chi_i = rank(Theta'). Otherwise it is the optimal truncation, with error eps_tr^2 as in Theorem 4.23; G need not be unitary.

A single-Kraus one-site update, A_i[alpha][t'][beta] = sum_t Kr[t'][t] A_i[alpha][t][beta], preserves the canonical form only if it is applied at the centre site (or if Kr is unitary). It costs d^2 chi^2 multiply-adds.

*Proof.* Theta reproduces the two-site block of (14.6), and G acts only on the physical indices. At full rank the SVD reconstructs the matrix exactly. Um has orthonormal columns, so A_i is left-normalised and the centre moves to i + 1. The other sites are untouched, so Theorem 4.23 applies at bond i. A non-unitary Kr applied away from the centre destroys the normalisation of its site. QED

(A numerical check with chi = 8, N_s = 6 and 30 random U(4) gates gave ||Psi_mps - Psi_exact||^2 = 4.5e-12. That check is consistent with the proposition and is not part of the proof.)

**Proposition 4.25 (relative discarded weight; accumulated error).** Assume ||Psi_0|| = 1 and that every operation between truncations is unitary.

- (a) If ||Psi||^2 = nu^2 before a truncation with absolute discarded weight eps_tr^2, then ||Psi_tr||^2 = nu^2 (1 - eps_rel^2), where eps_rel^2 = eps_tr^2 / nu^2. After K_tr truncations without renormalisation, ||Psi||^2 = prod_k (1 - eps_{k,rel}^2). So the text normaliser Z_text := ||Psi_final||^2 satisfies ln Z_text = sum_k ln(1 - eps_{k,rel}^2). Formula (14.12) as written assumes nu = 1.
- (b) ||Psi_final - Psi_exact|| <= sum_k eps_k, where eps_k is the norm discarded at truncation k.

*Proof.* (a) Theorem 4.23(iii) at each truncation. Unitaries preserve norms.

(b) Let e_k be the error after step k. A unitary step preserves ||e||. A truncation Trn_k maps y to Trn_k(y), and ||Trn_k(y) - x|| <= ||Trn_k(y) - y|| + ||y - x|| = eps_k + ||e_{k-1}||, where x is the exact state. Now induct. QED

**Remark 4.26 (moving the centre; purification; tangents)** [addendum (14.10)-(14.11)].

- Moving the centre without a gate means taking the QR decomposition of A_i as a (d chi_{i-1}) x chi_i matrix and multiplying the R factor into A_{i+1}.
- A word of Kraus rank rr purifies as an environment leg on its site, which makes the local dimension d rr in Proposition 4.24.
- Differentiating through a truncated SVD is not exact. Freezing Um and Vm, or keeping chi >= rank, restores the multilinear adjoint of Theorem 5.25 (forward). The size of the effect is UNVERIFIED.

**Theorem 4.27 (entanglement across a bond)** [addendum (14.15), (14.16); limits SF.6]. For a normalised state with Schmidt coefficients xi_k at a bond, S_cut = -sum_k xi_k^2 log2 xi_k^2 <= log2 chi. The mutual information satisfies I(L:R) = 2 S_cut. For noun wires a in L and b in R, I(a:b) <= 2 log2 chi.

*Proof.* Theorem 3.39(b)-(d). QED

**Proposition 4.28 (the exact rank a text needs)** [addendum Sec. 14.4, (14.17)].

- (a) A two-wire gate G on C^d (x) C^d has osr(G) <= d^2. Applied with one wire on each side of a cut, it multiplies sr_cut by at most d^2.
- (b) Moving a single site across a cut multiplies the Schmidt rank by at most d. So a SWAP of adjacent sites across the cut multiplies it by at most d^2, and this bound is attained.
- (c) Start from a product state and apply G_1, ..., G_T. Let G_i be the number of gates whose two wires lie on opposite sides of cut i, measured in the wires' own positions. Then chi_exact(i) <= min(d^{i+1}, d^{N_s - 1 - i}, d^{2 G_i}). Swaps used to implement non-adjacent gates do not enter the exact bound, because the swap-conjugated gate equals the gate on the original wires. An intermediate MPS during a swap network obeys the bound with an extra factor d per single-site crossing.
- (d) (Converse.) If a set Af of k wires in L satisfies I(Af : R) = 2k log2 d, then sr_cut >= d^k.

*Proof.* (a) The operator space on C^d has dimension d^2, so G = sum_{mm <= d^2} Xo_mm (x) Yo_mm. If Psi = sum_{k <= chi} xi_k u_k (x) w_k, then G Psi = sum_{mm,k} xi_k Xo_mm u_k (x) Yo_mm w_k has at most d^2 chi terms.

(b) The matrix of Psi across the cut L | (j, R') has rank chi. Consider the matrix across (L, j) | R'. Fixing the value of site j splits it into d blocks, each of rank at most chi. The example Bell(0,1) (x) Bell(2,3) has rank 1 across cut 1, and after SWAP(1,2) it has rank 4 = d^2 at d = 2.

(c) Compose (a) over the gates that straddle cut i, starting from rank 1, and intersect with Proposition 4.21.

(d) I(Af:R) <= I(L:R) = 2 S(rho_L) <= 2 log2 sr_cut, by Theorem 3.39(c),(d). QED

**Proposition 4.29 (two-wire reduced density matrix)** [addendum (14.19'), (14.12)]. Take the site-centred form at site qa, and let qa < qb with sites > qb right-normalised. Then

    rho_{qa qb}[t + d t'][tt + d tt'] = sum_{c,c'} Lenv[(c,c')][(t,tt)] Yenv[(c,c')][(t',tt')].

Here Lenv is built from A_qa and propagated through sites qa+1..qb-1 by contracting each site once against Lenv and once against its conjugate. Yenv[(c,c')][(t',tt')] = sum_e A_qb[c][t'][e] conj(A_qb[c'][tt'][e]). The cost is O((qb - qa) d^3 chi^3), and Tr rho_{qa qb} = ||Psi||^2.

*Proof.* By left-normalisation, the environment of the sites < qa is the identity on the bond (induction from site 0). By right-normalisation, so is the environment of the sites > qb. What remains is the sum over the physical indices of sites qa+1..qb-1, which the recursion evaluates site by site. QED

**Remark 4.30 (traced readout)** [spec Sec. 1.5; limits l.715]. Tracing the cup field of R[s,b], instead of contracting it against n2, gives rho_s[s][s'] = sum_b R[s,b] conj(R[s',b]). This is the mixed-state readout of spec Sec. 1.5, and it is NOT (1.6).

Example: R = I_2 and n2 = (1,0). Readout (1.6) gives p = (1, 0), while the traced diagonal divided by its trace gives (1/2, 1/2).

On an MPS, the joint distribution over all sites is not available without 2^W work. Per-site sampling replaces it; its constant is UNVERIFIED.

## 4.7 Tensor-train (MPO) verbs

**Definition 4.31 (tensor-train verb)** [addendum (12.13), T17]. A tensor-train verb is

    V[a,s,b] = sum_{r1, r2 < r} A[a][r1] B[r1][s][r2] C[r2][b].

It stores d r + r^2 d_s + r d numbers, against d^2 d_s for a dense verb. The flat layouts are A[a r + r1] (row-major), C[r2 d + b] (row-major) and B[r1 + r s + r d_s r2] (little-endian).

**Lemma 4.32.**

- (a) V, viewed as a matrix (a) x (s,b), has rank at most r. Viewed as (a,s) x (b), it also has rank at most r.
- (b) Every V admits an exact tensor train with r <= d. Take r_1 = the Schmidt rank of the (a)|(s,b) cut and r_2 = the Schmidt rank of the (a,s)|(b) cut.
- (c) The two-stage contraction xa[r1] = sum_a n1[a] A[a][r1], wv[r2] = sum_b C[r2][b] n2[b], sigma[s] = sum_{r2} (sum_{r1} xa[r1] B[r1][s][r2]) wv[r2] equals (1.5). It costs 2 d r + r^2 d_s + r d_s multiply-adds and needs 2r + d_s + 1 entries besides A, B and C.
- (d) The bilinearity of V in (n1, n2), and its sensitivity to their order, are unaffected. Only the rank is constrained.

Integer check [addendum T17], with d = r = d_s = 2 and the layouts of Definition 4.31: A = [1,2,3,-1], C = [2,0,1,1], B = [2,4,3,6,-1,1,0,3], n1 = (1,2), n2 = (3,-1). Then xa = (7,0), wv = (6,2) and sigma = (70, 126) by both paths.

*Proof.* (a) V_{(a)x(s,b)} = A times the contraction of B and C, which factors through C^r. The (a,s)|(b) cut is the same with C.

(b) Take two successive SVDs (Proposition 4.21 with the three sites (a), (s), (b)). The ranks are bounded by the smaller side of each cut, and both cuts have a side of dimension d.

(c) Substitute the definition into (1.5) and reorder the finite sums.

(d) (1.5) is unchanged as a function of the nouns. QED

**Lemma 4.33 (degenerate verbs)** [limits Sec. 2.7 (7), Sec. 2.8 (7)].

- (a) Let phi_V[k] = cst exp(i gam k) with gam real and k = a + d s + d d_s b. Then sigma[s] = cst e^{i gam d s} (sum_a n1[a] e^{i gam a})(sum_b n2[b] e^{i gam d d_s b}). If Z != 0, abs(sigma[s]) is constant in s, so p is uniform.
- (b) Let V[a,s,b] = cv[s] n1[a] n2[b] for the sentence's own nouns. Then sigma = cv (n1^T n1)(n2^T n2), p depends only on cv, and with eps_reg = 0 every derivative of p with respect to the nouns vanishes.

*Proof.* (a) The exponential factorises. (b) Substitute into (1.5). A non-zero scalar cancels in p. QED

## 4.8 The DisCoCirc register and the product approximation

**Definition 4.34 (text state)** [addendum (14.1)-(14.5), (12.17)]. Psi_0 is the product state of Definition 4.20, and Psi_{T_cl} = G_{T_cl} ... G_1 Psi_0. The two-layer discourse form is

    sigma_2[t] = sum_{s1,s2} sigma_hat_1[s1] Dsc[s1 + d_s t + d_s^2 s2] sigma_hat_1'[s2],

where Dsc is the discourse tensor.

**Proposition 4.35 (register width)** [addendum (14.5), (12.18)].

- The exact register has 2^{q_n N_e} amplitudes, independently of the number of clauses and of their nesting. Nesting only adds gates, Ng_text = sum_clauses Ng(c).
- If the wires of retired entities are each in a product state with the rest (or are postselected), the register can be restricted to the E_live live entities, with 2^{q_n E_live} amplitudes. Without that hypothesis, retiring an entangled wire leaves a mixed state on the rest. That state needs a density operator (4^{q_n E_live} entries) or a purification.
- The layered-memory formula of (12.18), max(2^Q + 2^{q_n} + (n-1) d_s, 2^{3 q_s} + n d_s), holds under the hypotheses stated in Proposition 4.35'.

*Proof.* Every gate maps C^{2^{q_n N_e}} to itself. If Psi = phi_ret (x) Psi_live, then discarding the retired wire leaves the pure state Psi_live. QED

**Proposition 4.35' (two-layer peak storage)** [addendum (12.17), (12.18)]. Assume:

- the n first-layer sentences are evaluated one after another by fused sweeps, each with peak storage at most P1 (P1 = 2^Q + 2^{q_n} for N TV N, by Lemma 4.10 and Proposition 4.11);
- of each first-layer sentence only its d_s-entry output is retained;
- all other first-layer storage is released before layer 2, and the discourse tensor Dsc (2^{3 q_s} entries) is not held during layer 1;
- layer 2 contracts Dsc with the n retained vectors (Definition 4.34 for n = 2) and writes its output into a slot of Dsc (Lemma 4.10(i), (ii)).

Then the peak storage is max(P1 + (n-1) d_s, 2^{3 q_s} + n d_s). At (q_n, q_s) = (4,2) with n = 2 this is max(1024 + 16 + 4, 64 + 8) = 1044.

*Proof.* The two layers are disjoint in time, so the peak is the larger of the two phase peaks. While sentence i is evaluated, the entries in use are those of its sweep (at most P1) and the i - 1 <= n - 1 outputs already finished, each of d_s entries. During layer 2 only Dsc and the n inputs exist, because the fused contraction steps write into slots of Dsc (Lemma 4.10(i) for the first step, where the open tensor is an input vector with no rest fields; Lemma 4.10(ii) for the later steps). The numbers are Q = 2 q_n + q_s = 10, 2^{q_n} = 16, d_s = 4 and 2^{3 q_s} = 64. QED

**Definition 4.36 (mean-field state; product approximation)** [addendum Sec. 13.2(a)].

- The *mean-field state* of Psi is rho_MF = rho_1 (x) ... (x) rho_{N_e}, where rho_j is the one-wire marginal of Psi. It is mixed in general.
- The *mean-field update* of a gate G on wires i, j is rho_j' = Tr_i[G (rho_i (x) rho_j) G^dag], and similarly for rho_i'.
- The *chi = 1 truncation* is Proposition 4.24 with chi = 1. It keeps a pure product state.
- A *best product approximation* is a product vector nearest to Psi in norm.

**Proposition 4.37 (four different approximations).**

- (a) rho_MF reproduces every one-wire marginal of Psi exactly, and has I(a:b) = 0 for every pair of wires. It equals |Psi><Psi| iff Psi is a product state, that is, iff sr_i(Psi) = 1 at every cut.
- (b) Across a single cut, the best product approximation is the leading Schmidt term xi_0 u_0 (x) conj(v_0), with squared error 1 - xi_0^2 (for a normalised state). Bond-by-bond chi = 1 truncation is optimal for each single cut, given the rest. It does not in general give the global best product vector. For the three-qubit W state Wst = (|001> + |010> + |100>)/sqrt3, the best product fidelity is 4/9, while sequential truncation from the left reaches only (2/3)(1/2) = 1/3.
- (c) Fix wires i, j and a two-wire gate G, and let rho_ij be the two-wire marginal of Psi before the gate, with one-wire marginals rho_i, rho_j.
  - (c1) If rho_ij = rho_i (x) rho_j, then the mean-field update rho_j' = Tr_i[G (rho_i (x) rho_j) G^dag] equals the exact marginal Tr_i[G rho_ij G^dag].
  - (c2) For a fixed G the converse fails. If G = Ao (x) Bo, then Tr_i[G X G^dag] = Bo Tr_i[X] Bo^dag for every X, so the update is exact for every rho_ij; for example G = I on a Bell pair, whose joint state is not a product.
  - (c3) The update is exact for EVERY two-wire unitary G iff rho_ij = rho_i (x) rho_j.

  The chi = 1 truncation discards 1 - xi_0^2 at each bond it touches, and it is exact iff G maps the current product state to a product state.
- (d) For a Bell pair, rho_MF = I/2 (x) I/2 keeps the marginals, whereas the chi = 1 truncation gives |00>, with marginal |0><0|. So (a) and (c) describe different objects.

*Proof.* (a) Partial traces of rho_MF give the rho_j, and the mutual information of a product state is 0. If |Psi><Psi| is a product, each factor is a marginal of a pure state and has rank 1.

(b) Theorem 4.22 with chi = 1 at each cut. For Wst: the first cut has Schmidt weights 2/3 and 1/3, and keeping the larger gives fidelity 2/3. The remaining state (|01> + |10>)/sqrt2 has weights 1/2 and 1/2, and keeping one gives fidelity 1/2.

Attainment. At the symmetric product vector (sqrt(2/3)|0> + sqrt(1/3)|1>)^{(x)3} the overlap is 3 x (2/3)(1/sqrt3)/sqrt3 = 2/3, so the fidelity is 4/9.

Upper bound. Let a, b, cv be unit vectors in C^2. Contracting cv first gives <a (x) b (x) cv|Wst> = sum_{x,y} conj(a_x) Mc[x][y] conj(b_y) = a^dag Mc conj(b), with the 2 x 2 matrix Mc = 3^{-1/2} [[conj(cv_1), conj(cv_0)], [conj(cv_0), 0]]. So the maximum of |<a b cv|Wst>| over unit a, b is at most ||Mc||_op. For every complex matrix, ||Mc||_op <= || |Mc| ||_op, where |Mc| is the entrywise modulus, because |Mc zv| <= |Mc| |zv| entrywise for every vector zv. Put x = |cv_0| and y = |cv_1|, with x^2 + y^2 = 1. Then |Mc| = 3^{-1/2} [[y, x], [x, 0]] is real symmetric, with largest eigenvalue 3^{-1/2} (y + sqrt(y^2 + 4x^2))/2 = 3^{-1/2} (y + sqrt(4 - 3y^2))/2. The derivative of y + sqrt(4 - 3y^2) is 1 - 3y/sqrt(4 - 3y^2), which vanishes on [0,1] only at y = 1/sqrt3; there the value is 4/sqrt3, against 2 at y = 0 and y = 1. So the eigenvalue is at most 3^{-1/2} (2/sqrt3) = 2/3, and |<a b cv|Wst>|^2 <= 4/9 for every product vector. (A random search over 2 x 10^4 product vectors found a maximum of 0.4385 < 4/9, consistent with the bound; the search is not part of the proof.)

(c1) Tr_i[G (rho_i (x) rho_j) G^dag] = Tr_i[G rho_ij G^dag] when the two arguments coincide.

(c2) For G = Ao (x) Bo with Ao unitary, Tr_i[(Ao (x) Bo) X (Ao (x) Bo)^dag] = Bo Tr_i[(Ao^dag Ao (x) I) X] Bo^dag = Bo Tr_i[X] Bo^dag by cyclicity of the partial trace in the i factor. Both rho_ij and rho_i (x) rho_j have partial trace rho_j over i, so both sides equal Bo rho_j Bo^dag.

(c3) "If" is (c1). "Only if": put Dl = rho_ij - rho_i (x) rho_j. Dl is Hermitian, Tr Dl = 0, and both partial traces of Dl vanish. The difference between the exact and the mean-field marginal is Tr_i[G Dl G^dag], so suppose this vanishes for every unitary G, and suppose Dl != 0. Write Dl = sum_k ev_k |f_k><f_k| in an orthonormal eigenbasis (k runs over d^2 indices). For a bijection beta from eigen-indices k to product labels (a, b), let G_beta be the unitary with G_beta f_k = |a>_i |b>_j, where (a, b) = beta(k). Then

    Tr_i[G_beta Dl G_beta^dag] = sum_b cs_b(beta) |b><b|,       cs_b(beta) = sum_{k : beta(k) = (., b)} ev_k,

so cs_b(beta) = 0 for every column b and every beta. If two eigenvalues differ, ev_k != ev_l, choose beta with k and l in different columns b != b' (possible since d >= 2), and let beta' exchange the labels of k and l. Then cs_b(beta') - cs_b(beta) = ev_l - ev_k != 0, a contradiction. So all ev_k are equal, and since Tr Dl = 0 they are all 0, contradicting Dl != 0.

The chi = 1 statement: by Theorem 4.23(iii) the discarded weight at a bond is sum_{k >= 1} xi_k^2 = 1 - xi_0^2, which is 0 iff the state after the gate has Schmidt rank 1 at that bond, that is, iff G maps the current product state to a product state.

(d) For a Bell pair the one-wire marginals are I/2, so rho_MF = I/2 (x) I/2. Its Schmidt coefficients are 2^{-1/2}, 2^{-1/2}, and keeping the first Schmidt term in the computational Schmidt basis gives |00> (up to normalisation), whose marginal is |0><0|. QED

**Remark 4.38 (status).** Proved in this chapter:

- the correctness of the fused sweep, and the corrected in-place rule (E-13, with its hypotheses);
- the peak recurrence (11.18') on trees, and the corrected optimal child order;
- the chain bound for cycles, of which (11.18'') is the wrapper term;
- the cut-width bound (11.21), with nd = the addendum's D;
- the spider node;
- the MPS bounds (14.7) and the optimality results (14.10)-(14.12), with relative weights;
- the accumulated-error bound;
- the entanglement bound (14.15)-(14.16) and the exact-rank bound (14.17), with swaps counted correctly;
- the tensor-train verb bounds (12.13)-(12.14).

UNVERIFIED: the items of Proposition 4.17(c), the cycle frequency (a generator statistic), the per-site sampling constant and the effect of inexact SVD tangents. The layered-memory formula (12.18) is proved under explicit hypotheses (Proposition 4.35').
# Chapter 5. Differentiation of circuit and tensor functionals

A circuit is a list of gates g_1, ..., g_Ng. The operator is U = g_Ng ... g_1 (C4), and it acts on W = |K| + q_s qubits. A gate is either *fixed* (H, CNOT, CZ) or *parametrised*: g_k = exp(-i alpha_k G_k/2) with G_k Hermitian (the generator) and Gamma_k := -(i/2) G_k. Fixed gates contribute no derivative term. The Kronecker symbol (x) with qubit subscripts names the qubits acted on; no C2 order is implied.

## 5.1 The objective and the generator calculus

**Definition 5.1 (postselected readout and losses)** [spec (5.1), Sec. 4.5, Sec. 1.4]. Let b(c) be the basis index whose K-bits are 0 and whose survivor field holds c. Put

- N_c = abs(psi_Ng[b(c)])^2;
- D = sum_c N_c, which lies in [0, 1];
- p_c = N_c / D.

In operator form, O_c := |b(c)><b(c)|, so N_c = <psi_Ng|O_c|psi_Ng> and D = <psi_Ng|sum_c O_c|psi_Ng>; under (H-readout) sum_c O_c = Pi := Pi_K (Definition 3.24), so D = <psi_Ng|Pi|psi_Ng>. We write Vk := g_Ng ... g_{k+1} for the gates after gate k.

The losses are:

- cross-entropy: Loss_CE = -sum_c y_c ln(p_c + eps_reg), with g_c = -y_c/(p_c + eps_reg);
- squared error: Loss_MSE = sum_c (p_c - y_c)^2, with g_c = 2(p_c - y_c).

**Lemma 5.2 (derivative of a gate)** [spec C13; limits Sec. 2.6 (1)]. For U(alpha) = exp(-i alpha G/2) with G Hermitian:

- U is unitary;
- dU/dalpha = Gamma U = U Gamma, where Gamma = -(i/2) G;
- dU^dag/dalpha = -U^dag Gamma.

No hypothesis on G^2 is needed.

*Proof.* Unitarity: (exp(-i alpha G/2))^dag = exp(i alpha G/2) = U^{-1}. Derivative: the exponential series may be differentiated term by term, and G commutes with its own powers. Adjoint: Gamma^dag = -Gamma. QED

**Lemma 5.3 (frequency content of a gate expectation).** Let G = sum_a ev_a Pr_a be the spectral decomposition, and put f(t) = <phi|U(t)^dag Ob U(t)|phi> for a fixed state phi and a fixed Hermitian Ob. Then

    f(t) = sum_{a,b} e^{i t (ev_a - ev_b)/2} <phi|Pr_a Ob Pr_b|phi>,

so freq(f) is contained in {(ev_a - ev_b)/2}.

- (i) If G^2 = I, then f(t) = a0 + b0 cos t + d0 sin t with a0, b0, d0 real.
- (ii) If the spectrum of G is {0, +1, -1}, then freq(f) is contained in {0, +-1/2, +-1}. In that case f is 4 pi-periodic, but in general not 2 pi-periodic (Proposition 5.21, forward).

*Proof.* Substitute U(t) = sum_a e^{-i t ev_a/2} Pr_a.

(i) Here U = cos(t/2) I - i sin(t/2) G. Expanding,

    U^dag Ob U = (Ob + G Ob G)/2 + cos t (Ob - G Ob G)/2 + sin t (i/2)(G Ob - Ob G),

and each bracket is Hermitian, so a0, b0, d0 are real.

(ii) The halved differences of {0, +-1} are {0, +-1/2, +-1}. QED

**Lemma 5.4 (spectrum of the controlled generator)** [spec C13]. On W >= 2 qubits, let Gc = |1><1|_ctl (x) Pa_tgt with Pa a Pauli matrix and tgt != ctl. Then:

- Gc has eigenvalue 0 with multiplicity 2^{W-1}, and eigenvalues +1 and -1 each with multiplicity 2^{W-2};
- Gc^2 = |1><1|_ctl (x) I;
- exp(-i pi Gc) = Pz_ctl.

The qudit generator Y_pq = -i|p><q| + i|q><p| on C^{dd} also has spectrum {0, +1, -1}, with 0 of multiplicity dd - 2. The generator -2|q><q| has spectrum {0, -2}.

*Proof.* On the control = 0 block Gc = 0. On the control = 1 block Gc = Pa_tgt, which has eigenvalues +-1. Also exp(-i pi Pa) = -I. Y_pq restricted to span{|p>, |q>} is Py. QED

## 5.2 Parameter-shift rules

**Theorem 5.5 (exactness criterion for shift rules).** Let f(t) = sum_omega c_omega e^{i omega t} have an absolutely convergent series with sum abs(c_omega omega) < infinity; a trigonometric polynomial is the finite case. Let h_1, ..., h_mm > 0 be shifts and dc_1, ..., dc_mm real coefficients. Then

    f' = sum_m dc_m Shift_{h_m}[f]

holds iff every frequency omega with c_omega != 0 satisfies

    2 sum_m dc_m sin(omega h_m) = omega.

In particular, if f has non-zero coefficients at frequencies of modulus greater than 2 sum_m abs(dc_m), no rule with these shifts is exact for f.

*Proof.* Shift_h[e^{i omega t}] = 2i sin(omega h) e^{i omega t}, and (e^{i omega t})' = i omega e^{i omega t}. So the difference of the two sides is

    sum_omega c_omega i (2 sum_m dc_m sin(omega h_m) - omega) e^{i omega t},

and this series converges absolutely. Its coefficient at omega is lim_{T -> infinity} (1/2T) integral_{-T}^{T} (difference) e^{-i omega t} dt. So the difference vanishes identically iff every coefficient vanishes. The last claim holds because abs(2 sum dc_m sin(.)) <= 2 sum abs(dc_m). QED

**Theorem 5.6 (two-term rule)** [spec (5.2), C13]. If G^2 = I, then f'(t) = [f(t + pi/2) - f(t - pi/2)]/2 exactly, for every t.

*Proof.* By Lemma 5.3(i), freq(f) is contained in {0, +-1}. Apply Theorem 5.5 with h = pi/2 and dc = 1/2: 2 (1/2) sin(pi/2) = 1. QED

**Theorem 5.7 (four-term rule for spectrum {0, +1, -1})** [spec (5.4), C13]. For generators with spectrum {0, +-1} (CRz, CRx, Y_pq), exactly,

    f'(t) = dc1 [f(t + pi/2) - f(t - pi/2)] + dc2 [f(t + 3pi/2) - f(t - 3pi/2)],
    dc1 = (sqrt2 + 1)/(4 sqrt2) = 0.42677670,    dc2 = -(sqrt2 - 1)/(4 sqrt2) = -0.07322330.

The two-term rule is not exact for such an f whenever its coefficient at +-1/2 is non-zero.

*Proof.* By Lemma 5.3(ii), freq(f) is contained in {0, +-1/2, +-1}. Theorem 5.5 requires 2(dc1 - dc2) = 1 at omega = 1 and sqrt2 (dc1 + dc2) = 1/2 at omega = 1/2, which gives the stated values. For the two-term rule at omega = 1/2, the left side of the criterion is 2 (1/2) sin(pi/4) = 1/sqrt2 != 1/2. QED

**Remark 5.8 (normalisations)** [spec Sec. 5.2; limits Sec. 2.7 (6)]. Writing the CRz generator as |1><1| (x) Pz/2, with spectrum {0, +-1/2}, and using exp(-i t G') is the same statement, because the frequencies are the eigenvalue differences of G'. For G = -2|q><q| (spectrum {0, -2}) the frequencies are {0, +-1}, so the two-term rule IS exact. The claim in the limits document that both qudit generators need more than two terms is correct only for Y_pq.

**Corollary 5.9 (chain-rule alternative)** [spec (5.5)]. Take the gate list CNOT, Rz_tgt(-theta/2), CNOT, Rz_tgt(theta/2) of Lemma 3.20(a). Let the second gate carry an independent angle b and the fourth an independent angle a, and write Fe(a, b) for the resulting expectation. Then f(theta) = Fe(theta/2, -theta/2), and

    f'(theta) = (1/2) d_a Fe - (1/2) d_b Fe,

with each partial derivative given exactly by Theorem 5.6. That is four evaluations in total. The applied angles here are s = +1/2 and s = -1/2 times theta.

*Proof.* Chain rule. In each argument separately, Fe has frequencies in {0, +-1}, by Lemma 5.3(i). QED

## 5.3 Postselected ratios

**Proposition 5.10 (quotient rule; 1/D bound)** [spec (5.3), Sec. 5.1; errata E-10; review 2.8(D)]. Let t be a single applied angle, with generator as in Theorem 5.6 or Theorem 5.7. Then N_c(t) and D(t) are trigonometric polynomials for which the corresponding rule is exact, and

    dp_c/dt = (N_c' - p_c D')/D,       dLoss/dt = sum_c g_c dp_c/dt,       D' = sum_c N_c'.

Moreover abs(N_c') <= 1/2 and abs(D') <= 1/2, so

    abs(dp_c/dt) <= (1 + p_c)/(2D) <= 1/D.

This bound is unbounded as D -> 0. If a parameter theta_p is read by #K_p gates (Lemma 5.20, forward), the per-parameter bound is multiplied by #K_p.

Assume D > 0 for all t. For G^2 = I, the ratio p_c is a trigonometric polynomial only if it is constant or D is constant. In every other case p_c has infinitely many non-zero Fourier coefficients, and no finite shift rule is exact for it.

*Proof.* N_c = <psi|O_c|psi> and D = <psi|sum_c O_c|psi> (= <psi|Pi_K|psi> under (H-readout)) are expectations of Hermitian projectors. As functions of one applied angle they have the form of Lemma 5.3, with Ob = Vk^dag O_c Vk (Definition 5.1).

Bounds. N_c(t) lies in [0, 1].

- In the two-term case, N_c' is half a difference of two numbers in [0,1], so abs(N_c') <= 1/2.
- In the four-term case, abs(N_c') <= abs(dc1) + abs(dc2) = dc1 - dc2 = 1/2.

The same argument applies to D. Then abs(dp_c/dt) <= (abs(N_c') + p_c abs(D'))/D.

Non-exactness. Suppose G^2 = I and p_c = N_c/D = Pq is a trigonometric polynomial. Then N_c = Pq D. Degrees of trigonometric polynomials add under multiplication, since the leading coefficients multiply. N_c has degree at most 1, so either D is constant, or D has degree 1 and Pq is constant. In every other case p_c is a real-analytic 2 pi-periodic function (D > 0) that is not a trigonometric polynomial. Its Fourier series is then absolutely convergent with infinitely many non-zero terms, and the last clause of Theorem 5.5 applies.

For the counterexample to the naive claim "D non-constant implies non-exact", take Rx(t) on qubit 0 and H on qubit 1, with K = {0}. Then D = cos^2(t/2) and N_c = D/2, so p_c = 1/2. QED

**Proposition 5.11 (propagation of readout errors: two models)** [spec (4.7), (5.3); errata E-11].

- (a) *Probability-level errors.* Suppose each N_c carries an error dN_c with abs(dN_c) <= e_N. To first order,
      dp_c = [(1 - p_c) dN_c - p_c sum_{c' != c} dN_{c'}]/D,
  so abs(dp_c) <= (1 + (n_cl - 2) p_c) e_N / D <= max(1, n_cl - 1) e_N / D. This model scales like 1/D.
- (b) *Amplitude errors.* If the state carries an error of norm err, then abs(dp_c) <= 2 sqrt(p_c(1 - p_c)) err/sqrt(D) + O(err^2) <= err/sqrt(D) + O(err^2), where D = p_surv under (H-readout). The constant is 1 (errata E-11 replaces the constant 4 of spec (4.7), (4.9)); its tightness and a non-asymptotic form are Theorem 6.27 (forward). This model scales like 1/sqrt(p_surv).

*Proof.* (a) Differentiate p_c = N_c / D with dD = sum dN_c. Then use the triangle inequality: (1 - p_c) + p_c (n_cl - 1) = 1 + (n_cl - 2) p_c.

(b) Let x in C^{n_cl} be the vector of readout amplitudes, x_c := psi[b(c)], so N_c = |x_c|^2 and D = ||x||^2 (= p_surv under (H-readout)), and p_c = |x_c|^2/D. An error delta of the state changes x by a vector dx with ||dx|| <= err. To first order, d|x_c|^2 = 2 Re(conj(x_c) dx_c) and dD = 2 Re<x, dx>, so

    dp_c = (2/D) Re<vv, dx>,        vv = x_c e_c - p_c x.

Then ||vv||^2 = |x_c|^2 - 2 p_c |x_c|^2 + p_c^2 D = D p_c (1 - p_c), using |x_c|^2 = p_c D. Cauchy-Schwarz gives |dp_c| <= 2 sqrt(p_c(1 - p_c)) err/sqrt(D) + O(err^2), and 2 sqrt(p(1-p)) <= 1 gives the second bound. QED

Statements about E[D] at initialisation hold only under explicit hypotheses:

- E[Z] = 2^{-2 q_n} for a Haar-random verb (Theorem 3.32);
- E[D] = 1/4 for the (1,1) IQP shape under sign-symmetric initialisation (Theorem 5.43, forward).

The IQP ansatz with uniform angles at (2,1) has mean 0.041, not 2^{-4} [errata E-1].

## 5.4 Adjoint (reverse-mode) differentiation

**Definition 5.12 (cotangent of a complex vector)** [addendum (11.27); spec (5.8)]. For a real Loss that is differentiable in Re x and Im x, the cotangent is

    lambda_x[j] = (1/2)(dLoss/dRe x[j] + i dLoss/dIm x[j]).

Equivalently, lambda_x is the unique vector with dLoss = 2 Re<lambda_x|dx> for all dx.

**Lemma 5.13 (transport rules).**

- (a) If y = Ao x with Ao complex-linear, then lambda_x = Ao^dag lambda_y.
- (b) If x feeds several intermediates, the contributions to lambda_x add.
- (c) If Loss = <x|Ob|x> with Ob Hermitian, then lambda_x = Ob x.
- (d) dLoss/dtheta = 2 Re<lambda_x|dx/dtheta>.

*Proof.* (a) 2 Re<lambda_y|Ao dx> = 2 Re<Ao^dag lambda_y|dx>. The representing vector is unique: if lambda and lambda' both represent dLoss, test dx = lambda - lambda' and dx = i(lambda - lambda').

(b) Add the differentials.

(c) d<x|Ob|x> = 2 Re<Ob x|dx>.

(d) Chain rule. QED

**Theorem 5.14 (adjoint differentiation)** [spec (5.6), (5.7); errata Sec. B; limits Sec. 2.6 (1)]. Let psi_k = g_k ... g_1|0>. Let lambda_Ng be the cotangent of psi_Ng, and put lambda_{k-1} = g_k^dag lambda_k and psi_{k-1} = g_k^dag psi_k. Then for every parametrised gate k,

    dLoss/dalpha_k = 2 Re <lambda_k|Gamma_k|psi_k> = Im <lambda_k|G_k|psi_k>.

If Loss = <psi_Ng|Ob|psi_Ng>, then lambda_Ng = Ob psi_Ng, and <lambda_k|psi_k> = Loss for every k.

*Proof.* Let Vk = g_Ng ... g_{k+1}. Then d psi_Ng/d alpha_k = Vk Gamma_k psi_k by Lemma 5.2. By Lemma 5.13(d),

    dLoss/dalpha_k = 2 Re<Vk^dag lambda_Ng|Gamma_k psi_k>,

and Vk^dag lambda_Ng = lambda_k. Also 2 Re(-(i/2) z) = Im z. Fixed gates are handled by the recurrence alone. The final identity holds because Vk is unitary. QED

**Corollary 5.15 (seed for the postselected loss)** [spec (5.8); addendum (11.27')]. Let Loss = Loss(p_0, ..., p_{n_cl - 1}) with p_c as in Definition 5.1, and put

    gbar = sum_c g_c p_c,      w_c = (g_c - gbar)/D.

The seed is lambda_Ng[b(c)] = w_c psi_Ng[b(c)], and lambda_Ng[b] = 0 at every other index. Equivalently, lambda_Ng = O_eff psi_Ng with O_eff = sum_c w_c O_c. For Loss = -ln p_y with eps_reg = 0, w_c = (1 - delta_{cy}/p_y)/D.

*Proof.* dN_c = 2 Re(conj(psi[b(c)]) dpsi[b(c)]) and dp_c = (dN_c - p_c dD)/D. Hence

    dLoss = sum_c g_c dp_c = sum_c w_c dN_c,

and this has the form 2 Re<lambda_Ng|dpsi> for the stated lambda_Ng. QED

**Proposition 5.16 (operation counts of the adjoint sweep)** [spec Sec. 5.3; addendum (11.23'); errata Sec. B]. The sweep k = Ng, ..., 1 computes Im<lambda_k|G_k|psi_k> and then applies g_k^dag to both vectors. It evaluates all n_par partial derivatives using:

- Ng forward gate applications;
- 2 Ng inverse-gate applications;
- one generator application and one inner product per parametrised gate;
- three vectors of length 2^W held at once.

The gate list must list each generator-carrying gate separately, because the sweep needs each g_k^dag and G_k individually.

*Proof.* Count the operations. Each g_k^dag is the same rotation with its angle negated, or a self-inverse fixed gate. QED

**Proposition 5.17 (complex Jacobian by the same sweep)** [limits Sec. 2.6 (3)(a); spec (5.8)]. Seed with the constant vector B_Ng = e_{b(c)} (it is not a loss seed), and let B_k = Vk^dag e_{b(c)}. Accumulate over the sweep

    J_c[p(k)] += s_k (-i/2) <B_k|G_k|psi_k>.

The result is J_c[j] = d sigma[c]/d theta_j, where sigma[c] = psi_Ng[b(c)].

*Proof.* d sigma[c]/d alpha_k = <e_{b(c)}|Vk Gamma_k psi_k> = (-i/2)<B_k|G_k|psi_k>. The factor s_k and the accumulation over all gates k with p(k) = j follow from the chain rule, because alpha_k = s_k theta_{p(k)} is linear in theta. QED

## 5.5 Signs, transposed words and shared parameters

**Lemma 5.18 (transposes and conjugates; general rule).** For the gate set of Lemma 3.18, U^dag = conj(U)^T is the reversed gate list with every angle negated.

More generally, a rotation R_P(t) about a Pauli string P containing n_Y factors Py satisfies

    R_P(t)^T = R_P((-1)^{n_Y} t),       conj(R_P(t)) = R_P(-(-1)^{n_Y} t),

and the same holds for controlled rotations |0><0| (x) I + |1><1| (x) R_P(t).

*Proof.* exp(-i t P/2)^T = exp(-i t P^T/2), and P^T = (-1)^{n_Y} P because Py^T = -Py while Px, Pz and I are symmetric. Also conj(exp(-i t P/2)) = exp(i t conj(P)/2), and conj(P) = (-1)^{n_Y} P. The gate-set statement is Lemma 3.18. QED

**Proposition 5.19 (sign rule for transposed effect words)** [spec C11, Sec. 5.3, C13, T7; limits NU.5]. Let a word appear as U_w^T (Lemma 2.22). Then each of its gates keeps its generator, up to sign, and is applied with angle alpha_k = s_k theta_w[i], where s_k = (-1)^{n_Y}. For the gate set of Lemma 3.18 this means s_k = -1 for Ry and +1 otherwise. Consequently

    dLoss/dtheta_w[i] = s_k dLoss/dalpha_k.

On a doubled register the four appearances of a word carry these signs:

- U: +1;
- U^T: (-1)^{n_Y};
- conj(U): -(-1)^{n_Y};
- U^dag: -1.

In the tree form (Section 5.7) no word is transposed, so every sign is +1.

*Proof.* Lemma 5.18 and the chain rule. QED

**Lemma 5.20 (shared parameters)** [spec Sec. 5.2, Sec. 5.5; errata Sec. B; limits "weight tying"]. Let theta_p be read by the gates k in K_p = {k : p(k) = p}, with alpha_k = s_k theta_p. Then

    dLoss/dtheta_p = sum_{k in K_p} s_k dLoss/dalpha_k.

For shift rules this is the sum of the single-occurrence shift derivatives. For a batch mean, divide by the batch size.

*Proof.* The map theta -> (alpha_k) is linear, with dalpha_k/dtheta_p = s_k [k in K_p]. QED

## 5.6 Periodicity

**Proposition 5.21 (periodicity of the loss)** [spec Sec. 5.6, C5, T10; review 2.8(G)].

(a) Shift a single-qubit rotation angle (G^2 = I) by 2 pi. Then psi_Ng changes by the global sign (-1)^{#occurrences}, and N_c, D, p_c and Loss are 2 pi-periodic.

(b) Shift a controlled-rotation angle by 2 pi. Then U changes by Pz_ctl (Lemma 3.16(ii)). The loss is always 4 pi-periodic, but in general not 2 pi-periodic. Example: prepare |+>|+>, apply CRz(theta)(0 -> 1) and then H on qubit 0, and let pr_0 be the probability that qubit 0 reads 0. Then

    pr_0(theta) = (1 + cos(theta/2))/2,       pr_0(theta + 2 pi) = (1 - cos(theta/2))/2.

(c) Every gate depends on its angle only through (cos(theta/2), sin(theta/2)). So replacing theta by theta + 4 pi n changes neither the state nor any derivative of it.

*Proof.* (a) exp(-i pi G) = -I.

(b) After CRz the amplitude at (b0, b1) is (1/2) exp(i b0 (2 b1 - 1) theta/2). After H on qubit 0, the amplitude with qubit 0 equal to 0 is 2^{-3/2}(1 + e^{i(2b1-1)theta/2}), whose squared modulus is (1 + cos(theta/2))/4. Summing over b1 gives pr_0(theta). Replacing theta by theta + 2 pi flips the sign of cos(theta/2).

(c) Lemma 3.8 and Lemma 3.13. QED

**Definition 5.22 (Adam)** [spec (5.9)]. Fix b1 = 0.9, b2 = 0.999, eps_A = 1e-8, and a step counter t >= 1. Componentwise, starting from mA = vA = 0:

    mA := b1 mA + (1 - b1) g,
    vA := b2 vA + (1 - b2) g^2,
    theta := theta - lr (mA/(1 - b1^t)) / (sqrt(vA/(1 - b2^t)) + eps_A).

**Corollary 5.23 (wrapping mod 4 pi)** [spec Sec. 5.5, 5.6, 6.3]. Suppose Loss depends on theta only through the gate unitaries, with no angle regulariser and no reparametrisation. Then replacing theta by theta mod 4 pi at any step leaves the Adam trajectory of (mA, vA, Loss, gradient) unchanged. Wrapping mod 2 pi is not admissible for controlled angles.

*Proof.* By Proposition 5.21(c) the gradients are unchanged. The Adam state depends only on the gradients. Proposition 5.21(b) rules out 2 pi wrapping. QED

## 5.7 Reverse mode on the word tree

**Definition 5.24 (node operation)** [addendum Sec. 11.7; spec Sec. 1.3]. This is Definition 4.13 on a tree Gw, with root output sigma. The spider nodes are mu (entrywise product), the counit e_F (sum of entries) and chain products Mc = M_1 ... M_kk.

**Theorem 5.25 (tree adjoints)** [addendum (11.27)-(11.29), (11.7), (11.9), (11.18''), T24]. Given the cotangent lambda_{rv_v} of a node's output, the cotangents of its operands are:

    word:     lambda_{phi_v}[j] = lambda_{rv_v}[out(j)] conj( prod_k rv_k[f_k(j)] ),
    child k:  lambda_{rv_k}[x] = sum_{j : f_k(j) = x} conj( phi_v[j] prod_{k' != k} rv_{k'}[f_{k'}(j)] ) lambda_{rv_v}[out(j)],
    mu:       lambda_uu[i] = conj(vv[i]) lambda_out[i],   lambda_vv[i] = conj(uu[i]) lambda_out[i],
    e_F:      lambda_uu[i] = lambda_out,
    chain:    lambda_{M_i} = (M_1 ... M_{i-1})^dag lambda_Mc (M_{i+1} ... M_kk)^dag.

The root seed is lambda_sigma[c] = w_c sigma[c] (Corollary 5.15). Contributions from repeated reads add. Postselection appears nowhere: Z enters only through w_c.

*Proof.* Word: fix all operands except phi_v. Then rv_v = Ao phi_v with Ao[o][j] = [out(j) = o] prod_k rv_k[f_k(j)], and lambda_{phi_v} = Ao^dag lambda_{rv_v} by Lemma 5.13(a).

Child k: in the same way, rv_v = Ao_k rv_k. The total differential of a multilinear map is the sum of its partial linear maps (Lemma 5.13(b)).

mu and e_F: these are a diagonal map and an all-ones map.

Chain: dMc = sum_i M_1 ... dM_i ... M_kk. With the Frobenius pairing 2 Re Tr(lambda^dag dMc) and cyclicity of the trace, this gives the stated cotangent. QED

**Theorem 5.26 (angle gradients on the tree)** [addendum (11.30); spec Sec. 5.3, 5.5]. Let the word circuit be g_1, ..., g_m, with phi_v = g_m ... g_1|0>. Put psi_k^w = g_k ... g_1|0> and lambda_k^w = g_{k+1}^dag ... g_m^dag lambda_{phi_v}, where lambda_{phi_v} is the word cotangent of Theorem 5.25 (so lambda_m^w = lambda_{phi_v}). Then

    dLoss/dalpha_k = Im<lambda_k^w|G_k|psi_k^w>

for every parametrised gate k of the word, with s_k = +1. Occurrences add (Lemma 5.20).

*Proof.* Theorem 5.14 holds for an arbitrary cotangent seed of its final vector. Here the final vector is phi_v, and its cotangent is lambda_{phi_v} from Theorem 5.25. No word is transposed. QED

**Proposition 5.27 (counts for the tree adjoint)** [addendum (11.31), (11.23''), Sec. 16; T24(b)]. Keep every forward reduced vector rv_w. Run the backward pass in reverse post-order. At node v:

1. regenerate phi_v;
2. form all lambda_{rv_k} into a temporary of size sum_k 2^{q_k}, then let each lambda_{rv_k} replace rv_k (rv_k is no longer needed);
3. replace phi_v by lambda_{phi_v};
4. regenerate phi_v and run Theorem 5.26 with three vectors of length 2^{Q_v}.

The number of complex entries held at once is at most

    sum_w 2^{q_out(w)} + 3 x 2^{Q_max} + max_v sum_{k child of v} 2^{q_k}.

The arithmetic is two word regenerations and one node operation per child per node, plus Proposition 5.16 per word.

*Proof.* Count along the schedule. At node v, the vectors in use are the retained reduced vectors (one per word, in the slot of rv_w or of its cotangent), the three word-length vectors and the temporary of step 2. QED

(The round-off of the backward word sweep is bounded by Theorem 6.9 applied to the list of inverse gates, forward.)

**Remark 5.28 (shift rules on the tree form).** In the tree form, sigma[c] = a_c^T phi_v with phi_v = U(theta)|0>. So N_c = <0|U^dag Ob U|0> with Ob = |conj(a_c)><conj(a_c)|, and Lemma 5.3 and Theorems 5.6 and 5.7 apply unchanged. The shift rules are properties of the function, not of the evaluator.

The claim that the last two gradients of "Alice sleeps and dreams" coincide because, under IQP with L = 1, the sentence depends on t_sleeps + t_dreams only is UNVERIFIED. The numerical agreements quoted in the addendum (T24) are checks, not proofs.

## 5.8 Seeds for other functionals

**Lemma 5.29 (normalisation and projective invariance)** [limits G.9, L.1, L.4, B1-B3; addendum (11.27')].

- (a) If y = x/||x||, then lambda_x = (lambda_y - y Re<y|lambda_y>)/||x||.
- (b) If y = x / sum_i x_i, with x real and sum_i x_i > 0, then lambda_x = (lambda_y - <y, lambda_y> 1)/sum_i x_i.
- (c) If Loss(t x) = Loss(x) for all t > 0, then Re<x|lambda_x> = 0.
- (d) For sigma = sum_k cf_k sig_k: lambda_{cf_k} = <sig_k|lambda_sigma> and lambda_{sig_k} = conj(cf_k) lambda_sigma.
- (e) If p_c = sum_k wt_k p_c^{(k)} with wt = softmax(beta), then dp_c/dbeta_k = wt_k (p_c^{(k)} - p_c).
- (f) If Loss = F(Z) with Z = ||x||^2, then lambda_x = F'(Z) x. In particular, the seed of ln Z is x/Z.

*Proof.* (a) dy = (dx - y Re<y|dx>)/||x||. (b) is the same computation with the real inner product. (c) Differentiate at t = 1. (d) Bilinearity. (e) dwt_k/dbeta_l = wt_k(delta_{kl} - wt_l). (f) Lemma 5.13(c) with Ob = F'(Z) I. QED

**Corollary 5.29' (seeds for Gram, InfoNCE and fidelity losses)** [limits (G.4), (G.7), (G.8), (L.4)]. Let psi_a be unit vectors, G_ab = <psi_a|psi_b> and F_ab = |G_ab|^2, and let Loss be a differentiable function of the real numbers F_ab.

- (a) dF_ab = 2 Re(conj(G_ab) dG_ab) with dG_ab = <dpsi_a|psi_b> + <psi_a|dpsi_b>. Hence each ordered pair (a, b) contributes
      lambda_{psi_a} += (dLoss/dF_ab) conj(G_ab) psi_b,        lambda_{psi_b} += (dLoss/dF_ab) G_ab psi_a      [limits (G.8)].
  Cotangents of the unnormalised sigma_a then follow from Lemma 5.29(a).
- (b) For the InfoNCE loss Loss = -sum_a [ F_{a a+}/T - ln sum_{b != a} exp(F_ab/T) ] [limits (G.4)], with p_ab = exp(F_ab/T) / sum_{b' != a} exp(F_ab'/T):
      dLoss/dF_{a a+} = -1/T + p_{a a+}/T,     dLoss/dF_ab = p_ab/T for every other b != a      [limits (G.7)].
- (c) For Loss = -F with F = |ip|^2/(Z_1 Z_2), ip = <sigma_1|sigma_2> and Z_i = ||sigma_i||^2:
      lambda_{sigma_1} = -( conj(ip) sigma_2/(Z_1 Z_2) - F sigma_1/Z_1 ),     lambda_{sigma_2} = -( ip sigma_1/(Z_1 Z_2) - F sigma_2/Z_2 )      [limits (L.4)].

*Proof.* (a) d|G|^2 = 2 Re(conj(G) dG). By Definition 5.12 we need dLoss = 2 Re<lambda|dpsi>. With lF := dLoss/dF_ab real, Re(lF conj(G_ab) <dpsi_a|psi_b>) = Re(lF G_ab <psi_b|dpsi_a>) = Re<lF conj(G_ab) psi_b | dpsi_a>, and Re(lF conj(G_ab) <psi_a|dpsi_b>) = Re<lF G_ab psi_a | dpsi_b>. Contributions add (Lemma 5.13(b)).

(b) Differentiate: F_{a a+} appears in the first term with coefficient -1/T, and d/dF_ab of ln sum_{b'} exp(F_ab'/T) is p_ab/T for every b != a (including a+).

(c) d|ip|^2 = 2 Re(conj(ip) <dsigma_1|sigma_2>) = 2 Re<conj(ip) sigma_2|dsigma_1> and dZ_1 = 2 Re<sigma_1|dsigma_1>. So dF = 2 Re< conj(ip) sigma_2/(Z_1 Z_2) - F sigma_1/Z_1 | dsigma_1 > + (the same with 1 and 2 exchanged and conj(ip) replaced by ip). Negate for Loss = -F. QED

**Proposition 5.30 (functionals of a reduced density matrix)** [limits SF.7, T.5, NU.6-NU.7; spec Sec. 5.1]. Let rho_Af = Tr_R|psi><psi| and Loss = F(rho_Af), where F is Frechet differentiable at rho_Af with dF = Tr(Lam drho), Lam Hermitian. Then:

- (a) lambda_psi = (Lam (x) I_R) psi.
- (b) If instead Loss = F(rho_n) with rho_n = rho_Af/tr and tr = Tr rho_Af, then Lam_eff = (Lam - Tr(Lam rho_n) I)/tr.
- (c) For rho' = sum_k Kr_k rho Kr_k^dag and P_rd = Tr(Ob rho')/Tr(rho'), the gradient with respect to Kr_k is (Ob - P_rd I) Kr_k rho / Tr(rho').
- (d) Standard cases, by the Daleckii-Krein formula (Appendix C):
  - F = Tr rho^2 gives Lam = 2 rho, with no hypothesis;
  - F = -Tr rho ln rho gives Lam = -(ln rho + I), provided rho > 0 (full rank); the -I drops if Tr drho = 0;
  - F = S(A||B), as a function of A, gives ln A - ln B + I, provided A, B > 0.

At a rank-deficient rho the entropy is not Frechet differentiable. Example: psi(t) = cos(t/2)|00> + sin(t/2)|11> has S = h(sin^2(t/2)), and dS/dp diverges at p = 0, although dS/dt(0) = 0. In that case differentiate directly in theta, or regularise with rho + eps I.

*Proof.* (a) drho = Tr_R(|dpsi><psi| + |psi><dpsi|), so Tr(Lam drho) = 2 Re<(Lam (x) I) psi|dpsi>. (b) drho_n = drho/tr - rho_n Tr(drho)/tr. (c) dP_rd = Tr((Ob - P_rd I) drho')/Tr(rho'). (d) The Daleckii-Krein formula needs g to be C^1 on an open set containing the spectrum. For Tr g(rho) it gives Lam = g'(rho). QED

**Remark 5.31 (shift rules for these functionals).** Every entry rho_Af[i][j] = <psi|(|j><i| (x) I)|psi> is a combination Ob_1 + i Ob_2 of two Hermitian expectations, so Theorems 5.6 and 5.7 apply entrywise.

For fidelities F_ab = abs(<sig_a|sig_b>)^2 / (Z_a Z_b), the shift rules apply to abs(<sig_a|sig_b>)^2, Z_a and Z_b, each applied per occurrence of the angle. They do not apply to the amplitude <sig_a|sig_b>: for G^2 = I it has frequencies +-1/2, where the two-term rule fails. A word shared between a and b gives frequencies up to +-2, so Lemma 5.20 must be used to split it.

Reparametrisations theta = theta(z) compose by the chain rule [addendum (13.12)].

## 5.9 SPSA

**Definition 5.32 (SPSA)** [spec Sec. 5.4]. Take Delta_sp in {-1, +1}^{n_par} with i.i.d. Rademacher entries. The estimator is

    ghat_i = [Loss(theta + c_sp Delta_sp) - Loss(theta - c_sp Delta_sp)]/(2 c_sp Delta_sp,i).

The gain sequences are a_k = a_0/(A_0 + k + 1)^{0.602} and c_sp,k = c_0/(k + 1)^{0.101} (Spall).

**Theorem 5.33 (unbiasedness and exact variance for a linear objective)** [errata E-2, replacing spec l.1051-1052; review 2.2]. Let Loss(theta) = gv . theta + const. Then exactly

    ghat_i = gv_i + sum_{j != i} gv_j Delta_j Delta_i,
    E ghat_i = gv_i,
    Var ghat_i = sum_{j != i} gv_j^2   (independent of c_sp),
    E||ghat - gv||^2 = (n_par - 1)||gv||^2.

If every evaluation carries independent additive noise of mean 0 and variance sigma_eta^2, then Var ghat_i = sum_{j != i} gv_j^2 + sigma_eta^2/(2 c_sp^2).

*Proof.* The difference of the two evaluations is 2 c_sp gv . Delta, and Delta_i^2 = 1. For j, l != i, E[Delta_j Delta_i Delta_l Delta_i] = delta_{jl}. The noise contributes (eta_+ - eta_-)/(2 c_sp Delta_i), which has variance sigma_eta^2/(2 c_sp^2) and is uncorrelated with the first term. QED

**Remark 5.34.** A variance has units [Loss]^2/[theta]^2, while the superseded form (n_par - 1) mean_j gv_j^2 / c_sp^2 has units [Loss]^2/[theta]^4, so that form cannot be right. Enlarging c_sp cannot reduce the intrinsic variance (n_par - 1)||gv||^2. It reduces only the noise term, and it trades that reduction against the bias of Proposition 5.35 (forward). The exact enumeration over 2^8 sign vectors, which gives a variance of 0.466300 at c_sp = 0.01, 0.1 and 1 [review 2.2], was computed on a linear surrogate. "O(n_par) more steps than an exact gradient" is a heuristic reading.

**Proposition 5.35 (SPSA bias)** [errata E-2; review 2.2; spec l.1049]. Let Loss be C^5 near theta, and let f_ijk denote its third partial derivatives. Then

    E ghat_i - gv_i = c_sp^2 Bc_i + O(c_sp^4),       Bc_i = (1/6)(3 sum_{j != i} f_ijj + f_iii).

Minimising Bc_i^2 c_sp^4 + sigma_eta^2/(2 c_sp^2) over c_sp gives c_sp^6 = sigma_eta^2/(4 Bc_i^2). In the single-coordinate case this is c_opt = (3 sigma_eta/abs(f_iii))^{1/3}. With sigma_eta ~ u abs(Loss), it gives the rule c_sp ~ u^{1/3}.

*Proof.* Expand in Taylor series to fifth order. The even orders cancel in the difference, and dividing by 2 c_sp leaves a remainder of O(c_sp^4). E[Delta_j Delta_k Delta_l Delta_i] = 1 iff the four indices pair up. For j != i there are three pairings, each contributing f_ijj; the case where all four indices are equal contributes f_iii. QED

## 5.10 Second-order objects

**Definition 5.36** [limits Sec. 2.6 (3); Stokes, Izaac, Killoran, Carleo, Quantum 4, 269 (2020)]. Let s_n = sigma/sqrt(Z). Define:

- the Fubini-Study metric F_FS[j][k] = Re(<d_j s_n|d_k s_n> - <d_j s_n|s_n><s_n|d_k s_n>);
- the classical Fisher matrix F_cl[j][k] = sum_c (1/p_c)(dp_c/dtheta_j)(dp_c/dtheta_k);
- Hess, the Hessian of Loss.

The quantum natural gradient step is theta := theta - eta_lr (F_FS + lam_r I)^{-1} grad Loss. Levenberg-Marquardt solves (Mm + mu_r I) delta = -grad Loss.

**Proposition 5.37 (Fubini-Study metric from the Jacobian)** [limits Sec. 2.6 (3)(b)].

- (a) 1 - abs(<s_n(theta)|s_n(theta + dtheta)>)^2 = sum_{jk} F_FS[j][k] dtheta_j dtheta_k + O(abs(dtheta)^3).
- (b) F_FS[j][k] = Re((J^dag J)[j][k]/D - conj(v_j) v_k/D^2), where v_j = <sigma|d_j sigma> and D = Z.
- (c) dN_c/dtheta_j = 2 Re(conj(sigma[c]) J_c[j]) and dD/dtheta_j = 2 Re v_j.
- (d) Per sentence, rank F_FS <= 2(2^{q_s} - 1) and rank F_cl <= 2^{q_s} - 1. For a binary readout, F_cl = gp_0 gp_0^T/(p_0 p_1), where gp_c is the gradient of p_c.

*Proof.* (a) Expand abs(<s|s'>)^2/<s'|s'> to second order. On real vectors, the quadratic form of a Hermitian matrix equals the quadratic form of its real part.

(b) Let Pi_s = I - |s_n><s_n|. Then Pi_s d_j s_n = (J_j - sigma v_j/Z)/sqrt(Z).

(c) Differentiate N_c = abs(sigma[c])^2.

(d) F_FS is the real part of the Gram matrix of vectors lying in the complex (2^{q_s} - 1)-dimensional space s_n^perp. The gradients gp_c sum to zero, so they span at most n_cl - 1 dimensions. QED

**Remark 5.38 (batch rank).** Rank is subadditive over a batch of B sentences, so the batch rank is at most 2B(2^{q_s} - 1). This is why the regularisers lam_r and mu_r are needed. F_cl is the Gauss-Newton part of the Hessian of Loss_CE:

    Hess_CE = sum_c (y_c/p_c^2) gp_c gp_c^T - sum_c (y_c/p_c) Hess p_c.

Replacing y by its expectation p in the first term gives F_cl.

**Proposition 5.39 (Hessians)** [limits Sec. 2.6 (3)].

(a) Let K_c[j][k] = d^2 sigma[c]/dtheta_j dtheta_k, and write N_{c,jk} for second derivatives (a comma marks derivatives). Then

- N_{c,jk} = 2 Re(conj(J_c[k]) J_c[j] + conj(sigma[c]) K_c[j][k]);
- D_{,jk} = sum_c N_{c,jk};
- p_{c,jk} = (N_{c,jk} - p_{c,k} D_{,j} - p_{c,j} D_{,k} - p_c D_{,jk})/D.

For Loss = -ln p_y,

    Hess[j][k] = -( N_{y,jk}/N_y - N_{y,j} N_{y,k}/N_y^2 - D_{,jk}/D + D_{,j} D_{,k}/D^2 ).

(b) Forward-over-reverse: K_c[.][p] . xi = sum_{k in K_p} s_k (-i/2)(<dB_k|G_k|psi_k> + <B_k|G_k|dpsi_k>). The tangents satisfy

    dpsi_k = g_k dpsi_{k-1} + s_k xi_{p(k)} Gamma_k psi_k,
    dB_{k-1} = g_k^dag dB_k - s_k xi_{p(k)} g_k^dag Gamma_k B_k.

(c) For angles in the circuit, applying the shift rule twice gives exact second derivatives of N_c and D. For G^2 = I,

    f'' = [f(t + pi) - f(t)]/2,       f_jk = [f(+,+) - f(+,-) - f(-,+) + f(-,-)]/4   (shifts +-pi/2).

*Proof.* (a) Quotient rule twice, and (ln N)_{,jk} = N_{,jk}/N - N_{,j} N_{,k}/N^2.

(b) Differentiate the per-gate identity of Proposition 5.17 in the direction xi, using Lemma 5.2 and Lemma 5.20.

(c) Frequency sets are closed under differentiation, so Theorem 5.5 applies to f' as well. Also, for f = a0 + b0 cos t + d0 sin t, f(t + pi) = a0 - b0 cos t - d0 sin t. QED

## 5.11 The landscape at the origin

**Theorem 5.40 (critical point at the origin for real-generator ansatze)** [errata E-10; review 2.8(E); spec l.1072]. Consider a sentence circuit or word tree such that:

- the gates at theta = 0 are real (H, CNOT, CZ, and I for every rotation at angle 0);
- the generators are real symmetric (Px, Pz, |1><1| (x) Pz, |1><1| (x) Px, and real Pauli strings);
- the readout projectors are real.

Then every first derivative of N_c and of D vanishes at theta = 0. If also D(0) > 0, the same holds for p_c and for Loss (Loss_CE needs p_y(0) > 0).

*Proof.* For one parameter with generator G at gate k, write f(t) = <phi|g_k(t)^dag Ob g_k(t)|phi>, where phi = psi_{k-1} is real and Ob = Vk^dag O_c Vk is real symmetric. By Lemma 5.2, f'(0) = <phi|(Gamma^dag Ob + Ob Gamma)|phi> = (i/2)<phi|(G Ob - Ob G)|phi>. For real symmetric G, Ob and real phi, phi^T Ob G phi = (phi^T Ob G phi)^T = phi^T G Ob phi, so f'(0) = 0.

In the tree form, sigma(0) is real, and each d phi_w = -(i/2) G_k (real vector) is purely imaginary, so d sigma is purely imaginary. Hence dN_c = 2 Re(conj(sigma_c) d sigma_c) = 0. QED

**Theorem 5.41 (the IQP saddle for the (1,1) transitive sentence)** [errata E-10; review 2.8(E); spec Sec. 8]. Let q_n = q_s = 1. Take nouns n_i = Rx(t2) Rz(t1) Rx(t0)|0>, and a verb of ansatz 0 with L = 1: one H layer on the wires (a, s, b), followed by any product of CRz gates on those wires. Read out sigma[s] = sum_{a,b} n_1[a] V[a,s,b] n_2[b] and p_0 = abs(sigma[0])^2 / Z. At theta = 0:

- p_0 = 1/2 and D = 1/4;
- every first derivative, and every diagonal second derivative, of N_c, D and p_0 vanishes;
- for an Rx angle theta of noun i and the angle phi of a CRz gate joining noun i's wire to the s wire (in either direction), d^2 p_0/dtheta dphi = -1/4.

So theta = 0 is a saddle.

*Proof.* At theta = 0 the nouns are e_0 and V is uniform with entries 2^{-3/2}. So sigma[s] = 2^{-3/2}, N_0 = N_1 = 1/8, D = 1/4 and p_0 = 1/2. First derivatives vanish by Theorem 5.40.

Diagonal second derivatives, noun angles. For an Rx angle, dn = -(i/2) e_1 and d^2 n = -(1/4) e_0. For an Rz angle, dn = -(i/2) e_0 and d^2 n = -(1/4) e_0. In both cases d sigma[s] = -(i/2) 2^{-3/2}, because V is uniform, and d^2 sigma = -(1/4) sigma. So N_{s,tt} = 2 (1/4) 2^{-3} - (1/2) 2^{-3} = 0.

Diagonal second derivatives, verb angles. For a CRz angle with generator Gc, write uV := 2^{-3/2} sum_k |k> for the uniform verb vector at theta = 0. Then dV = -(i/2) Gc uV and d^2 V = -(1/4) Gc^2 uV. At the index (0,s,0) this gives the same cancellation, weighted by [bit_ctl = 1]. So D_{,jj} = 0 and p_{0,jj} = 0.

Mixed derivative, with the CRz controlled by s and targeting a:

- d sigma_theta[s] = -(i/2) 2^{-3/2};
- d sigma_phi[s] = -(i/2) s 2^{-3/2};
- d^2 sigma[s] = (1/4) s 2^{-3/2}.

So N_{0,theta phi} = 0, N_{1,theta phi} = 1/8, D_{,theta phi} = 1/8, and p_{0,theta phi} = (0 - (1/2)(1/8))/(1/4) = -1/4.

Mixed derivative, with the CRz controlled by a and targeting s:

- d sigma_phi = 0;
- d^2 sigma[s] = -(1/4)(-1)^s 2^{-3/2};
- N_{s,theta phi} = -(1/16)(-1)^s, D_{,theta phi} = 0, and p_{0,theta phi} = -1/4.

The 2 x 2 block [[0, -1/4], [-1/4, 0]] has eigenvalues +-1/4, so the quadratic form is indefinite. QED

**Remark 5.42 (scope).** The errata state the saddle for "every IQP-ansatz sentence". Theorem 5.40 gives the vanishing gradient in that generality. The vanishing diagonal Hessian holds for the (1,1) shape (Theorem 5.41), but it is FALSE at (2,1): Theorem 7.34(d) (forward) gives d^2 p_0/dpsi^2(0) = 1/8 for a verb CRz angle. The mixed derivative with an Rz noun angle is 0: there dn = -(i/2) e_0, and the two terms cancel. So Rz gradients near the origin are of second order [errata E-10].

**Theorem 5.43 (exact E[D] = 1/4 at sign-symmetric initialisation)** [errata E-10, E-1; review 2.8(E); spec l.970]. Take the setting of Theorem 5.41 with an arbitrary product of CRz gates after the single H layer. Let the three noun angles of each noun and the verb angles be independent, and let each noun angle have a distribution symmetric under negation. Then E[D] = 1/4 = 2^{-|K|} exactly.

*Proof.* Write rho_i = |n_i><n_i| = (1/2)(I + x_i Px + y_i Py + z_i Pz). Then

    D = (1/4) sum_{Pa, Pb in {I, Px, Py, Pz}} r_{1,Pa} r_{2,Pb} T_{Pa Pb},

where r_I = 1 and T_{Pa Pb} = sum_{a a' b b'} Pa[a][a'] Pb[b][b'] sum_s V[a,s,b] conj(V[a',s,b']).

Since abs(V[j]) = 2^{-3/2} for every j and every angle, T_{II} = 1 and T_{Pz I} = T_{I Pz} = T_{Pz Pz} = 0.

The Bloch vector of Rx(t2) Rz(t1) Rx(t0)|0> is

    x = sin t0 sin t1,
    y = -sin t0 cos t1 cos t2 - cos t0 sin t2,
    z = -sin t0 cos t1 sin t2 + cos t0 cos t2.

x is odd in t1, and y is odd under (t0, t2) -> (-t0, -t2). So E[x_i] = E[y_i] = 0.

By independence, E[D] = (1/4) sum E[r_{1,Pa}] E[r_{2,Pb}] E[T_{Pa Pb}]. The terms with Pa or Pb in {Px, Py} vanish because E[x_i] = E[y_i] = 0. Of the remaining terms, only T_{II} = 1 is non-zero. QED

**Remark 5.44 (trainability: proved and conjectured)** [errata E-10; review 2.8(A)-(D), (F); spec l.1072-1074 (superseded)].

- (i) Proved: the bound (1 + p_c)/(2D) of Proposition 5.10; E[Z] = 2^{-2 q_n} for Haar verbs (Theorem 3.32); and Theorem 5.43.
- (ii) Gradient variance under a global 2-design (Theorem 7.31, forward). For a rank-1 projector expectation on W qubits, Var = O(2^{-2W}). This applies to N_c in the bent-wire form with W = |K| + q_s. The unnormalised N_c of the Bell form carries the factor 2^{sum_e q_e}, and there the order is 2^{-W}. In both forms, Var[dN_c]/E[Z]^2 = O(2^{-2 q_s}) (Corollary 7.32, forward).
- (iii) Conjecture (UNVERIFIED): Var_theta[dp_c/dtheta] stays bounded away from 0 as W grows for the normalised readout. Errata E-10 measures 0.06 +- 0.01, flat over W = 3..10. This is consistent with (ii).
- (iv) Empirical, not results of this document: the fraction of sentences with D < 2^{-6}, the gradient-norm tail frequencies, and the frequency of non-global stationary points in teacher-student tests.
- (v) Exact test vector: dN_0/dtheta = 0 exactly for the third angle of the object noun and the second CRz of the verb in the worked example (J[0][7] = 0 in Proposition 5.17).
# Chapter 6. Numerical analysis: rounding, error propagation and fixed point

This chapter bounds the difference between the exact objects of Chapters 2-5 and the objects a finite-precision computation produces, under explicit rounding models. It makes no assumption about any particular device. Where a statement depends on a rounding mode that is not modelled here, it is marked UNVERIFIED.

## 6.1 Rounding models

**Definition 6.1 (floating-point model)** [spec C9, C12]. A format with p significand bits has unit roundoff u = 2^{-p}. The three formats used are fp32 (p = 24), fp16 (p = 11) and bf16 (p = 8). *No-underflow hypothesis (NU):* no operand, product or result has non-zero magnitude below the smallest normal number (2^-14 for fp16), and no overflow occurs. Under (NU), every basic operation op in {+, -, x} satisfies fl(x op y) = (x op y)(1 + dl) with abs(dl) <= u. A fused multiply-add rounds once.

If (NU) is dropped, gradual underflow adds an absolute error of at most half the subnormal spacing per operation. For fp16 this is 2^-25 (Higham 2002, Sec. 2.1, eq. (2.8)).

**Definition 6.2 (fixed-point formats)** [spec Sec. 4.3; errata E-6, Sec. B].

- Q1.15 has values x/2^15 with x in [-32768, 32767], so its range is [-1, 1 - 2^-15] and +1 is not representable.
- Q2.14 has values x/2^14 with range [-2, 2 - 2^-14], so +-1 are representable.
- Q3.29, Q1.31, Q2.30 and Q3.61 are defined analogously.

Rounding to Q1.15 is absolute: abs(rnd_15(y) - y) <= 2^-16. The map sat clamps each component to [-1, 1 - 2^-15]. The pre-scaled reference of an exact object y is S15 y, where S15 = 1 - 2^-15.

**Lemma 6.3 (exact operations).**

- (a) Conversion from fp16 to fp32 is exact.
- (b) Under (NU), the product of two fp16 numbers is exact in fp32.
- (c) int16 x int16 products are exact in int32. A Q2.14 x Q1.15 product is an exact Q3.29 number, and a Q1.15 x Q1.15 product is an exact Q2.30 number.
- (d) A sum of such products is exact in int32 provided every partial sum has magnitude below 2^31.

*Proof.*

(a) fp16 has 11 significand bits and exponents in [-24, 15], including subnormals. fp32 has 24 bits and exponents in [-149, 127]. So every fp16 value is an fp32 value.

(b) The product of two 11-bit significands has at most 22 significant bits. The exponent of its leading bit lies in [-48, 31], because 65504^2 < 2^32. Both fit fp32.

(c) abs(x1 x2) <= 2^30 < 2^31. The scalings are 2^-14 x 2^-15 = 2^-29 and 2^-15 x 2^-15 = 2^-30.

(d) Integer addition is exact unless a partial sum overflows. QED

**Lemma 6.4 (dot-product error; Higham 2002, Sec. 3.1, Lemma 3.1 and eq. (3.5), cited).** Let x, y be in R^n with n u < 1, and assume (NU). Computing the dot product in any order, with fused or separate multiply-adds, gives

    abs(fl(x . y) - x . y) <= gamma_n sum_i abs(x_i y_i) <= gamma_n ||x|| ||y||.

*Proof of the specialisation.* Each term x_i y_i passes through at most n roundings, so fl(x . y) = sum_i x_i y_i (1 + th_i) with abs(th_i) <= (1 + u)^n - 1 <= gamma_n. The second inequality is Cauchy-Schwarz. For n = 4 and u <= 2^-11 this gives gamma_4 <= 4u(1 + 2^-8), and for n = 8, gamma_8 <= 8u(1 + 2^-7). QED

## 6.2 Error of one gate application

**Lemma 6.5 (butterfly row as real dot products)** [spec (4.1)]. Let psi'_a = u_{a0} psi_0 + u_{a1} psi_1, where abs(u_{a0})^2 + abs(u_{a1})^2 = 1. Then Re psi'_a = xr . yv and Im psi'_a = xi . yv, with

    xr = (Re u_{a0}, -Im u_{a0}, Re u_{a1}, -Im u_{a1}),   xi = (Im u_{a0}, Re u_{a0}, Im u_{a1}, Re u_{a1}),
    yv = (Re psi_0, Im psi_0, Re psi_1, Im psi_1).

Here ||xr|| = ||xi|| = 1, and ||yv|| = ||(psi_0, psi_1)||. For any subset Jb of the four positions, abs(sum_{i in Jb} xr_i yv_i) <= ||yv||.

*Proof.* Expand (a + ib)(c + id) = (ac - bd) + i(ad + bc). The subset bound is Cauchy-Schwarz on the subvectors. QED

**Theorem 6.6 (per-gate constant)** [spec (4.2), Sec. 4.6, Sec. 10 item 7]. Assume (NU). Let U be a 2x2 unitary applied as a butterfly to all N/2 pairs. Let hat U = U + dU be its stored entries, and hat psi' the computed result from an exactly stored psi. Then ||hat psi' - U psi|| <= cg u ||psi||, where the constant cg depends on the setting.

- (i) All arithmetic and storage in one format with u <= 2^-11, and the gate rounded in that format: cg = 8(1 + 2^-8) + sqrt2 < 9.45. Pure fp16 gives cg < 10.
- (ii) fp16 storage, fp32 arithmetic and fp32 gate entries, with u = u16: cg = 1 + 9.45 (2^-13 + 2^-24) < 1.0012.
- (iii) As (ii), but with the gate entries rounded to fp16: cg = 1 + sqrt2 + O(2^-11) < 2.42.
- (iv) A fused 4x4 two-qubit block in the setting of (i): cg = 8 sqrt8 (1 + 2^-7) + 2 < 25.

Without (NU), add an absolute term sqrt(2N) kk 2^-25, with kk = 1 for the fp16 store and kk <= 4 for pure-fp16 arithmetic.

*Proof.* The gate term. abs(dU_ab) <= u abs(U_ab), so ||dU||_F <= u ||U||_F = sqrt2 u, since a 2x2 unitary has ||U||_F^2 = 2.

The arithmetic term. By Lemma 6.5 each of the four real outputs of a pair is a 4-term dot product with ||x|| <= 1 + u. Lemma 6.4 bounds its error by gamma_4 (1 + u) ||psi_pair||. Over the four outputs of a pair the error is at most 8u(1 + 2^-8) ||psi_pair||. Summing squares over the disjoint pairs gives 8u(1 + 2^-8) ||psi||.

(i) Add the two terms.

(ii) The fp32 computation errs by at most 9.45 u32 ||psi||, by (i) with u32. The only fp16 rounding is the store, which errs by at most u16 ||computed|| <= u16 (1 + 9.45 u32) ||psi||. The total is u16 (1 + 9.45 x 2^-13 + 9.45 x 2^-24) ||psi||.

(iii) Add the fp16 gate term sqrt2 u16 ||psi||, which also enters the store bound as a factor (1 + sqrt2 u16). This gives 2.4161.

(iv) A 4x4 unitary has ||U||_F = 2. Each of the 8 real outputs of a quadruple is an 8-term dot product. So the arithmetic term is sqrt8 gamma_8 <= 8 sqrt8 (1 + 2^-7) u = 22.8 u, and the total is 24.8 u.

Underflow. Each subnormal result carries an absolute error of at most 2^-25 per component and operation. QED

**Remark 6.7 (fp16 quantisation of an m-qubit block)** [spec Sec. 3.3 (l.728), Sec. 4.6]. For a 2^m x 2^m unitary rounded to fp16, ||dU||_F <= u16 2^{m/2}. For 2^m = 32 this is 2.8e-3, that is, 5.7 fp16 store roundings (the spec says about 6). By Lemma 6.3(b), the products in the block application are exact in fp32 under (NU). Gate entries generated in fp16 arithmetic fall outside Theorem 6.6. bf16 has u_bf16 = 2^-8, so cg u_bf16 >= 3.9e-3 per gate.

## 6.3 Linear error growth under unitary evolution

**Lemma 6.8 (nothing grows)** [spec Sec. 4.1, (4.3)]. If ||psi|| = 1 and every gate is unitary, then after any number of gates abs(psi_i) <= 1. Also ||U e|| = ||e|| for every vector e.

*Proof.* The squared moduli sum to ||psi||^2 = 1. Unitaries are isometries. QED

**Theorem 6.9 (accumulated error)** [spec (4.3)]. Let psi_k = U_k psi_{k-1}, and hat psi_k = fl(hat U_k hat psi_{k-1}), with per-gate constant cg as in Theorem 6.6. Assume hat psi_0 = psi_0 and ||psi_0|| = 1. Then

    ||hat psi_Ng - psi_Ng|| <= (1 + cg u)^Ng - 1 <= Ng cg u exp(Ng cg u).

The same bound holds for the inverse-gate sweep of Proposition 5.16.

*Proof.* Put e_k = hat psi_k - psi_k. Then e_k = U_k e_{k-1} + rr_k, with ||rr_k|| <= cg u ||hat psi_{k-1}|| <= cg u (1 + ||e_{k-1}||). By Lemma 6.8, ||e_k|| <= (1 + cg u)||e_{k-1}|| + cg u. Put a_k = ||e_k|| + 1. Then a_k <= (1 + cg u) a_{k-1} with a_0 = 1, which gives the bound. QED

**Remark 6.10 (random-walk estimates are not bounds)** [spec (4.3), Sec. 4.6, Sec. 10 item 7]. The figures "~2u sqrt(Ng)" and "~0.58 u sqrt(Ng)" assume independent roundings, uniform on [-u, u], added in quadrature. They are heuristics. Only Theorem 6.9 is a bound. Correlated roundings, such as the truncation bias of Remark 6.16 (forward), violate the independence assumption.

**Lemma 6.11 (angle sensitivity; the half-angle turn code)** [spec C5, Sec. 4.2, Sec. 5.6, Sec. 6.3; addendum (13.2); spec T10]. Let P be a Pauli string, so P^2 = I.

- (a) ||R_P(theta) - R_P(theta')||_op = 2 abs(sin((theta - theta')/4)) <= abs(theta - theta')/2.
- (b) Write phi_h = theta/2. The map k -> R_P(4 pi k/65536) is an injective group homomorphism Z/65536 -> U(2^n), because R_P(theta + 2 pi) = -R_P(theta) != R_P(theta). Adding codes modulo 2^16 is therefore adding angles modulo 4 pi. A controlled rotation, whose control-0 block is I and whose control-1 block is R_P, is unchanged by this identification, whereas the shift theta -> theta + 2 pi would change it (Lemma 3.16(ii)).
- (c) Storing k = round(theta 65536/(4 pi)) mod 65536 gives abs(phi_h - 2 pi k/65536) <= pi/65536 = 4.79e-5, so the state error is at most 4.79e-5 per gate. Storing theta in fp16 with round-to-nearest gives a state error of at most ulp(theta)/4. That is 4.9e-4 for theta in [2, 4) and 1.95e-3 for theta in [8, 4 pi). A full-ulp (truncation) error doubles these. The spec's "about 1e-3" near pi is the full-ulp case [spec Sec. 4.2].

*Proof.* (a) R_P(theta) - R_P(theta') = R_P(theta')(R_P(theta - theta') - I), and R_P(a) - I has eigenvalues exp(-+i a/2) - 1, of modulus 2 abs(sin(a/4)).

(b) exp(-i 2 pi P) = I and exp(-i pi P) = -I.

(c) Nearest-integer rounding gives abs(dk) <= 1/2. Round-to-nearest in fp16 gives abs(dtheta) <= ulp/2. Then apply (a). QED

Example [spec Sec. 8, T10]: theta = 0.8 gives k = round(4172.15) = 4172, and phi_h = 0.399985 differs from 0.4 by 1.45e-5.

**Lemma 6.12 (octant reduction; polynomials; table)** [spec Sec. 4.4, C5]. Write k = 8192 oc + rr with oc in {0..7} and rr in {0..8191}. Let rr' = rr if oc is even and 8192 - rr if oc is odd, and put x = (pi/4) rr'/8192, which lies in [0, pi/4]. With (sn, cs) = (sin x, cos x), the pair (sin phi_h, cos phi_h) is, for oc = 0..7:

    (sn, cs), (cs, sn), (cs, -sn), (sn, -cs), (-sn, -cs), (-cs, -sn), (-cs, sn), (-sn, cs).

On [0, pi/4]:

- the degree-7 Taylor polynomial of sin has error at most (pi/4)^9/9! = 3.1e-7;
- the degree-8 Taylor polynomial of cos has error at most (pi/4)^10/10! = 2.5e-8;
- the degree-5 sine polynomial can err by (pi/4)^7/7! = 3.7e-5 > 2^-15.

Now take a 257-entry Q2.14 table T[jj] = floor(16384 sin(jj pi/512) + 1/2). For a 14-bit quarter index qx, put jj = qx >> 6 and fr = qx & 63, and interpolate as T[jj] + ((T[jj+1] - T[jj]) fr + 32) >> 6. The total error is at most 2^-15 + 2^-15 + (pi/512)^2/8 = 6.57e-5 < 2^-13.89.

Exhaustive evaluation over all 16384 quarter codes gives the sharper maximum 6.03e-5 < 2^-14 = 6.10e-5, attained at qx = 6722. That is a finite check (a proof by enumeration, not reproduced here).

*Proof.* phi_h = (pi/4)(oc + rr/8192). The eight cases are the reflection identities of sin and cos. On [0, pi/4] the Taylor series are alternating with decreasing terms, so each remainder is bounded by the first omitted term.

For the table:

- linear interpolation errs by at most h^2 max abs(sin'')/8 with h = pi/512;
- each table entry errs by at most 2^-15, and the interpolated value is a convex combination of two entries, so it inherits at most 2^-15;
- the final rounding (+32) >> 6 errs by at most half a Q2.14 unit, that is 2^-15. QED

## 6.4 Fixed-point safety from unitarity

**Theorem 6.13 (intermediates are bounded; overflow is impossible)** [spec (4.1), (4.5), Sec. 4.3; errata E-6, Sec. B]. Let a 2x2 gate have entries rounded to Q2.14, so hat U = U + dU with abs(dU_ab) <= 2^-15. Then ||dU||_F <= sqrt8 2^-15, and for rotations ||dU||_op <= sqrt2 2^-15. Apply the gate to a Q1.15 state hat psi with ||hat psi|| <= 1 + eta and eta < 0.41. Then:

- (a) every product is an exact Q3.29 int32 (Lemma 6.3(c));
- (b) every partial sum of the 4 products of one real output (8 products for a fused 4x4 block) is exact and has value below sqrt2. In Q3.29 that is below 2^29.5, which leaves at least 1.5 bits of headroom in int32. The magnitude-only bounds sqrt2 (2x2) and 2 (4x4), without Cauchy-Schwarz, leave 1.5 and 1 bits;
- (c) the store x' = sat((acc + 2^13) >> 14) rounds to nearest (ties up), with absolute error at most 2^-16. The store saturates only when the rounded computed value exceeds 1 - 2^-15. That can happen: the Q2.14 row of code 71 is (16384, 112), whose norm exceeds 16384, and applied to the Q1.15 pair (32766, 221), whose norm is at most S15, it gives (acc + 2^13) >> 14 = 32768. By Theorem 6.14 (forward), saturation never increases the distance to the pre-scaled reference.

*Proof.* (b) By Lemma 6.5 applied to the perturbed row hat x, with ||hat x|| <= 1 + sqrt(r_p) 2^-15 for r_p = 4 (2x2) or 8 (4x4), every partial sum is at most ||hat x|| ||yv|| < (1 + 8.7e-5)(1 + eta) < sqrt2.

(c) (acc + 2^13) >> 14 = floor((acc + 2^13)/2^14), which is the nearest integer to acc/2^14 with ties rounded up. The example is a direct evaluation. QED

**Theorem 6.14 (saturation is non-expansive; pre-scale invariance)** [errata E-6; spec Sec. 4.3, Sec. 4.5, l.866; review 2.4]. Let y be the exact pre-rounding vector of one store, and let tt = S15^j x (j >= 1) be the pre-scaled reference. Here x = U psi on the circuit path, or x is the exact tensordot intermediate after j words (Section 6.5); in both cases ||x|| <= 1, for the circuit path because U is unitary and psi is a unit vector, and for the tensordot path because ||T Phi||_F <= ||T||_F ||Phi||_F for each contraction of unit word tensors (Cauchy-Schwarz; the same argument is Theorem 6.20, forward). Then tt lies in the box Bx = [-1, 1 - 2^-15]^mm, and

    ||sat(rnd_15(y)) - tt|| <= ||rnd_15(y) - tt|| <= ||y - tt|| + sqrt(mm) 2^-16,

where mm is the number of real components. Every readout ratio p_c = N_c/D is invariant under the global scale S15^j.

*Proof.* Since j >= 1, every real component of tt lies in [-S15, S15], which is contained in [-1, 1 - 2^-15]; so tt is in Bx. Bx is closed and convex. The Euclidean projection onto a closed convex set is non-expansive (by the variational inequality), and it fixes tt. sat is that projection, because Bx is a product of intervals. The rounding term comes from Definition 6.2. Scaling by S15^j multiplies N_c and D by S15^{2j}, which cancels in the ratio. QED

For j = 0 (no pre-scale) tt need not lie in Bx, since a component equal to +1 lies outside it; the narrowing error is then bounded by Lemma 6.22 (forward) instead, with the extra saturation term.

**Remark 6.15 (the S15 initialisation)** [spec l.801-803, l.865-867; errata E-6; review 2.4]. Initialising |0...0> as the Q1.15 value S15 is the circuit-path pre-scale. The relative error of "3e-5" in the spec is S15 - 1 itself, and it cancels in every ratio. Saturation adds no error beyond the bound of Theorem 6.14.

**Remark 6.16 (truncation bias; a heuristic)** [spec (4.5)]. Replacing (acc + 2^13) >> 14 by acc >> 14 (floor) gives an error in (-2^-15, 0]. If the fractional parts are uniformly distributed, the mean error is -2^-16. These biases add coherently only when the later gates are (near-)identity or diagonal. In general the drift is sum_k U_Ng ... U_{k+1} bias_k. The only rigorous statement is the worst case, Ng sqrt(2N) 2^-15, which follows from Theorem 6.9.

**Proposition 6.17 (per-gate fixed-point error)** [spec (4.6), Sec. 4.3, T8]. On the Q1.15 path with 2N real components:

- the store error is ||e_rnd|| <= sqrt(2N) 2^-16 = 2^{W/2 - 15.5};
- the gate error is ||dU psi|| <= ||dU||_op, which is at most sqrt2 2^-15 = 4.3e-5 for rotations, sqrt8 2^-15 = 8.6e-5 for general 2x2 gates, and sqrt32 2^-15 = 1.7e-4 for 4x4 gates.

So g_Q15(W) = 2^{W/2 - 15.5} + 8.6e-5. This is 4.3e-4 at W = 8, 7.8e-4 at W = 10, 1.5e-3 at W = 12 and 5.6e-3 at W = 16. A fused 4x4 block incurs one store, so it counts as one gate.

For Q1.31 (gate entries in Q2.30, products exact in int64, store (acc + 2^29) >> 30): g_Q31(W) = 2^{W/2 - 31.5} + 1.3e-9.

The Ng-gate error with respect to the pre-scaled reference satisfies Theorem 6.9 with cg u replaced by g_Q15. The norm drift from gate quantisation is at most ||dU||_op per gate. The store adds up to sqrt(2N) 2^-16 per gate.

*Proof.* Each stored real rounds by at most 2^-16 (Theorem 6.13(c)). For rotations, dU = dco I - i dsi Pa (Ry: real antisymmetric part; Rz: diagonal), so ||dU||_op = sqrt(dco^2 + dsi^2) <= sqrt2 2^-15.

The Q1.31 accumulator: by Lemma 6.5, every partial sum is below sqrt2 x 2^61 < 2^63.

Recurrence: put e_k = ||hat psi_k - S15 psi_k||. By Theorem 6.14,

    e_k <= ||(U + dU) hat psi_{k-1} - U S15 psi_{k-1}|| + sqrt(2N) 2^-16 <= (1 + ||dU||) e_{k-1} + g_Q15,

and the argument of Theorem 6.9 finishes. QED

**Remark 6.18 (format comparison)** [spec Sec. 4.3, (4.6), (4.2)]. Q1.15 error is absolute and grows like sqrt N. fp16 error is relative and does not depend on W. In 2-norm:

- g_Q15(W) < 10 u16 iff W <= 15;
- g_Q15(W) < u16 iff W <= 8 (g_Q15(8) = 4.3e-4 and g_Q15(9) = 5.7e-4).

The spec's "equal at W = 9" is this crossover.

**Remark 6.19 (flush-to-zero; UNVERIFIED rounding mode)** [spec Sec. 4.2 (l.796), Sec. 10 item 7]. Consider a rounding model that flushes results below 2^-14 to zero. Each of the 4 products of a real output loses less than 2^-14, and so does the output store, so a gate adds at most sqrt(2N) x 5 x 2^-14. That is 1.4e-2 at W = 10, or 1.1e-2 counting the products alone. The spec's value of 2.8e-3 at W = 10 corresponds to one flushed term per component and is inconsistent with its own four-product formula. Under gradual underflow the term is instead at most 2^-25 per subnormal result (Definition 6.1). Q1.15 has no such mode.

## 6.5 The fused tensordot: range and propagation

**Theorem 6.20 (range of every intermediate)** [spec (4.8), Sec. 1.3, Sec. 6.2; errata E-6]. Form sigma from unit word tensors by successive contractions R = T Phi_w, with T matricised as restT x cup and the word as cup x restW. Then every intermediate T, and sigma itself, satisfies ||T||_F <= 1. So every real component has modulus at most 1, and Z <= 1. With pre-scaled words S15 phi_w, the bound after j words is S15^j.

*Proof.* Induct on the number of words. For each output entry, Cauchy-Schwarz over k gives abs(R[rT, rW])^2 <= (sum_k abs(T[rT,k])^2)(sum_k abs(Phi[k,rW])^2). Summing over entries gives ||T Phi||_F <= ||T||_F ||Phi||_F. QED

**Corollary 6.21 (accumulator bound)** [spec (4.8), (4.5), Sec. 4.7; review 2.4]. For one output entry, the int32 accumulator of 2^q exact Q2.30 products of stored Q1.15 numbers satisfies

    abs(acc) <= ||hat T row|| ||hat Phi col|| 2^30 <= (1 + 2^{(q+1)/2 - 16})^2 2^30 < 2^31     for q <= 28,

and the same holds for the imaginary part. The store (acc + 2^14) >> 15 errs by at most 2^-16.

*Proof.* abs(x_re y_re - x_im y_im) <= abs(x_k) abs(y_k). Then apply Cauchy-Schwarz. Each stored vector has norm at most 1 plus its rounding error, which is at most sqrt(2^{q+1}) 2^-16. QED

**Lemma 6.22 (narrowing without pre-scale)** [errata E-6; spec l.930; review 2.4]. Let a unit-norm object with n complex (2n real) components be narrowed to Q1.15 by rounding and saturation. At most one real component lies in (1 - 2^-16, 1]. Such a component saturates, with error up to 2^-15. So the narrowing error is at most sqrt(2n + 3) 2^-16. With the pre-scale S15, the error is at most sqrt(2n) 2^-16.

Example: the unit vector (sqrt(1 - 3 x 2^-32), 2^-16, 2^-16, 2^-16) narrows with error 2.645734 x 2^-16. As the first component tends to 1 this tends to sqrt7 x 2^-16 = 2.645751 x 2^-16, so the constant sqrt(2n + 3) is sharp but not attained.

*Proof.* Two components above 1 - 2^-16 would give norm^2 > 2(1 - 2^-16)^2 > 1. A component in that interval maps to 2^15 x_i > 32767.5, which rounds to 32768 and is clamped to 32767, with error at most 2 x 2^-16. Hence the error is at most sqrt((2n - 1) + 4) 2^-16. With the pre-scale no component saturates (Theorem 6.13(c)). In the example each small component (a tie at 2^15 x 2^-16 = 1/2) rounds up to 2^-15, an error of exactly 2^-16. The first component x_0 = 1 - ep with ep = 1 - sqrt(1 - 3 x 2^-32), about 1.5 x 2^-32, saturates to 1 - 2^-15, with error 2^-15 - ep < 2^-15. So the squared error is (2 - 2^16 ep)^2 2^-32 + 3 x 2^-32 < 7 x 2^-32, and it tends to 7 x 2^-32 as ep -> 0. QED

**Theorem 6.23 (Lipschitz recurrence for contraction chains)** [spec (4.9), (4.8); errata E-11; addendum (12.16); review 2.9]. Let the exact chain be T_{k+1} = T_k Phi_k (matricised) with T_1 = phi_{w_1}. Let the computed chain be hat T_{k+1} = rnd(hat T_k hat Phi_k), with stored factors hat Phi_k = Phi_k + dPhi_k and store errors eta_k = ||rnd(hat T_k hat Phi_k) - hat T_k hat Phi_k||_F. Put e_k = ||hat T_k - T_k||_F and Lk = ||hat Phi_k||_op. Then

    e_{k+1} <= Lk e_k + ||T_k||_F ||dPhi_k||_F + eta_k.

If every ||Phi_k||_F <= 1 (unit words, so ||T_k||_F <= 1 by Theorem 6.20), then Lk <= 1 + ||dPhi_k||_F and

    ||d sigma|| <= (sum_w ||dPhi_w||_F + sum_steps eta_step) prod_k (1 + ||dPhi_k||_F).

*Proof.* hat T_k hat Phi_k - T_k Phi_k = (hat T_k - T_k) hat Phi_k + T_k dPhi_k. Use ||A B||_F <= ||A||_F ||B||_op for the first term and ||A B||_F <= ||A||_F ||B||_F for the second. Unrolling e_{k+1} <= (1 + a_k) e_k + b_k gives e_n <= (sum b_k) prod(1 + a_k). QED

**Remark 6.24 (non-unit factors)** [review 2.9, 5(e); errata E-11; addendum (12.16)]. If ||Phi_k||_F = sg > 1, the gain can reach sg^k. For sg = 1.5 and k = 6 the gain is 11.4, which needs ceil(log2 11.4) = 4 guard bits. Theorems 6.20-6.21 then do not apply. Constraining ||A||_op <= 1, ||C||_op <= 1 and ||B||_F <= 1 [addendum (12.16)] restores Lk <= 1 via ||A B||_F <= ||A||_F ||B||_op, provided the first factor has ||T_1||_F <= 1. The observed Lk in 0.54-0.85 for Haar (2,1) verbs is numerical, not a theorem. For a non-linear recurrence, a statement of this type needs a Lipschitz constant, and none is claimed here.

**Corollary 6.25 (tensordot error budgets)** [spec Sec. 4.7, l.930-953, T6; errata E-6]. On the Q1.15 path with pre-scale, a word with Q_w qubits has ||dPhi_w|| <= 2^{Q_w/2 - 15.5}, and a stored intermediate with #R complex entries has eta <= sqrt(2 #R) 2^-16. For N TV N:

- at (1,1): 2 x 2^-15 + 2^-14 + sqrt8 2^-16 + sqrt4 2^-16 = 1.96e-4;
- at (2,1): 2 x 2^-14.5 + 2^-13 + sqrt16 2^-16 + sqrt4 2^-16 = 3.0e-4.

Without the pre-scale these become 2.38e-4 and 3.33e-4 (Lemma 6.22).

On the fp16-store path, each stored object errs by at most u16, so 3 words and 2 stores give ||d sigma|| <= 5 u16 = 2.44e-3, independently of W.

The "typical" figures (for example 0.58 u16 sqrt5) are quadrature heuristics (Remark 6.10).

*Proof.* Combine Theorem 6.23 with Lemma 6.22 and Corollary 6.21, and do the arithmetic. QED

## 6.6 Probabilities, postselection and the tight amplification constant

**Theorem 6.26 (probability of a projector)** [spec (4.4), Sec. 4.5]. For any orthogonal projector Pr, unit psi and hat psi = psi + delta with err = ||delta||,

    abs(<hat psi|Pr|hat psi> - <psi|Pr|psi>) <= 2 err + err^2.

*Proof.* The difference equals 2 Re<delta|Pr|psi> + <delta|Pr|delta>. QED

**Theorem 6.27 (postselection amplification; tight constant)** [spec (4.7), (4.9), l.856, l.926; errata E-11; review 2.9; spec T8]. Let Pi be the postselection projector, psi a unit vector with p_surv = ||Pi psi||^2 > 0, and psi_post = Pi psi/sqrt(p_surv). Let hat psi = psi + delta with err = ||delta||, and let p_c = <psi_post|Pr|psi_post> for a projector Pr.

- (a) If Pi hat psi != 0 (automatic when err < sqrt(p_surv)) and err < 2 sqrt(p_surv), then ||hat psi_post - psi_post|| <= 2 err/(2 sqrt(p_surv) - err), and abs(dp_c) is at most the same quantity.
- (b) (First order, tensordot path, with Z in place of p_surv.) dp_c = (2/Z) Re<vv, delta>, where vv = sigma_c e_c - p_c sigma and ||vv||^2 = Z p_c(1 - p_c). Hence

      abs(dp_c) <= 2 sqrt(p_c(1 - p_c)) err/sqrt(Z) + O(err^2) <= err/sqrt(Z) + O(err^2),

  and equality holds at p_c = 1/2 with delta parallel to vv.
- (c) (Runtime certificate.) Let D = ||Pi hat psi||^2 be the computed survival. Then abs(sqrt D - sqrt(p_surv)) <= err, and abs(dp_c) <= 2 err/(2 sqrt D - err) whenever sqrt D > err, which also certifies Pi psi != 0. If the computed survival is hat D = D(1 + dl) with abs(dl) <= gd := (1 + u32)^dd - 1 (Lemma 6.33(b), forward), use hat D/(1 + gd) in place of D; this is a certified lower bound on D.
- (d) The constant 4 of the spec is valid but exactly 4 times too large. It comes from a factor 2 in 2||x - y||/||x|| (Lemma 3.27) times a factor 2 in 2||a - b|| (Lemma 3.28).

*Proof.* (a) Apply Lemma 3.27 to x = Pi psi and y = Pi hat psi. Then ||x - y|| <= err and ||y|| >= sqrt(p_surv) - err. Then apply Lemma 3.28.

(b) Differentiate p_c = abs(sigma_c)^2/Z. The identity ||vv||^2 = abs(sigma_c)^2 - 2 p_c abs(sigma_c)^2 + p_c^2 Z = Z p_c(1 - p_c) is direct. Then use Cauchy-Schwarz and 2 sqrt(p(1 - p)) <= 1. After absorbing a phase term i Im<u|dx> u, which does not change abs(<c|.>)^2, the tangent part is orthogonal to the state.

(c) By the reverse triangle inequality, ||x|| >= sqrt D - err. Apply Lemma 3.27 with ||x|| + ||y|| >= 2 sqrt D - err.

(d) (b) is attained, so no constant below 1 is possible. QED

**Corollary 6.28 (survival floors; amplification factor)** [spec Sec. 4.5 rule (3), Sec. 4.6, l.935-952, Sec. 6.2, T8; errata E-11; review 2.9].

(i) 2 err/(2 sqrt D - err) <= tol holds iff D >= p_min := (err/tol + err/2)^2. With tol = 0.05:

- err = 6.96e-3 gives p_min = 0.020, against the spec's 0.31;
- err = 2.80e-3 gives p_min = 0.0033, against the spec's 0.05.

These values of err are the spec's quadrature estimates (Remark 6.10), not bounds. With the rigorous Theorem 6.9 bound for 50 pure-fp16 gates, err <= (1 + 9.45 x 2^-11)^50 - 1 = 0.259 (first-order value 50 x 9.45 x 2^-11 = 0.231). Since 2 err/(2 - err) > 0.05 already at D = 1, no floor D <= 1 certifies tol = 0.05.

(ii) The output-probability error is E_out <= A_amp err. For a ratio readout p_c = N_c/D with no postselection, A_amp = 1 (Theorem 6.27 with Pi = I). For a raw projector expectation, A_amp = 2 (Theorem 6.26). With postselection, to first order, A_amp = 1/sqrt(p_surv), against the spec's 4/sqrt(p_surv); at p_surv = 0.1 that is 3.16 against 12.65.

(iii) First-order values from Corollary 6.25:

- Q1.15 at (1,1): 1.96e-4/sqrt(0.1403) = 5.2e-4 (spec: 2.1e-3);
- the same at Z = 2^-6: 1.57e-3 (spec: 6.3e-3);
- (2,1) at Z = 2^-6: 2.4e-3 (spec: 9.6e-3);
- circuit-form Q1.15 with Ng = 7 and g_Q15(3) = 1.47e-4: 2.8e-3 (spec: 1.1e-2);
- the fp16-store path at D = 2^-6, with err = 5 x 2^-11: the certificate of Theorem 6.27(c) is 0.0197, which passes tol = 0.05. The tight floor is 2^-8.6.

*Proof.* (i) is algebra on Theorem 6.27(c). (ii) combines Theorems 6.9, 6.26 and 6.27. (iii) is arithmetic. QED

**Remark 6.29 (amplification heuristics)** [spec Sec. 2.3, Sec. 4.5, l.858; addendum (13.6), (13.7), l.708, l.788; errata E-1, E-11]. By Theorem 6.27(b), the first-order amplification is 1/sqrt(Z). Replacing Z by E[Z] = 2^{-2 q_n} (Theorem 3.32) gives 2^{q_n}. This matches the spec's l.858 heuristic 2^{m/2} with m = 2 q_n. It corrects the addendum's 2^{q_n + 1} (l.708, l.788), and also the value 2^{q_n + 2} that errata E-1's consequence text gives for spec l.858. Since Z -> Z^{-1/2} is convex, Jensen's inequality gives E[Z^{-1/2}] >= 2^{q_n}. So 2^{q_n} is a lower bound on the mean amplification. "Each halving of p_surv costs half a bit" is the identity 1/sqrt(p/2) = sqrt2/sqrt(p). The multi-sentence statement that the amplification is 1/sqrt(prod_t p_t) is a heuristic (UNVERIFIED).

**Corollary 6.30 (truncations and gates)** [addendum (14.12); spec Sec. 4.1]. Interleave gates, each with per-step error at most g_gate relative to the norm of its input, with Schmidt truncations that discard at step k a relative norm eps_k. Then

    ||hat Psi - Psi|| <= (1 + g_gate)^Ng prod_k (1 + eps_k) - 1 = Ng g_gate + sum_k eps_k + O((Ng g_gate + sum eps_k)^2).

*Proof.* Gate steps satisfy e_k <= (1 + g_gate) e_{k-1} + g_gate, as in Theorem 6.9. The exact state is not truncated, so a truncation step Trn_k gives ||Trn_k(hat y) - x|| <= ||Trn_k(hat y) - hat y|| + ||hat y - x|| <= eps_k(1 + e_{k-1}) + e_{k-1}. In both cases a_k = e_k + 1 is multiplied by at most (1 + g_gate) or (1 + eps_k). QED

## 6.7 Accumulating sums of squares

**Lemma 6.31 (the RNE absorption rule)** [errata E-8, replacing spec C12 l.113; review 2.5]. Let s be a normal fp16 number in [2^ee, 2^{ee+1}) with -14 <= ee <= 15, so ulp(s) = 2^{ee - 10}, and let t > 0. Then:

- fl(s + t) = s iff t < 2^{ee - 11}, or t = 2^{ee - 11} exactly and the last significand bit of s is even;
- t <= 2^-12 s is sufficient;
- the criterion "t below 2^-11 of the running sum" is not sufficient. For s = 1.5 and t = 2^-11 (1 + 2^-10), t/s = 0.67 x 2^-11, yet fl(s + t) = 1.5 + 2^-10.

*Proof.* For 0 < t < ulp(s), s + t lies strictly between s and s + ulp. Round-to-nearest-even returns s iff t < ulp/2; on a tie it returns the neighbour with an even last bit. Since s < 2^{ee+1}, 2^-12 s < 2^{ee - 11}. QED

**Proposition 6.32 (fp16 constant series)** [errata E-7, replacing spec T8 l.1281-1282; spec l.850; review 2.5].

- (a) Every partial sum k 2^-11 with k <= 2048 is an fp16 number, and sum(2048) = 1. The next addition, 1 + 2^-11, is a half-ulp tie that resolves to 1. So sum(k) = 1 for all k >= 2048, against exact values 1 + 2^-11, 2 and 4 at k = 2049, 4096 and 8192.
- (b) The series of 2^-12 terms stalls at 0.5 = sum(2048), and gives 0.5 against the exact 1 at k = 4096.
- (c) Both series are exact in fp32, because every partial sum has at most 24 significant bits.

*Proof.* Lemma 6.31 with ee = 0 and ee = -1. QED

**Lemma 6.33 (reduction error; exact squares).**

- (a) For fp16 amplitudes widened to fp32, re^2 and im^2 are exact under (NU), and re^2 + im^2 is rounded once.
- (b) Sum n non-negative terms as L_p sequential partial sums of n/L_p terms each, followed by a binary tree of depth log2 L_p. The relative error is at most (1 + u32)^dd - 1, with dd = n/L_p - 1 + log2 L_p.
- (c) On the Q1.15 path, the squares are exact Q2.30 integers, and their sum is exact in int32 whenever ||hat psi||^2 < 2.

*Proof.* (b) With non-negative summands there is no cancellation, and each term passes through at most dd additions. (c) The sum of the squared codes is ||hat psi||^2 2^30 < 2^31. QED

**Remark 6.34 (why probabilities are not accumulated in fp16)** [spec C12, Sec. 4.5; limits precision rules]. For a normalised state, abs(psi_i)^2 ~ 2^-W is fp16-subnormal from W = 15 on. A sequential fp16 sum of 2^12 equal terms loses 50 % (Proposition 6.32(b)). For n non-negative terms, each rounding loses at most 2^-11 times the final computed sum hat s. So the relative loss with respect to the exact sum is at most (n - 1) 2^-11/(1 + (n - 1) 2^-11). For n = 1024 this is 1/3, and it is attained by 0.5 followed by 1023 terms of 2^-12. Hence N_c, D, norms and Gram entries are accumulated in fp32 or int32 (Lemma 6.33).

## 6.8 Integer angle codes

**Proposition 6.35 (integer angle codes)** [spec Sec. 6.3, C5, T10; addendum (13.2) l.891-894, (13.12), T19].

- (i) k = round(theta 65536/(4 pi)) mod 2^16, computed from the unwrapped angle, is the code of Lemma 6.11 and decodes with abs(dphi_h) <= 4.79e-5.
- (ii) Let aa be an integer with abs(aa) < 2^32. Let Cstar = sE sW 65536/(4 pi) > 0 be real, where sE and sW are fixed positive reals, and let Cc = round(Cstar 2^32). Assume abs(aa) Cc < 2^63, so that the product is exact in int64. Then k' = floor(aa Cc / 2^32) mod 2^16 satisfies abs(k' - round(aa Cstar)) <= 1 (mod 2^16).
- (iii) The Q1.15 pair (rnd(32767 cos phi_h), rnd(32767 sin phi_h)) has norm S15 up to sqrt2 2^-16. A component x rounds to 0 iff 32767 x lies in [-1/2, 1/2) (ties up), that is, essentially iff its modulus is below 1/(2 x 32767) = 1/65534 (about 1.526e-5); for example x = 2^-16 gives 32767/65536 = 0.49998, which rounds to 0. A Q2.14 (cos, sin) pair has norm 1 up to sqrt2 2^-15.
- (iv) Whatever the integer k, R_P(4 pi k/65536) is exactly unitary.

*Proof.* (i) theta/(4 pi) = phi_h/(2 pi), and Lemma 6.11(b) makes the modulus exact.

(ii) aa Cc / 2^32 = aa Cstar + dl with abs(dl) <= abs(aa)/2^33 < 1/2. So floor(aa Cstar + dl) lies in {round(aa Cstar) - 1, round(aa Cstar)}.

(iii) Definition 6.2, applied componentwise.

(iv) R_P(t) is unitary for every real t. QED

**Proposition 6.36 (perturbation of a bilinear map)** [addendum l.1065-1067]. Let th = Wm x, with perturbations Wm + dW and x + dx. Then

    abs(dth) <= ||dW|| ||x|| + ||Wm|| ||dx|| + ||dW|| ||dx||.

The addendum's bound omits the last, second-order term, and is false as stated: take Wm = x = dW = dx = 1, so that dth = 3 > 2.

*Proof.* dth = dW x + Wm dx + dW dx. Apply the triangle inequality and the definition of the operator norm. QED

**Remark 6.37 (statements verified numerically, not used here)** [addendum (13.4), (13.4'), (14.10), (14.13), (14.14), T15, T19; limits SF.1-SF.3, B9]. Algorithms verified only numerically in the engineering documents are not used as hypotheses of any result of this document, and their accuracy claims are UNVERIFIED here.
# Chapter 7. Capacity: dimension counting and the function class

This chapter asks two questions: which sentence functions the model can realise, and how many real parameters that takes.

## 7.1 Manifolds and parameter maps

**Definition 7.1 (pure-state manifolds).** The pure states of Q qubits are the points of CP^{d_Q - 1}, where d_Q = 2^Q. The real pure states are the points of RP^{d_Q - 1}. The quotient map is pi_proj: unit sphere -> CP^{d_Q - 1}.

**Theorem 7.2** [addendum (12.1), (12.2)]. dim_R CP^{d_Q - 1} = 2 d_Q - 2 = 2^{Q+1} - 2, and dim_R RP^{d_Q - 1} = d_Q - 1.

*Proof.* The charts Ch_j = {[z] : z_j != 0}, with maps [z] -> (z_i/z_j)_{i != j}, are bijections onto C^{d_Q - 1}, and their transition maps are holomorphic. The real case uses the same charts with real coordinates. QED

**Definition 7.3 (realised degrees of freedom)** [addendum (12.5)]. Let f_w: R^{P_w} -> S(d_Q) be the parameter map theta -> U_w(theta)|0>, and put phi = f_w(theta). The horizontal space at phi is Hor_phi = {v : <phi|v> = 0}, of real dimension 2 d_Q - 2. Let Pp be the real-orthogonal projector onto Hor_phi. Then

    rk_w(theta) := rank_R(Pp df_w/dtheta).

**Lemma 7.4.** rk_w(theta) = rank d(pi_proj o f_w)(theta) <= min(P_w, 2^{Q_w+1} - 2).

*Proof.* Differentiating ||f_w||^2 = 1 gives Re<phi|df> = 0, so df lies in Hor_phi (+) span_R{i phi}. The differential d pi_proj kills the vertical direction i phi and is injective on Hor_phi. QED

**Proposition 7.5 (parameter counts)** [spec C10, Sec. 5.5, Sec. 7]. Under Definition 3.17:

- Q = 1 words have P_w = 3 (Ry-only: 1);
- IQP (ansatz 0 or 1) has P_w = L(Q - 1);
- Sim15 has 2QL;
- Sim14 has 4QL;
- Ry-only has Q.

Example: a vocabulary of 1000 words at (1,1) with L = 1 (600 nouns with Q = 1, 250 transitive verbs with Q = 3, 100 adjectives and 50 intransitive verbs with Q = 2) has 6000 angles under Sim14 and 2450 under IQP.

*Proof.* Count the angle slots of the gate lists. QED

**Theorem 7.6 (necessary depth)** [addendum (12.3)]. Let g: R^P -> Xm be a smooth map into a manifold of dimension nx > P. Then g(R^P) has measure zero, and in particular g is not surjective. So pi_proj o f_w can cover its target only if P_w >= dim target. This gives

    IQP:    L >= (2^{Q+1} - 2)/(Q - 1),
    Sim14:  L >= (2^{Q+1} - 2)/(4Q),
    Sim15:  L >= (2^Q - 1)/(2Q)      (the target is RP^{2^Q - 1}, because Sim15 words are real for Q >= 2 by Lemma 3.18(v)).

The ceilings for Q = 2..14, 16, 18 are:

- Sim15 and Sim14: 1, 2, 2, 4, 6, 10, 16, 29, 52, 94, 171, 316, 586, 2048, 7282;
- IQP: 6, 7, 10, 16, 26, 43, 73, 128, 228, 410, 745, 1366, 2521, 8738, 30841.

*Proof.* By Sard's theorem (Appendix C), the set of critical values has measure zero. When P < nx every point is critical, because rank dg <= P < nx. QED

**Corollary 7.7 (comparison of the depth bounds)** [addendum (12.3)].

- (a) The Sim14 and Sim15 bounds coincide, because (2^{Q+1} - 2)/(4Q) = (2^Q - 1)/(2Q).
- (b) The ratio of the IQP bound to the Sim14 bound is 4Q/(Q - 1), which tends to 4.
- (c) For Q >= 2, Ry-only never covers RP^{2^Q - 1}, because Q < 2^Q - 1.

*Proof.* (a) (2^{Q+1} - 2)/(4Q) = 2(2^Q - 1)/(4Q) = (2^Q - 1)/(2Q).

(b) By Theorem 7.6, [(2^{Q+1} - 2)/(Q - 1)] / [(2^{Q+1} - 2)/(4Q)] = 4Q/(Q - 1), which tends to 4 as Q -> infinity.

(c) Ry-only has P_w = Q (Definition 3.17; it has no layers). For Q >= 2 its words are real (Lemma 3.18(v)), so pi_proj o f_w maps into RP^{2^Q - 1}, of dimension 2^Q - 1 (Theorem 7.2). Also 2^Q - 1 - Q >= 1 for Q >= 2: this holds at Q = 2, and 2^{Q+1} - 1 - (Q+1) = 2(2^Q - 1 - Q) + Q. So P_w < 2^Q - 1, and Theorem 7.6 shows that the image has measure zero. QED

**Proposition 7.8 (reality).** For Q >= 2 the Sim15 and Ry-only images consist of real vectors. So neither ansatz is surjective onto CP^{2^Q - 1} at any depth.

*Proof.* Lemma 3.18(v) shows the images are real, so they lie in RP^{2^Q - 1}, which has dimension 2^Q - 1 < 2^{Q+1} - 2. Then apply Theorem 7.2. QED

**Proposition 7.9 (Q = 1 words)** [addendum (12.3)]. The map (t0, t1, t2) -> [Rx(t2) Rz(t1) Rx(t0)|0>] is onto CP^1, and it has rank 2 wherever sin t0 != 0. Under Ry-only, t0 -> [Ry(t0)|0>] is onto RP^1.

*Proof.* Rz(t1) Rx(t0)|0> is, up to phase, the Bloch point with polar angle t0 and azimuth t1 - pi/2. Spherical coordinates cover S^2 and are a local diffeomorphism wherever sin t0 != 0. Rx(t2) acts on CP^1 as a diffeomorphism. By Lemma 7.4 the rank is at most 2. QED

**Theorem 7.10 (IQP, ansatz 0, L = 1: flat moduli)** [addendum (12.4)]. The image of pi_proj o f_w lies in the flat-modulus torus {[x] : abs(x_k) = 2^{-Q/2} for all k}, which has dimension 2^Q - 1. The image itself has dimension at most Q - 1. Under ansatz 1 the image is H^{(x)Q} applied to the same set, so the same dimension bound holds.

*Proof.* Theorem 3.22 and Lemma 7.4. QED

**Corollary 7.11.** Under ansatz 0 with L = 1, every computational-basis marginal of every subset of qubits of a word is uniform. So a Z-basis readout of a freshly prepared wire carries no information about the angles. The reduced density matrix of a wire does depend on the angles. For example, at Q = 2 qubit 1 has <Px> = (1 + cos theta)/2. So the wire carries information that a later non-diagonal gate can read.

*Proof.* All moduli are equal. For Q = 2, the amplitudes are (1/2)(1, e^{-i theta/2}, 1, e^{i theta/2}) by Theorem 3.22, and computing the off-diagonal entry of rho_1 gives (1 + e^{-i theta})/4. QED

**Remark 7.12 (numerical ranks; UNVERIFIED as theorems)** [addendum Sec. 12.1, l.600-606]. Jacobian ranks computed in fp64 at random points:

| Ansatz | Q | L | angles | rank |
|---|---|---|---|---|
| Sim15 | 2 | 1 | 4 | 3 (full real) |
| Sim14 | 2 | 1 | 8 | 6 (full complex) |
| Sim14 | 3 | 2 | 24 | 14 (full) |
| Sim15 | 3 | 2 | 12 | 7 (full real) |
| Sim15 | 3 | 1 | 6 | 6 of 7 |
| IQP | 2 | 1 | 1 | 1 of 6 |
| IQP | 3 | 1 | 2 | 2 of 14 |

A full rank at one point shows that the image has non-empty interior. It does not show surjectivity.

**Conjecture 7.13** [addendum Sec. 19 item 3, Sec. 13.2(c); limits l.324].

- At L >= L_sat, Sim14 has generic rank 2^{Q+1} - 2 and is onto CP^{2^Q - 1}.
- At L >= L_sat, Sim15 has generic rank 2^Q - 1 and is onto RP^{2^Q - 1}.

Also open: the depth at which Sim15 generates SO(d^2) and Sim14 generates SU(d^2) as gates, and whether these ansatze reach the eigenbasis of an arbitrary density matrix.

**Proposition 7.14 (parameters versus coordinates)** [addendum (12.20), Sec. 12.1 after (12.5); limits l.1125-1127, l.1218].

- (a) A dense word with kl legs of q qubits has 2^{kl q} complex (2^{kl q + 1} real) coordinates, and represents a point of a manifold of dimension 2^{kl q + 1} - 2.
- (b) If P_w < 2 d_Q - 2, the image of an angle parametrisation has measure zero (Theorem 7.6). So an angle table can use at most 2 fewer reals than the amplitude vector without becoming measure-zero. The Q = 1 word, with 3 angles against 4 reals, is onto (Proposition 7.9).
- (c) The map U(d_Q) -> CP^{d_Q - 1}, U -> [U e_0], has fibres that are cosets of U(1) x U(d_Q - 1). Its image has dimension d_Q^2 - 1 - (d_Q - 1)^2 = 2 d_Q - 2. A dense per-word unitary (d_Q^2 reals) therefore adds expressivity only when the word acts as a map (gates, MPO cores), not as a state.
- (d) dim U(dd) = dim_R u(dd) = dd^2, because u(dd) is the space of skew-Hermitian matrices. A dense complex matrix has 2 dd^2 reals.

*Proof.* (a) Theorem 7.2. (b) Theorem 7.6. (c) The orbit-stabiliser theorem for the transitive action of U(d_Q). (d) Lie-algebra count. QED

## 7.2 The function class in the angles

**Lemma 7.15 (spectral form of gates)** [spec C5, C13]. If G^2 = I, then exp(-i theta G/2) = e^{-i theta/2} Pr_+ + e^{i theta/2} Pr_-, so Omega_g = {-1, +1}. If G = |1><1| (x) Pa with Pa^2 = I, then exp(-i theta G/2) = Pr_0 + e^{-i theta/2} Pr_+ + e^{i theta/2} Pr_-, so Omega_g = {-1, 0, +1}.

*Proof.* The spectral decomposition of G. QED

**Theorem 7.16 (the readout as a rational trigonometric function)** [addendum (12.6)-(12.8)]. Fix a parse. Let G_1q and G_ctrl be the numbers of parametrised one-qubit and controlled gates. Then

    sigma[s](theta) = sum_om c_om(s) exp(i om . theta/2),     om_k in { sum_{g : k(g) = k} e_g : e_g in Omega_g },
    N_c(theta) = sum_nu a_nu(c) exp(i nu . theta/2),  with nu a difference of two such om and a_{-nu} = conj(a_nu),
    p_c = N_c / Z.

The frequencies of N_c in theta_k have modulus at most m_k, the multiplicity of theta_k. If theta_k sits only on one-qubit rotations, these frequencies are integers and N_c is 2 pi-periodic in theta_k. If a controlled gate carries theta_k, they are half-integers and N_c is 4 pi-periodic. sigma[s] has at most 2^{G_1q} 3^{G_ctrl} distinct frequency vectors, and N_c at most 3^{G_1q} 5^{G_ctrl}.

*Proof.* sigma[s] is a fixed contraction of the product of the word states (Theorem 2.20), so it is a sum of products of gate-matrix entries. Substitute Lemma 7.15 and expand. N_c = sigma conj(sigma) turns frequencies into differences. The difference set {+-1} - {+-1} has 3 elements, and {-1,0,1} - {-1,0,1} has 5. Parity: for one-qubit gates every e_g is odd, so every nu_k is even. QED

**Corollary 7.17 (repeated words)** [addendum (12.6); spec (5.2), C13, Sec. 5.5]. A word occurring m times gives m_k = m for each of its angles. The two-term shift rule gives i sin(om pi/2) e^{i om t} per mode, so it is exact for f iff every frequency with a non-zero coefficient lies in {0, +-1}. It therefore fails:

- generically when m >= 2 (for f = cos 2t it gives 0 instead of -2 sin 2t);
- already at m = 1 for controlled-gate angles, which have frequency +-1/2.

By the product rule, derivatives add over occurrences, so adjoint gradients are unaffected (Lemma 5.20).

*Proof.* Theorem 7.16 and Theorem 5.5. QED

**Theorem 7.18 (data re-uploading)** [addendum (13.11)]. Let layer l = 1..L use the angles theta_l + Wt_l x, where Wt_l is a data-weight matrix. Let P_L be the number of parametrised gates per layer, and let wt_g be the row of Wt_l belonging to gate g. Then:

- every amplitude of |w(x)> is sum_om b_om e^{i om . x}, with om in {(1/2) sum_g e_g wt_g : e_g in Omega_g}, the sum running over all L P_L gates;
- the probabilities have frequencies in the difference set;
- if all the directions are tied, wt_g = wt, the probabilities are trigonometric polynomials in wt.x whose frequencies lie in (1/2)Z with modulus at most L P_L (in wt.x units), and are integers when every gate is a one-qubit rotation. For example, a single Ry(x) gives cos^2(x/2) = 1/2 + (1/2) cos x.

*Proof.* exp(-i(theta + wt.x)G/2) = exp(-i theta G/2) exp(-i (wt.x) G/2), because the two factors commute. Expand as in Theorem 7.16. The attribution of this construction (Perez-Salinas 2020; Schuld, Sweke and Meyer 2021) is UNVERIFIED. QED

**Proposition 7.19 (the ceiling of any front end)** [addendum (13.1), (13.1'), (13.8), (13.9), Sec. 15.3(d)]. For every map x -> theta_w(x), the word state lies in CP^{2^{Q_w} - 1}, which has dimension 2^{Q_w + 1} - 2. A front end changes which point is chosen, not how many dimensions are available. For example, a q_n = 1 noun is a point of the Bloch sphere.

*Proof.* Theorem 7.2. QED

**Remark 7.20.** The claim that real language needs q_n = 4..6 is an UNVERIFIED heuristic [addendum Sec. 19 item 6]. That training drives Z above 2^{-2 q_n} is a Conjecture [addendum Sec. 19 item 7]. The law of Z at initialisation is Theorem 3.32 and Corollary 3.33.

## 7.3 Multilinearity and the impossibility of switching

**Theorem 7.21 (affinity)** [addendum (15.1)]. Fix a parse and every occurrence except w. Then sigma = A_w phi_w, where A_w is a d_s x 2^{Q_w} matrix built from the other words and the cups. Consequently:

- (a) d^2 sigma/d phi_w^2 = 0;
- (b) N_c - N_{c'} = phi_w^dag Hq phi_w, where Hq = A_w^dag (|c><c| - |c'><c'|) A_w is Hermitian, so the decision boundary is a real quadric cone;
- (c) along a real line phi = uu + t vv, the score is a real quadratic in t;
- (d) the score is homogeneous of degree 2.

*Proof.* By Theorem 2.20, Psi is linear in each factor, and sigma is a fixed linear map of Psi. QED

**Definition 7.22 (slot degree).** A sentence map Fs: V_1 x ... x V_n -> C^{d_s} has slot degree at most k in slot j if, for all fixed other arguments and all uu, vv, the map t -> Fs(..., (1-t)uu + t vv, ...) is a polynomial of degree at most k in the real variable t. An occurrence that enters once has slot degree 1 (Theorem 7.21). A word whose state fills k slots has slot degree k.

**Theorem 7.23 (no switching inside one evaluation)** [addendum (15.1), 15.2(c)]. Let Fs have slot degree at most k, fix phi(t) = (1-t)uu + t vv for t in [0,1], and assume Z > 0 on the segment.

- (a) Fs(phi(t)) = sum_{i <= k} t^i Fs_i. For k = 1 this is (1-t) Fs(uu) + t Fs(vv).
- (b) N_c - N_{c'} and N_c - lam Z, for lam in (0,1), are real polynomials in t of degree at most 2k. So each decision changes at most 2k times, and each level set {p_c = lam} either meets the segment in at most 2k points or contains all of it.
- (c) Suppose Fs(phi(t)) = Fs(uu) for all t < t0 and Fs(phi(t)) = Fs(vv) for all t > t0, for some t0 in (0,1). Then Fs(uu) = Fs(vv). The same holds with p_c in place of Fs.
- (d) If abs(N_c - N_{c'}) <= mu_b on [0,1], then abs(d/dt (N_c - N_{c'})) <= 8 k^2 mu_b there. So moving from -(1-eps)mu_b to +(1-eps)mu_b takes a t-interval of length at least (1-eps)/(4k^2).

Raising (q_n, q_s, L) raises the dimension, not the degree. The degree is fixed by how many times an occurrence enters.

*Proof.* (a) By definition. For k = 1, a polynomial of degree 1 is determined by its values at t = 0 and t = 1.

(b) abs(Fs_c)^2 has real degree at most 2k, and a non-zero polynomial of degree at most 2k has at most 2k real roots. Since Z > 0, p_c = lam iff N_c - lam Z = 0.

(c) A polynomial that is constant on an interval is constant everywhere. For p_c, N_c - p_c(uu) Z vanishes on [0, t0), hence identically.

(d) By Markov's inequality (Appendix C), max abs(Pl') <= 2 n^2 max abs(Pl)/(b - a) for Pl of degree n on [a, b]. Take n = 2k and b - a = 1. QED

**Proposition 7.24 (idealised transition law).** Let k = 1, Fs(uu) = al e_c and Fs(vv) orthogonal to e_c, with a1 = abs(al)^2 and b1 = ||Fs(vv)||^2. Put rho_t = (1-t)/t. Then

    1 - p_c = 1/(1 + (a1/b1) rho_t^2).

The 10 %-90 % window spans rho_t in [1/(3 sqrt(a1/b1)), 3/sqrt(a1/b1)], a factor of 9, which is width ln 9 in ln rho_t for every a1/b1.

For k slots, assume in addition that Fs vanishes on every mixed product of uu and vv. Then 1 - p_c = 1/(1 + kap rho_t^{2k}), and the window has width 9^{1/k} in rho_t. Without that hypothesis the cross terms change the law. For example, at k = 2 with Fs(uu,uu) = al e_c, Fs(vv,vv) orthogonal to e_c with b1 = ||Fs(vv,vv)||^2, and Fs(uu,vv) = Fs(vv,uu) = gam e_c, one gets 1 - p_c = 1/(1 + abs(al rho_t^2 + 2 gam rho_t)^2 / b1), which reduces to 1/(1 + abs(rho_t^2 + 2 gam rho_t)^2) when al = b1 = 1; its window tends to a factor of 9 for large abs(gam). Width in t is not bounded below.

*Proof.* Orthogonality kills the cross terms, so N_c = (1-t)^2 a1 and Z - N_c = t^2 b1. Solve 1/(1+x) = 0.9 and 0.1. For k slots, the stated hypothesis leaves only the terms (1-t)^k and t^k. In the k = 2 example, Fs(phi(t), phi(t)) = ((1-t)^2 al + 2 t(1-t) gam) e_c + t^2 Fs(vv,vv), so N_c = abs((1-t)^2 al + 2 t(1-t) gam)^2 and Z - N_c = t^4 b1; dividing both by t^4 gives the stated law. QED

**Remark 7.25 (priced escapes)** [addendum 15.2(c), (11.25), (14.18), (13.2c)]. There are three ways to go beyond degree 1:

- re-prepare the word k times, which gives slot degree k at the price of k preparations;
- enumerate candidates and take an argmax over evaluations;
- put a classical selector in front.

For approximating sign(x) on [-1, -delta] union [delta, 1] to error eps by a polynomial Pl of degree n with abs(Pl) <= 1 on [-1, 1], one needs n >= sqrt((1 - eps)/delta).

*Proof.* Pl(delta) - Pl(-delta) >= 2(1 - eps), so abs(Pl'(xi)) >= (1 - eps)/delta for some xi. Markov's inequality gives abs(Pl') <= n^2 on [-1,1]. QED

Degree O(delta^{-1} log(1/eps)) suffices (Eremenko and Yuditskii 2007, cited, UNVERIFIED).

**Proposition 7.26 (the ceiling depends on d)** [review 4; addendum (12.9), l.666]. Let n_1, ..., n_m be unit vectors in C^d, and let V range over the whole of C^{d x d_s x d}. This covers the dense-tensor model, or an ansatz assumed surjective per Conjecture 7.13.

- (a) If the n_i are linearly independent (so m <= d), then for every table Tt(i,j) in C^{d_s} \ {0} there is a unit V with sigma(n_i, n_j) proportional to Tt(i,j) for all i, j. So every table of readout distributions is realisable.
- (b) If sum_i la_i n_i = 0 with la != 0 (forced when m > d), then sum_i la_i sigma(n_i, n_j) = 0 for every V and every j. Example: d = d_s = 2 and n_3 proportional to n_1 + n_2. Requiring p(1,j) = p(2,j) = (1,0) forces p(3,j) = (1,0), so the table with p(3,j) = (0,1) is not realisable.

So the addendum's "cannot represent any non-multilinear function, independent of q_n, q_s, L" is correct for functions on the whole state space (Theorem 7.23), but on a finite vocabulary it holds only when the vocabulary vectors are linearly dependent. For IQP with L = 1 (flat moduli, Theorem 7.10), the V constructed in (a) is in general not reachable.

*Proof.* (a) Let n~_i be the dual basis, <n~_i|n_{i'}> = delta_{ii'}, on span{n_i}. Put V[a,s,b] = cst sum_{i,j} conj(n~_i[a]) Tt(i,j)[s] conj(n~_j[b]). Then sigma(n_i, n_j) = cst Tt(i,j).

(b) sigma is linear in its first slot. In the example, sigma(n_3, n_j) is proportional to sigma(n_1, n_j) + sigma(n_2, n_j), which is proportional to e_0. QED

**Remark 7.27 (interpretive items)** [addendum l.666, Sec. 15.2(d,e), 15.5]. The following are definitions or interpretations, not theorems:

- phi_w depends only on theta_w (spec (1.2)), so a word has no context-dependent meaning;
- information flows only along cups;
- n_cl classes need n_cl <= d_s;
- "coreference and ambiguity need an argmax". This last item holds for functions on the whole state space by Theorem 7.23; on a finite linearly independent vocabulary, Proposition 7.26(a) shows that no argmax is needed. The addendum's escape list for it is (ii) or (iii).

The model defines p(answer | text) [addendum (14.19)], not a distribution over sentences.

## 7.4 Word-order sensitivity

**Proposition 7.28** [addendum (12.10)]. With V_s[a][b] = V[a,s,b],

    sigma_{ALB}[s] - sigma_{BLA}[s] = A^T (V_s - V_s^T) B.

- (a) This vanishes for all unit A, B iff every V_s is symmetric. The symmetric tensors form a subspace of complex dimension d_s d(d+1)/2, which is proper for d >= 2 and has measure zero on the unit sphere.
- (b) For a fixed pair with A and B linearly independent, vanishing imposes d_s independent linear conditions on V.
- (c) For d_s >= 2 and linearly independent A, B, the readouts p^{ALB} and p^{BLA} differ for all V outside a null set. If A is parallel to B there is no sensitivity.

*Proof.* The difference formula is spec (1.5). (a) Take A = e_i and B = e_j.

(b) The functional Mt -> A^T Mt B on antisymmetric Mt is non-zero iff A and B are linearly independent: take Mt = x y^T - y x^T with (A^T x)(B^T y) - (A^T y)(B^T x) != 0.

(c) p_0^{ALB} - p_0^{BLA} is real-analytic in V on the set Z > 0. It is not identically zero: V_0 = conj(A) conj(B)^T and V_1 = conj(B) conj(A)^T give sigma_{ALB} proportional to (1, gm) and sigma_{BLA} proportional to (gm, 1), with gm = abs(<A|B>)^2 < 1. The set where Z_ALB = 0 or Z_BLA = 0 is the union of the kernels of the two complex-linear maps V -> sigma_ALB and V -> sigma_BLA, each a proper subspace, so of real codimension >= 2. Its complement is therefore open, connected and of full measure. A real-analytic function on a connected open set that is not identically zero vanishes only on a null set. Finally p is homogeneous of degree 0 in V (N_c and Z are both homogeneous of degree 2), so its zero set is a cone; by integration in polar coordinates, a cone that is a null set meets the unit sphere in a set of surface measure zero. QED

**Remark 7.29 (numerical instance)** [spec Sec. 8; addendum (12.10), Sec. 13.6(ii)]. At the spec's Sec. 8 point, "Alice loves Bob" has p_0 = 0.58263882 and "Bob loves Alice" has p_0 = 0.42051694. These values are quoted, not re-derived.

## 7.5 Low-rank verbs

**Proposition 7.30** [addendum (12.13), (12.20)]. Verbs of the form V[a,s,b] = sum_{r1, r2 < r} A[a][r1] B[r1][s][r2] C[r2][b], with r <= d, have:

- rank V_{(a)x(s,b)} <= r;
- a parameter count of 2 d r + r^2 d_s;
- an image of complex dimension at most 2 d r + r^2 d_s - 2 r^2.

*Proof.* The rank bound is Lemma 4.32(a). The gauge group GL(r) x GL(r) acts by (A Gg, Gg^{-1} B Hg, Hg^{-1} C) and leaves V unchanged. On the Zariski-open set where A has full column rank and C has full row rank, the action is free, so its orbits have dimension 2 r^2 and lie in the fibres. By the fibre-dimension theorem, the image has dimension at most the parameter count minus 2 r^2. QED

## 7.6 Trainability

**Theorem 7.31 (gradient variance under a 2-design; after McClean et al., Nat. Commun. 9, 4812 (2018)).** Let Ob be an observable on C^N, and put

    Ec(theta) = Tr(Ob U_- Wg(theta) rho Wg(theta)^dag U_-^dag),

where Wg(theta) = exp(-i theta G/2), G^2 <= I and rho = |psi><psi|. Let U_- be a unitary 2-design, independent of psi. Then, conditionally on psi, E[dEc/dtheta] = 0 and

    Var[dEc/dtheta] = Var_{psi'}(G) Tr(O_0^2)/(2(N^2 - 1)),       psi' = Wg(theta) psi.

*Proof.* dEc/dtheta = Tr(U_-^dag Ob U_- Xh), with Xh = -(i/2)[G, rho'] Hermitian and traceless. For a 2-design, E[(U^dag Ob U)^{(x)2}] commutes with every V (x) V. By Schur-Weyl duality it equals al I + be Fsw, where Fsw is the swap. Taking traces gives

    al N^2 + be N = (Tr Ob)^2,     al N + be N^2 = Tr Ob^2,

so be = Tr(O_0^2)/(N^2 - 1). Then E[Tr(Ao Xh)^2] = be Tr(Xh^2), where Ao = U_-^dag Ob U_-, and E[Tr(Ao Xh)] = Tr(Ob) Tr(Xh)/N = 0. Finally, for pure rho', Tr(Xh^2) = (1/2) Var_{psi'}(G). QED

**Corollary 7.32 (projector versus Pauli; which W) [errata E-10; spec Sec. 5.5].**

- (a) For a rank-1 projector Ob, Var <= 1/(2N(N+1)), which is about 2^{-2W}/2. For a Pauli string, Var <= N/(2(N^2 - 1)), which is about 2^{-W}/2.
- (b) In the bent-wire form (Theorem 2.24), N_c is a rank-1 projector expectation on W = |K| + q_s qubits, so under a global 2-design Var[dN_c] = O(2^{-2W}). Here E[Z] = 2^{-|K|} = 2^{q_s - W}.
- (c) In the Bell-circuit form, N_c = 2^{Sq} times a rank-1 projector expectation on W_B = 2 Sq + q_s qubits, where Sq = sum_e q_e. So Var[dN_c] <= 2^{2Sq}/(2 N_B(N_B + 1)), which is about 2^{-W_B - q_s}/2, with E[Z] = 2^{-Sq}. The spec's "Var ~ 2^{-W}" is the right order for this unnormalised N_c on W_B.
- (d) In both forms, Var[dN_c]/E[Z]^2 = O(2^{-2 q_s}). This is consistent with the observed flat Var[dp_c] (Remark 5.44(iii)), though it does not prove it.

*Proof.* (a) Theorem 7.31 with Var_psi(G) <= 1. A rank-1 projector has Tr(O_0^2) = 1 - 1/N, and a Pauli string has Tr(O_0^2) = N. (b) and (c) follow from (a) and the respective scalings, with E[Z] from Corollary 3.33(c). (d) is arithmetic. QED

The Monte Carlo check of (c) quoted in the review (Var x 2^W roughly constant at 0.21-0.25 for W = 3..11, q_s = 1) is consistent with this.

**Remark 7.33 (what is and is not proved).**

- Theorem 7.31 needs a global 2-design, which shallow sentence circuits are not.
- For shallow circuits with local costs the variance decays at worst polynomially (Cerezo et al., Nat. Commun. 12, 1791 (2021); cited).
- At fixed W every variance is a fixed positive number.
- The Lie-algebraic prediction Var ~ 1/dim g (Ragone et al. 2024) is UNVERIFIED here, and its hypothesis fails for one-layer IQP [limits l.1094].

**Theorem 7.34 (the IQP origin; with a correction to errata E-10)** [errata E-10]. Let every word be IQP (ansatz 0 or 1, any L) or a Q = 1 word Rx Rz Rx, and set theta = 0.

- (a) grad N_c(0) = 0 for every c, and grad Z(0) = 0. If Z(0) > 0, then grad p_c(0) = 0. If also p_y(0) > 0, the gradient of Loss_CE vanishes.
- (b) Under ansatz 0 with L = 1, if the word that carries the survivor field is IQP, then Z(0) > 0 and p_c(0) = 1/d_s. Under ansatz 1 the value can differ. For example, N TV N at (1,1) gives p_0(0) = 1, so Loss_CE is +infinity for label 1.
- (c) For N TV N at (1,1) under ansatz 0 with L = 1, let theta be Alice's first Rx angle and phi the verb's CRz(a -> s) angle, with all other angles 0. Then exactly

      p_0 = 1/2 - (1/2) sin(theta) sin(phi/2).

  The Hessian on this plane is [[0, -1/4], [-1/4, 0]], and every pure second derivative vanishes at 0 (Theorem 5.41).
- (d) Correction: E-10's "diagonal Hessian exactly 0 for every IQP sentence" is false. For N TV N at (2,1) under ansatz 0 with L = 1, let psi be the verb's CRz(s -> b_0) angle. Along psi, p_0 = 1/(1 + cos^2(psi/2)), so d^2 p_0/dpsi^2(0) = 1/8. The origin is still a saddle. On the plane of the noun-1 angle theta and the verb's CRz(a_1 -> s) angle phi,

      p_0 = cos^2((theta - phi)/4)/(cos^2((theta - phi)/4) + cos^2((theta + phi)/4)) = 1/2 + theta phi/16 + O(3).

  Whether a saddle occurs at every other profile is UNVERIFIED.

*Proof.* (a) At theta = 0 every gate is real. The generators Px, Pz and |1><1| (x) Pz are real, so this is Theorem 5.40. (Ry-based ansatze are excluded, because -i Py/2 is real.)

(b) The IQP words equal |+>^{(x)Q}, and the Q = 1 words equal |0>. All amplitudes are therefore non-negative and do not depend on s. The term with every cup index 0 is positive, so Z(0) > 0.

(c) n1 = cos(theta/2)|0> - i sin(theta/2)|1>, n2 = |0>, and V[a,s,b] = 2^{-3/2} e^{i a (2s-1) phi/2}. Hence abs(sigma[s])^2 = (1/8)(1 -+ sin theta sin(phi/2)), with the upper sign for s = 0.

(d) With n1 = n2 = (1/2)(1,1,1,1) and V = 2^{-5/2} e^{i s (2 b_0 - 1) psi/2}, sum_b n2[b] e^{...} = 2 cos(s psi/2), so N_1/N_0 = cos^2(psi/2). For the plane, sigma[s] is proportional to conj(al) + e^{i(2s-1)phi/2} al with al = 1 + e^{i theta/2}. Apply 1 + cos x = 2 cos^2(x/2) and expand to second order. QED

**Remark 7.35 (counting identities; cross-reference only)** [spec Sec. 7; addendum (12.11), (12.14); review 2.3, 5(d); errata E-4]. The amplitude-count identities are consequences of Propositions 4.9, 4.11 and 4.35 and Theorem 4.14:

- the fused-sweep peak 2^{2q_n + q_s} + 2^{q_n + q_s} + 2 x 2^{q_n};
- the in-place variant;
- the bent-wire widths (Proposition 2.25(iii));
- the naive product state 2^{4 q_n + q_s};
- the MPO storage d r + r^2 d_s + r d.

A pure branch uses fewer entries than the doubled register 4^W. The largest word's 2^{Q_w} amplitudes are the smallest object of the state-vector and fused-contraction evaluators, not a universal lower bound: Feynman-path summation uses O(W) memory in exponential time [errata E-4]. Byte tables, head sizes and stakeholder statements are engineering. The generalisation, retrieval and probe figures of addendum Sec. 15.3(b,d,e) are UNVERIFIED.
# Appendix A. Correspondence to the engineering documents

## A.1 Result -> engineering labels it underpins

| Result | Title | Engineering labels |
|---|---|---|
| Definition 1.1 | (free pregroup, discrete case) | spec (1.1), Sec. 1.1; addendum (11.1)-(11.3), C15 |
| Lemma 1.2 | (string adjoints) | addendum Sec. 11.2 |
| Definition 1.3 | (lexicon, grammaticality) | spec Sec. 1.1 table; addendum Sec. 11.2 table, C15 |
| Lemma 1.4 | (switching lemma) | spec l.150; addendum (11.2)-(11.3); errata Sec. B; review 2.10 |
| Corollary 1.5 |  | spec (1.1): "expansions are never needed"; addendum l.88 |
| Definition 1.6 | (reduction) | addendum l.88-93; errata E-12 |
| Theorem 1.7 | (derivations are planar matchings) | errata E-12; addendum l.88-93; review 2.10, 5(a) |
| Lemma 1.8 | (survivor-cup bijection) | addendum (11.5), equivalent form |
| Definition 1.9 | (chart recurrence) | addendum (11.4), (11.5); errata E-12 |
| Theorem 1.10 | (correctness and exact counting) | addendum (11.4)-(11.5), l.95-126, T11; errata E-12; review 2.10, 5(a) |
| Theorem 1.11 | (exact cost) | errata E-12; review 2.10, correcting addendum l.107 "~m^3/12"; addendum Sec. 16 |
| Proposition 1.12 | (enumeration of parses) | addendum l.117-122; (11.25), (11.26) |
| Theorem 1.13 | (Buszkowski; cited) | addendum Sec. 11.6 l.454; review 8 |
| Proposition 1.14 | (crossing matchings) | addendum Sec. 11.6, l.457-459 |
| Proposition 1.15 | (lexical ambiguity lattice) | errata E-12; review 2.10 |
| Proposition 1.16 | (Catalan ambiguity) | addendum (11.25), (11.26), l.103-104, 124; errata E-12 |
| Definition 1.17 | (ambiguity objects) | limits B1-B3; addendum (11.25), (11.26) |
| Proposition 1.18 | (row-stochastic slot mixing is not a mixture of assignments) | limits l.1463; addendum (11.25) |
| Definition 1.19 | (runs, greedy run) | spec l.162-174; errata E-12 |
| Theorem 1.20 | (accepting runs = reductions) | errata E-12 |
| Proposition 1.21 | (exactness criterion for greedy) | spec l.175-176; errata E-12; addendum l.129-139 |
| Corollary 1.22 | (degree-one condition) | spec l.175-176 (corrects its justification); errata E-12 |
| Proposition 1.23 | (the shape table) | spec Sec. 1.1 table, l.176, Sec. 9 item 1 (l.1242); addendum l.129; review 7 P0 |
| Proposition 1.24 | (greedy is incomplete one word outside the table) | errata E-12; spec l.162-163; review 2.10; addendum l.129-132, T11 |
| Proposition 1.25 | (plain backtracking is exponential) | errata E-12; spec l.176-179 |
| Conjecture 1.26 | (characterisation of greedy failure) | errata E-12; addendum l.130-132; review 2.10 |
| Definition 1.27 | (nesting depth) | addendum (11.21), (11.23) |
| Proposition 1.28 | (right embedding: constant depth) | addendum l.183; (11.21), (11.23) |
| Proposition 1.29 | (centre embedding: +3 per object relative) | addendum l.183 |
| Conjecture 1.30 | (uniqueness of the embedding reductions for general k) | addendum Sec. 19 item 9 |
| Example 1.30' | (aux-inverted question) | addendum Sec. 19 item 9 |
| Definition 1.31 | (meaning along a reduction) | spec (1.5); addendum C15, (11.10)-(11.13') |
| Lemma 1.32 | (an injective type code) | spec Sec. 6.3 l.1121-1122, C7, T10; addendum C15 |
| Example 1.33 | ("Alice loves Bob") | spec Sec. 8 l.1184, (1.1), C7 |
| Definition 2.1 | (strict monoidal category; symmetry) | (background for spec Sec. 1.2) |
| Lemma 2.2 | (disjoint operations commute) | spec Sec. 1.3 (commuting word applications); addendum (11.22) |
| Definition 2.3 | (rigid monoidal category) | spec Sec. 1.2 (grammar category) |
| Definition 2.4 | (compact closed category) | spec Sec. 1.2 |
| Lemma 2.5 | (name/coname bijection; the bent wire) | spec Sec. 1.4 (bend-the-wire) |
| Definition 2.6 | (pregroup) | spec (1.1) |
| Proposition 2.7 | (Lambek) | spec (1.1), Sec. 1.2 |
| Definition 2.8 | (free rigid monoidal category on B) | spec Sec. 1.2; addendum (11.25) (distinct parses are distinct morphisms) |
| Definition 2.9 | (self-dual structure on C^{2^q}) | spec C8 |
| Lemma 2.10 | (snake equations in FHilb) | spec C8 (either pairing) |
| Proposition 2.11 | (normalisation and pairing of the cup) | spec C8 |
| Lemma 2.12 | (Bell-circuit realisation of the cup) | spec C8, (1.4), (2.6) |
| Lemma 2.13 | (the factor 2^{-q/2} is optimal) | spec (2.6); addendum (11.22) (optimality of the Bell factor) |
| Definition 2.14 | (word fields) | spec C7, (0.9) |
| Lemma 2.15 | (the C7 index map is an isomorphism) | spec C7, Sec. 1.1 |
| Definition 2.16 | (DisCoCat functor) | spec Sec. 1.2, C7 |
| Proposition 2.17 | (Fr is well defined and preserves the rigid structure) | spec Sec. 1.2 |
| Proposition 2.18 | (independence of cup order) | spec l.150 (order of cups), (1.3); errata Sec. B |
| Definition 2.19 | (word meaning; sentence morphism; normaliser) | spec (1.2), (1.4) |
| Theorem 2.20 | (component formula) | spec (1.3), (1.5) |
| Remark 2.21 | (frames and the Choi correspondence) | addendum Sec. 14.1, (14.3)-(14.4), 13.2(a) |
| Lemma 2.22 | (state to effect) | spec Sec. 1.4, C8, C11 |
| Definition 2.23 | (bent-wire assignment) | spec Sec. 1.4 |
| Theorem 2.24 | (the bent-wire circuit is exact) | spec Sec. 1.4, (1.3), (1.4), Sec. 8, T5, T6 |
| Proposition 2.25 | (the assignment is forced; odd cycles) | spec Sec. 1.4; addendum (11.22); spec (2.6) |
| Corollary 2.26 | (wire maps at (q_n, q_s) = (1, 1)) | spec Sec. 1.4; T5-T7 |
| Definition 2.27 | (special commutative dagger-Frobenius algebra) | addendum (11.6)-(11.9) |
| Proposition 2.28 | (the basis spider) | addendum (11.6)-(11.9) |
| Lemma 2.29 | (spiders induce the cup) | addendum (11.6); spec C8 |
| Lemma 2.30 | (non-unitarity and circuit realisation) | addendum (11.7)-(11.9); spec (1.4), (1.6), (2.6); addendum T13 |
| Definition 2.31 | (relative pronoun and coordination tensors) | addendum (11.10)-(11.13'), C15 |
| Proposition 2.32 | (sentence formulas) | addendum (11.14)-(11.17'), T12, T13 |
| Corollary 2.32' | (complementiser orientation) | addendum (11.14'), T14 |
| Proposition 2.33 | (sum semantics for coordination) | addendum (11.12'), text after (11.17') |
| Conjecture 2.34 | (spiders in the bent-wire circuit) | addendum (11.22), T18; spec (2.6), Sec. 1.4 |
| Definition 2.35 | (text register) | addendum C17, (13.5), (13.6), (14.1), (14.2) |
| Lemma 2.36 | (index formula and field swap) | addendum (14.1), C17 |
| Proposition 2.37 | (Frobenius bending is diagonal and commutative) | addendum (14.3), (11.6), (11.8) |
| Definition 2.38 | (CPM(FHilb)) | limits l.184, l.206 |
| Theorem 2.39 | (Selinger; Choi 1975; cited) | limits l.184, l.638 |
| Proposition 2.40 | (specialisations) | limits l.184, l.206, l.638; addendum (11.7)-(11.9), (13.7) |
| Definition 2.41 | (words as CP maps; higher-order boxes) | limits l.184, l.810, HO.2-HO.4 |
| Remark 2.42 | (qudit and mixed-radix version) | addendum Sec. 12.5, (12.16'); limits l.1110 |
| Proposition 2.43 | (order invariance of the Frobenius merge) | addendum 11.3; review 5(a) |
| Proposition 2.44 | (norm collapse of the merge; growth of the sum) | review 5(a); spec l.217 |
| Definition 3.1 | (register) | spec C1 |
| Definition 3.2 | (Kronecker order) | spec C2 |
| Lemma 3.3 | (basis vectors factor) | spec C1, C2 |
| Corollary 3.4 | (a word adjoined in the high bits) | spec (2.7), C2 |
| Definition 3.5 | (bit-field insertion) | limits T.2; spec C7 |
| Lemma 3.6 | (bit-insertion enumeration) | spec (2.2)-(2.5), T1 |
| Definition 3.7 | (fixed matrices) | spec (0.10), (0.4) |
| Lemma 3.8 | (rotation matrices) | spec C5, (0.1)-(0.3), C13 |
| Definition 3.9 | (local gates) | spec C13, T2 |
| Lemma 3.10 | (butterfly) | spec (2.1), C13 |
| Lemma 3.11 | (quad update; lifts) | spec (2.3), (0.12), C6 |
| Lemma 3.12 | (controlled gates: bit-test definitions equal the dense forms) | spec (0.5)-(0.8), (0.11), C6 |
| Lemma 3.13 | (controlled rotations from their generators) | spec C13, (0.7), (0.8) |
| Remark 3.14 | (relabelled layouts) | spec (3.1), Sec. 3.1-3.2, Sec. 2.4; limits HO.1, HO.2 |
| Lemma 3.15 | (flip masks versus CNOT) | spec Sec. 2.4, (0.5) |
| Lemma 3.16 | (2 pi versus 4 pi periodicity) | spec C5, Sec. 5.6, T6 |
| Definition 3.17 | (ansatze) | spec C10, Sec. 5.5 |
| Lemma 3.18 | (unitarity, transposes, conjugates, realness) | spec C11, Sec. 1.4, T3; limits NU.5 |
| Remark 3.19 | (dimension counts; see Chapter 7) | addendum (12.1), (12.2), (14.4) |
| Lemma 3.20 | (gate identities) | spec (5.5), T3 |
| Lemma 3.21 | (the Q = 1 noun) | spec Sec. 8, C10, T4 |
| Theorem 3.22 | (IQP fixed-modulus lemma) | spec C10, Sec. 8, T4; addendum (12.4); review 3 item 15 |
| Remark 3.23 | (angle generation of arbitrary vectors) | addendum Sec. 12.5, l.819-821; limits l.1139 |
| Definition 3.24 | (postselection and compaction) | spec (2.5), (2.5'), (2.6), Sec. 4.5, Sec. 5.1, (5.1), (1.6) |
| Lemma 3.25 | (compaction; the denominator) | spec (2.5), (2.5'), (2.2)-(2.4) |
| Lemma 3.26 | (the Bell cup in index form) | spec C8, (1.4), (2.6), Sec. 1.4; errata Sec. B (l.208); review 2.11 |
| Lemma 3.27 | (sharp normalisation inequality) | errata E-11; spec (4.7), (4.9) |
| Lemma 3.28 | (pure-state trace distance) | errata E-11; review 2.9 |
| Definition 3.29 | (sentence contraction) | spec (1.5), C7, l.285; review 2.1 |
| Lemma 3.30 | (sigma is a projection coordinate vector) | errata E-1; spec (4.8); review 2.1 |
| Lemma 3.31 | (Haar second moment) | errata E-1 (Reason: Haar covariance) |
| Theorem 3.32 | (law of Z for a Haar-random verb) | errata E-1, replacing spec l.218; spec l.285, l.858, l.970, T6 (l.1272); addendum l.705-712, l.786-788; review 2.1 |
| Corollary 3.33 | (general forms) | limits l.518, l.832, l.1218; errata E-1 |
| Corollary 3.34 | (distribution of Z relative to the p_min floors) | spec Sec. 4.5 rule (3), l.863-864; errata E-1, E-11 |
| Remark 3.35 | (what is and is not proved about Z) | errata E-1, E-10; review 2.1; limits L.1-L.2; addendum (13.10'), (11.25), (11.26) |
| Definition 3.36 | (pure-state fidelity; Gram matrix) | spec Sec. 1.5, Sec. 9 item 7; limits G.1-G.3 |
| Definition 3.37 | (density operators; partial trace) | spec Sec. 1.5; addendum (13.10), (14.19); limits T.1, SF.1 |
| Lemma 3.38 | (properties of the partial trace) | limits T.1, HO.7, HO.8; addendum (14.18) |
| Theorem 3.39 | (Schmidt decomposition; entropy; mutual information) | addendum (14.15), (14.16); limits SF.6, T-W4 |
| Definition 3.40 | (spectral functionals) | limits SF.4, SF.5; addendum Sec. 14.6 |
| Theorem 3.41 | (Uhlmann fidelity for qubits) | spec Sec. 1.5; addendum Sec. 14.6; limits SF.5 |
| Lemma 3.42 | (pure states; Fuchs-van de Graaf) | limits G.3, T-SF2 |
| Lemma 3.43 | (entropy examples) | limits T-SF3 |
| Definition 3.44 | (Loewner order; graded hyponymy) | limits SF.4, SF.5; Bankova et al. arXiv:1601.04908; Balkir et al. arXiv:1506.06534 |
| Proposition 3.45 | (computing k_max) | limits SF.5 |
| Lemma 3.46 | (Z-basis correlators by the Walsh-Hadamard transform) | limits T.3, T.4; spec Sec. 2.1 |
| Remark 3.47 | (completely positive word updates) | limits NU.1-NU.4 |
| Remark 3.48 | (further exact identities) | limits G.5, G.6, HO.5, HO.7, HO.8, HO.10, B3; addendum (12.15), (12.19); review 5(c) |
| Definition 4.1 | (tensor with fields) | spec C7, Sec. 1.3 |
| Lemma 4.2 | (mixed radix) | addendum (12.16'), T16 |
| Remark 4.3 | (scope of radix generality) | spec Sec. 1.6; limits Sec. 2.7 (7) Test A |
| Definition 4.4 | (monolithic sentence state) | spec (1.3), (1.4), (1.6), (1.7) |
| Proposition 4.5 | (density-matrix model) | spec Sec. 1.6; addendum (13.10) |
| Lemma 4.6 | (contraction is a matrix product) | spec (2.8) |
| Definition 4.7 | (fused contraction step) | spec Sec. 1.3 |
| Theorem 4.8 | (the sweep computes sigma) | spec Sec. 1.3, (1.3) |
| Proposition 4.9 | (storage of a step) | spec Sec. 1.3, Sec. 2.3 |
| Lemma 4.10 | (in-place slot rule, corrected) | spec Sec. 1.3 l.258; errata E-13 |
| Proposition 4.11 | (N TV N storage and cost) | spec Sec. 1.3, Sec. 2.3, Sec. 7; addendum (12.11); limits L.3 |
| Remark 4.12 | (fold sequences and the zero-angle value) | spec Sec. 1.3, Sec. 8, T5, T6; limits G.5 |
| Definition 4.13 | (word graph; node operation) | addendum C16, Sec. 11.7 |
| Theorem 4.14 | (peak recurrence) | addendum (11.18'), (11.19) |
| Proposition 4.15 | (optimal child order; corrects the rule of (11.18')) | addendum (11.18') (corrects its ordering rule) |
| Proposition 4.16 | (crude bound) | addendum (11.20) |
| Proposition 4.17 | (cycles) | addendum Sec. 11.4, (11.18'') |
| Theorem 4.18 | (cut-width bound for the sweep) | addendum (11.21) |
| Proposition 4.19 | (spider node) | addendum (11.7), (11.9), C15 |
| Definition 4.20 | (MPS) | addendum (14.6), (14.1) |
| Proposition 4.21 | (bond dimension) | addendum (14.7) |
| Theorem 4.22 | (Eckart-Young-Mirsky; Eckart and Young 1936, Mirsky 1960) | addendum (14.10)-(14.12) |
| Theorem 4.23 | (bond-centred truncation is optimal) | addendum (14.10)-(14.12) |
| Proposition 4.24 | (two-site gate update) | addendum (14.8)-(14.12), C6, C17, T21 |
| Proposition 4.25 | (relative discarded weight; accumulated error) | addendum (14.12) (relative-weight correction) |
| Remark 4.26 | (moving the centre; purification; tangents) | addendum (14.10)-(14.11) |
| Theorem 4.27 | (entanglement across a bond) | addendum (14.15), (14.16); limits SF.6 |
| Proposition 4.28 | (the exact rank a text needs) | addendum Sec. 14.4, (14.17) |
| Proposition 4.29 | (two-wire reduced density matrix) | addendum (14.19'), (14.12) |
| Remark 4.30 | (traced readout) | spec Sec. 1.5; limits l.715 |
| Definition 4.31 | (tensor-train verb) | addendum (12.13), T17 |
| Lemma 4.32 |  | addendum (12.13), (12.14), T17 |
| Lemma 4.33 | (degenerate verbs) | limits Sec. 2.7 (7), Sec. 2.8 (7) |
| Definition 4.34 | (text state) | addendum (14.1)-(14.5), (12.17) |
| Proposition 4.35 | (register width) | addendum (14.5), (12.18) |
| Proposition 4.35' | (two-layer peak storage) | addendum (12.17), (12.18) |
| Definition 4.36 | (mean-field state; product approximation) | addendum Sec. 13.2(a) |
| Proposition 4.37 | (four different approximations) | addendum Sec. 13.2(a), MPS rows |
| Remark 4.38 | (status) | (status of Chapter 4) |
| Definition 5.1 | (postselected readout and losses) | spec (5.1), Sec. 4.5, Sec. 1.4 |
| Lemma 5.2 | (derivative of a gate) | spec C13; limits Sec. 2.6 (1) |
| Lemma 5.3 | (frequency content of a gate expectation) | spec (5.2), (5.4), Sec. 5.2 |
| Lemma 5.4 | (spectrum of the controlled generator) | spec C13 |
| Theorem 5.5 | (exactness criterion for shift rules) | spec (5.2), (5.4) |
| Theorem 5.6 | (two-term rule) | spec (5.2), C13 |
| Theorem 5.7 | (four-term rule for spectrum {0, +1, -1}) | spec (5.4), C13 |
| Remark 5.8 | (normalisations) | spec Sec. 5.2; limits Sec. 2.7 (6) |
| Corollary 5.9 | (chain-rule alternative) | spec (5.5) |
| Proposition 5.10 | (quotient rule; 1/D bound) | spec (5.3), Sec. 5.1; errata E-10; review 2.8(D) |
| Proposition 5.11 | (propagation of readout errors: two models) | spec (4.7), (5.3); errata E-11 |
| Definition 5.12 | (cotangent of a complex vector) | addendum (11.27); spec (5.8) |
| Lemma 5.13 | (transport rules) | addendum (11.27); spec (5.6)-(5.8) |
| Theorem 5.14 | (adjoint differentiation) | spec (5.6), (5.7); errata Sec. B; limits Sec. 2.6 (1) |
| Corollary 5.15 | (seed for the postselected loss) | spec (5.8); addendum (11.27') |
| Proposition 5.16 | (operation counts of the adjoint sweep) | spec Sec. 5.3; addendum (11.23'); errata Sec. B |
| Proposition 5.17 | (complex Jacobian by the same sweep) | limits Sec. 2.6 (3)(a); spec (5.8) |
| Lemma 5.18 | (transposes and conjugates; general rule) | spec C11; limits NU.5 |
| Proposition 5.19 | (sign rule for transposed effect words) | spec C11, Sec. 5.3, C13, T7; limits NU.5 |
| Lemma 5.20 | (shared parameters) | spec Sec. 5.2, Sec. 5.5; errata Sec. B; limits "weight tying" |
| Proposition 5.21 | (periodicity of the loss) | spec Sec. 5.6, C5, T10; review 2.8(G) |
| Definition 5.22 | (Adam) | spec (5.9) |
| Corollary 5.23 | (wrapping mod 4 pi) | spec Sec. 5.5, 5.6, 6.3 |
| Definition 5.24 | (node operation) | addendum Sec. 11.7; spec Sec. 1.3 |
| Theorem 5.25 | (tree adjoints) | addendum (11.27)-(11.29), (11.7), (11.9), (11.18''), T24 |
| Theorem 5.26 | (angle gradients on the tree) | addendum (11.30); spec Sec. 5.3, 5.5 |
| Proposition 5.27 | (counts for the tree adjoint) | addendum (11.31), (11.23''), Sec. 16; T24(b) |
| Remark 5.28 | (shift rules on the tree form) | addendum Sec. 11.7, T24; spec Sec. 5.2 |
| Lemma 5.29 | (normalisation and projective invariance) | limits G.9, L.1, L.4, B1-B3; addendum (11.27') |
| Corollary 5.29' | (seeds for Gram, InfoNCE and fidelity losses) | limits G.4, G.7, G.8, L.4 |
| Proposition 5.30 | (functionals of a reduced density matrix) | limits SF.7, T.5, NU.6-NU.7; spec Sec. 5.1 |
| Remark 5.31 | (shift rules for these functionals) | limits T.5, G.7-G.8; addendum (13.12) |
| Definition 5.32 | (SPSA) | spec Sec. 5.4 |
| Theorem 5.33 | (unbiasedness and exact variance for a linear objective) | errata E-2, replacing spec l.1051-1052; review 2.2 |
| Remark 5.34 |  | review 2.2 |
| Proposition 5.35 | (SPSA bias) | errata E-2; review 2.2; spec l.1049 |
| Definition 5.36 |  | limits Sec. 2.6 (3); Stokes, Izaac, Killoran, Carleo, Quantum 4, 269 (2020) |
| Proposition 5.37 | (Fubini-Study metric from the Jacobian) | limits Sec. 2.6 (3)(b) |
| Remark 5.38 | (batch rank) | limits Sec. 2.6 (3) |
| Proposition 5.39 | (Hessians) | limits Sec. 2.6 (3) |
| Theorem 5.40 | (critical point at the origin for real-generator ansatze) | errata E-10; review 2.8(E); spec l.1072 |
| Theorem 5.41 | (the IQP saddle for the (1,1) transitive sentence) | errata E-10; review 2.8(E); spec Sec. 8 |
| Remark 5.42 | (scope) | errata E-10 |
| Theorem 5.43 | (exact E(D) = 1/4 at sign-symmetric initialisation) | errata E-10, E-1; review 2.8(E); spec l.970 |
| Remark 5.44 | (trainability: proved and conjectured) | errata E-10; review 2.8(A)-(D), (F); spec l.1072-1074 (superseded) |
| Definition 6.1 | (floating-point model) | spec C9, C12 |
| Definition 6.2 | (fixed-point formats) | spec Sec. 4.3; errata E-6, Sec. B |
| Lemma 6.3 | (exact operations) | spec Sec. 4.2, (4.2), (4.5) |
| Lemma 6.4 | (dot-product error) | spec (4.2) |
| Lemma 6.5 | (butterfly row as real dot products) | spec (4.1) |
| Theorem 6.6 | (per-gate constant) | spec (4.2), Sec. 4.6, Sec. 10 item 7 |
| Remark 6.7 | (fp16 quantisation of an m-qubit block) | spec Sec. 3.3 (l.728), Sec. 4.6 |
| Lemma 6.8 | (nothing grows) | spec Sec. 4.1, (4.3) |
| Theorem 6.9 | (accumulated error) | spec (4.3) |
| Remark 6.10 | (random-walk estimates are not bounds) | spec (4.3), Sec. 4.6, Sec. 10 item 7 |
| Lemma 6.11 | (angle sensitivity; the half-angle turn code) | spec C5, Sec. 4.2, Sec. 5.6, Sec. 6.3; addendum (13.2); spec T10 |
| Lemma 6.12 | (octant reduction; polynomials; table) | spec Sec. 4.4, C5 |
| Theorem 6.13 | (intermediates are bounded; overflow is impossible) | spec (4.1), (4.5), Sec. 4.3; errata E-6, Sec. B |
| Theorem 6.14 | (saturation is non-expansive; pre-scale invariance) | errata E-6; spec Sec. 4.3, Sec. 4.5, l.866; review 2.4 |
| Remark 6.15 | (the S15 initialisation) | spec l.801-803, l.865-867; errata E-6; review 2.4 |
| Remark 6.16 | (truncation bias; a heuristic) | spec (4.5) |
| Proposition 6.17 | (per-gate fixed-point error) | spec (4.6), Sec. 4.3, T8 |
| Remark 6.18 | (format comparison) | spec Sec. 4.3, (4.6), (4.2) |
| Remark 6.19 | (flush-to-zero; UNVERIFIED rounding mode) | spec Sec. 4.2 (l.796), Sec. 10 item 7 |
| Theorem 6.20 | (range of every intermediate) | spec (4.8), Sec. 1.3, Sec. 6.2; errata E-6 |
| Corollary 6.21 | (accumulator bound) | spec (4.8), (4.5), Sec. 4.7; review 2.4 |
| Lemma 6.22 | (narrowing without pre-scale) | errata E-6; spec l.930; review 2.4 |
| Theorem 6.23 | (Lipschitz recurrence for contraction chains) | spec (4.9), (4.8); errata E-11; addendum (12.16); review 2.9 |
| Remark 6.24 | (non-unit factors) | review 2.9, 5(e); errata E-11; addendum (12.16) |
| Corollary 6.25 | (tensordot error budgets) | spec Sec. 4.7, l.930-953, T6; errata E-6 |
| Theorem 6.26 | (probability of a projector) | spec (4.4), Sec. 4.5 |
| Theorem 6.27 | (postselection amplification; tight constant) | spec (4.7), (4.9), l.856, l.926; errata E-11; review 2.9; spec T8 |
| Corollary 6.28 | (survival floors; amplification factor) | spec Sec. 4.5 rule (3), Sec. 4.6, l.935-952, Sec. 6.2, T8; errata E-11; review 2.9 |
| Remark 6.29 | (amplification heuristics) | spec Sec. 2.3, Sec. 4.5, l.858; addendum (13.6), (13.7), l.708, l.788; errata E-1, E-11 |
| Corollary 6.30 | (truncations and gates) | addendum (14.12); spec Sec. 4.1 |
| Lemma 6.31 | (the RNE absorption rule) | errata E-8, replacing spec C12 l.113; review 2.5 |
| Proposition 6.32 | (fp16 constant series) | errata E-7, replacing spec T8 l.1281-1282; spec l.850; review 2.5 |
| Lemma 6.33 | (reduction error; exact squares) | spec Sec. 4.5, C12, T8 |
| Remark 6.34 | (why probabilities are not accumulated in fp16) | spec C12, Sec. 4.5; limits precision rules |
| Proposition 6.35 | (integer angle codes) | spec Sec. 6.3, C5, T10; addendum (13.2) l.891-894, (13.12), T19 |
| Proposition 6.36 | (perturbation of a bilinear map) | addendum l.1065-1067 |
| Remark 6.37 | (statements verified numerically, not used here) | addendum (14.10), (14.13), (14.14), T15, (13.4), (13.4'), T19; limits SF.1-SF.3, B9 |
| Definition 7.1 | (pure-state manifolds) | addendum (12.1), (12.2) |
| Theorem 7.2 |  | addendum (12.1), (12.2) |
| Definition 7.3 | (realised degrees of freedom) | addendum (12.5) |
| Lemma 7.4 |  | addendum (12.5) |
| Proposition 7.5 | (parameter counts) | spec C10, Sec. 5.5, Sec. 7 |
| Theorem 7.6 | (necessary depth) | addendum (12.3) |
| Corollary 7.7 | (comparison of the depth bounds) | addendum (12.3) |
| Proposition 7.8 | (reality) | addendum (12.3); spec C10 |
| Proposition 7.9 | (Q = 1 words) | addendum (12.3) |
| Theorem 7.10 | (IQP, ansatz 0, L = 1: flat moduli) | addendum (12.4) |
| Corollary 7.11 |  | limits l.939; spec C10 |
| Remark 7.12 | (numerical ranks; UNVERIFIED as theorems) | addendum Sec. 12.1, l.600-606 |
| Conjecture 7.13 |  | addendum Sec. 19 item 3, Sec. 13.2(c); limits l.324 |
| Proposition 7.14 | (parameters versus coordinates) | addendum (12.20), Sec. 12.1 after (12.5); limits l.1125-1127, l.1218 |
| Lemma 7.15 | (spectral form of gates) | spec C5, C13 |
| Theorem 7.16 | (the readout as a rational trigonometric function) | addendum (12.6)-(12.8) |
| Corollary 7.17 | (repeated words) | addendum (12.6); spec (5.2), C13, Sec. 5.5 |
| Theorem 7.18 | (data re-uploading) | addendum (13.11) |
| Proposition 7.19 | (the ceiling of any front end) | addendum (13.1), (13.1'), (13.8), (13.9), Sec. 15.3(d) |
| Remark 7.20 |  | addendum Sec. 19 items 6, 7 |
| Theorem 7.21 | (affinity) | addendum (15.1) |
| Definition 7.22 | (slot degree) | addendum 15.2(c) |
| Theorem 7.23 | (no switching inside one evaluation) | addendum (15.1), 15.2(c) |
| Proposition 7.24 | (idealised transition law) | addendum 15.2(c) ('factor 3') |
| Remark 7.25 | (priced escapes) | addendum 15.2(c), (11.25), (14.18), (13.2c) |
| Proposition 7.26 | (the ceiling depends on d) | review 4; addendum (12.9), l.666 |
| Remark 7.27 | (interpretive items) | addendum l.666, Sec. 15.2(d,e), 15.5 |
| Proposition 7.28 |  | addendum (12.10) |
| Remark 7.29 | (numerical instance) | spec Sec. 8; addendum (12.10), Sec. 13.6(ii) |
| Proposition 7.30 |  | addendum (12.13), (12.20) |
| Theorem 7.31 | (gradient variance under a 2-design) | errata E-10; spec Sec. 5.5 |
| Corollary 7.32 | (projector versus Pauli; which W) | errata E-10; spec Sec. 5.5 |
| Remark 7.33 | (what is and is not proved) | errata E-10; limits l.1094 |
| Theorem 7.34 | (the IQP origin; with a correction to errata E-10) | errata E-10 |
| Remark 7.35 | (counting identities; cross-reference only) | spec Sec. 7; addendum (12.11), (12.14); review 2.3, 5(d); errata E-4 |

## A.2 Engineering label -> result proving it

Each engineering label is marked as one of:

- the result(s) proving it;
- "definition", if nothing needs proving;
- "corrected by", if the result proves a corrected form;
- "asserted only", if no result here proves it;
- "engineering", if it has no mathematical content.

### Specification (spec)

| Label | Status in this document |
|---|---|
| (0.1)-(0.3) Rx, Ry, Rz | Lemma 3.8 |
| (0.4), (0.10) fixed matrices | Definition 3.7 |
| (0.5)-(0.8), (0.11) CNOT, CZ, CRz, CRx | Lemmas 3.12, 3.13 |
| (0.9) field widths | Definition 2.14 |
| (0.12) lifts | Lemma 3.11 |
| (1.1) pregroup reduction | Definition 1.1, Corollary 1.5, Theorem 1.7, Proposition 2.7 |
| (1.2) word state | Definition 2.19 |
| (1.3) component formula | Theorem 2.20, Proposition 2.18, Theorem 4.8 |
| (1.4) Z, P_post | Lemma 2.12, Theorem 2.24, Lemma 3.26(c), Corollary 3.33 |
| (1.5) N TV N contraction | Theorem 2.20, Definition 3.29 |
| (1.6) Born readout | Definition 3.29, Definition 4.4 (the traced readout differs: Remark 4.30) |
| (1.7) regularised readout and loss | definition (Definition 4.4) |
| (2.1) butterfly | Lemma 3.10 |
| (2.2)-(2.4) i0, i00, compact | Definition 3.5, Lemma 3.6 |
| (2.5), (2.5') compaction | Lemma 3.25 (D = p_surv under (H-readout)) |
| (2.6) Bell gather | Lemmas 2.12, 2.13, 3.26; Proposition 2.25(iv) |
| (2.7) word adjoined in high bits | Corollary 3.4 |
| (2.8) matrix-product form | Lemma 4.6 |
| (3.1) storage layouts | Remark 3.14 |
| (4.1) fixed-point butterfly | Lemma 6.5, Theorem 6.13 |
| (4.2) per-gate constant | Theorem 6.6 (with hypothesis (NU)) |
| (4.3) accumulated error | Theorem 6.9; the random-walk figures are heuristic (Remark 6.10) |
| (4.4) projector probability | Theorem 6.26 |
| (4.5) Q1.15 store rule | Theorem 6.13(c); truncation variant, Remark 6.16 |
| (4.6) g_Q15 | Proposition 6.17 |
| (4.7) amplification with constant 4 | corrected by Theorem 6.27 (constant 1), Lemmas 3.27-3.28 |
| (4.8) tensordot range | Theorem 6.20, Corollary 6.21, Lemma 3.30 |
| (4.9) tensordot propagation | corrected by Theorems 6.23, 6.27 |
| (5.1) postselected ratio | Definition 5.1 |
| (5.2) two-term shift | Theorem 5.6 (exact only for spectrum {+-1}: Corollary 7.17) |
| (5.3) quotient rule | Proposition 5.10 |
| (5.4) four-term shift | Theorem 5.7 |
| (5.5) CRz decomposition | Lemma 3.20, Corollary 5.9 |
| (5.6), (5.7) adjoint | Theorem 5.14 |
| (5.8) loss seed | Corollary 5.15 |
| (5.9) Adam | Definition 5.22, Corollary 5.23 |
| C1, C2 | Definitions 3.1, 3.2; Section 0.1 |
| C3, C4 | Section 0.1 |
| C5 | Lemmas 3.8, 3.16, 6.11 |
| C6 | Lemmas 3.11, 3.12 |
| C7 | Definition 2.14, Lemma 2.15 |
| C8 | Proposition 2.11, Lemmas 2.12, 2.29 (which pairing the reference implementation uses: UNVERIFIED) |
| C9 | Section 0.1; Definition 6.1 |
| C10 | Definition 3.17, Proposition 7.5 |
| C11 | Lemmas 2.22, 3.18; Proposition 5.19 |
| C12 | corrected by Lemma 6.31 (E-8); Lemma 6.33 |
| C13 | Lemmas 3.13, 5.4, 7.15 |
| T1 | Lemma 3.6 |
| T2 | Definition 3.9, Lemma 3.10 |
| T3 | Lemmas 3.18, 3.20 |
| T4 | Lemma 3.21, Theorem 3.22 |
| T5 | Theorem 2.24, Corollary 2.26 |
| T6 | Theorem 2.24, Remark 4.12, Theorem 3.32 |
| T7 | Proposition 5.19; the gradient values are numerical checks |
| T8 | Theorem 6.27, Proposition 6.32 |
| T9 | engineering |
| T10 | Lemmas 1.32, 6.11; Proposition 6.35 |
| l.150 switching lemma | Lemma 1.4 (cited); Proposition 2.18 (order of cups) |
| l.162-179 stack reducer | Theorem 1.20, Propositions 1.21-1.25 (the spec's justification is corrected in Proposition 1.23) |
| l.217-218 E[Z] = 1 | corrected by Theorem 3.32 (E-1) |
| l.258 in-place slot rule | corrected by Lemma 4.10 (hypothesis needed) |
| l.796 flush-to-zero value | corrected by Remark 6.19 (value inconsistent with its own formula) |
| l.858, l.863-864 amplification, floors | Remark 6.29, Corollary 3.34, Corollary 6.28 |
| l.970 E[D] = 2^{-|K|} | Theorem 3.32 (Haar verb); Theorem 5.43 ((1,1) IQP); not general (E-1) |
| l.1051-1052 SPSA variance | corrected by Theorem 5.33 (E-2) |
| l.1072-1074 initialisation, barren plateau | corrected by Theorems 5.40, 5.41, 7.34; Corollary 7.32 |
| l.1281-1282 fp16 stall | corrected by Proposition 6.32 (E-7) |
| Sec. 1.4 bent wire | Theorem 2.24, Proposition 2.25 |
| Sec. 1.5 fidelity, mixed readout | Definitions 3.36, 3.37; Theorem 3.41; Remark 4.30 |
| Sec. 1.6 radix-d model | Remark 4.3, Proposition 4.5 |
| Sec. 7 memory tables | engineering; amplitude counts in Remark 7.35 |
| Sec. 9 item 1 (parser) | Proposition 1.23 |
| Sec. 10 item 8 (treewidth) | open (Appendix B) |

### Addendum

| Label | Status in this document |
|---|---|
| (11.1)-(11.3) | Definition 1.1, Lemma 1.4 |
| (11.4), (11.5) | Definition 1.9, Theorem 1.10 (l.100 omits the target filter) |
| (11.6) | Proposition 2.28, Lemma 2.29 |
| (11.7)-(11.9) | Lemma 2.30, Propositions 2.40(iv), 4.19 |
| (11.10)-(11.13') | Definition 2.31 |
| (11.12') sum semantics | Proposition 2.33 |
| (11.14)-(11.17') | Proposition 2.32 |
| (11.14') THAT orientation | Corollary 2.32' |
| (11.18') | Theorem 4.14; ordering rule corrected by Proposition 4.15 |
| (11.18'') | Proposition 4.17 (wrapper term of a chain bound) |
| (11.19) | Theorem 4.14 |
| (11.20) | Proposition 4.16 |
| (11.21) | Theorem 4.18 (correct as stated, with D = nd) |
| (11.22) | Proposition 2.25, Lemma 2.13; the spider case is Conjecture 2.34 |
| (11.23) | Definition 1.27, Proposition 1.28 |
| (11.23') | Proposition 5.16 |
| (11.23'') | W_train = sum over STATE words of Q_w is W_hw of Theorem 2.24; the instance believes/thinks/sleeps (8 at (1,1), 11 at (2,1)) follows from Proposition 2.25(ii); the growth law W_train ~ (n_unitary/2) mean Q_w is a heuristic (UNVERIFIED): a tree 2-colouring can make the STATE class anywhere from 1 to n_unitary - 1 words (Appendix B) |
| (11.25), (11.26) | Propositions 1.12, 1.16; Definition 1.17 |
| (11.27)-(11.29) | Definition 5.12, Theorem 5.25 |
| (11.27') | Corollary 5.15, Lemma 5.29 |
| (11.30) | Theorem 5.26 |
| (11.31) | Proposition 5.27 |
| C14 | engineering (units) |
| C15 | Definitions 1.3, 1.31, 2.31; Lemma 1.32 |
| C16 | Definition 4.13 |
| C17 | Definition 2.35, Lemma 2.36, Proposition 4.24 |
| (12.1), (12.2) | Theorem 7.2 |
| (12.3) | Theorem 7.6, Corollary 7.7, Proposition 7.9 |
| (12.4) | Theorem 3.22, Theorem 7.10 |
| (12.5) | Definition 7.3, Lemma 7.4 |
| (12.6)-(12.8) | Theorem 7.16, Corollary 7.17 |
| (12.9) | Theorem 7.21; "independent of q_n, q_s, L" is qualified by Proposition 7.26 |
| (12.10) | Proposition 7.28 |
| (12.11), (12.11') | Proposition 4.11 (amplitude counts); bytes are engineering |
| (12.12) | engineering |
| (12.13) | Definition 4.31, Proposition 7.30 |
| (12.14) | Lemma 4.32(c) |
| (12.15) | Remark 3.48(vi) |
| (12.16) | Theorem 6.23, Remark 6.24 |
| (12.16') | Lemma 4.2 |
| (12.17) | Definition 4.34, Proposition 4.35' |
| (12.18) | Proposition 4.35' (under its stated hypotheses) |
| (12.19) | Remark 3.48(v) |
| (12.20) | Propositions 7.14, 7.30 |
| (13.1), (13.1'), (13.8), (13.9) | Proposition 7.19 |
| (13.2) | Lemma 6.11, Proposition 6.35 |
| (13.2c) | Remark 7.25 |
| (13.3) | engineering |
| (13.4), (13.4') | engineering (Remark 6.37) |
| (13.5), (13.6) | Definition 2.35 |
| (13.7) | Proposition 2.40(v) |
| (13.10) | Proposition 4.5, Definition 3.37 |
| (13.10') | Remark 3.35(v) |
| (13.11) | Theorem 7.18 |
| (13.12) | chain rule (Remark 5.31) |
| l.1065-1067 int8 export bound | corrected by Proposition 6.36 (second-order term missing) |
| (14.1), (14.2) | Definition 2.35, Lemma 2.36 |
| (14.3) | Proposition 2.37 |
| (14.4) | Remark 2.21, Proposition 2.37 |
| (14.5) | Proposition 4.35 |
| (14.6) | Definition 4.20 |
| (14.7) | Proposition 4.21 |
| (14.8), (14.9) | Proposition 4.24 |
| (14.10)-(14.12) | Theorem 4.23; Propositions 4.24, 4.25; Corollary 6.30 ((14.12) assumes unit norm) |
| (14.13), (14.14) | algorithms, numerically checked (Remark 6.37) |
| (14.15), (14.16) | Theorems 3.39, 4.27 |
| (14.17) | Proposition 4.28 (swaps counted per crossing; recall statement replaced by (d)) |
| (14.18) | Lemma 3.38(e) |
| (14.19) | Definition 3.37, Remark 7.27 |
| (14.19') | Proposition 4.29 |
| (15.1) | Theorems 7.21, 7.23 |
| 15.2(c) | Theorem 7.23, Proposition 7.24, Remark 7.25 |
| Sec. 15.3(b), (d), (e) figures | UNVERIFIED (Remark 7.35) |
| T11 | Theorem 1.10, Proposition 1.24 |
| T12 | Proposition 2.32 |
| T13 | Lemma 2.30, Proposition 2.32 (reference decimals checked numerically) |
| T14 | Corollary 2.32' |
| T15 | Remark 6.37 |
| T16 | Lemma 4.2 |
| T17 | Lemma 4.32 |
| T18 | Conjecture 2.34 (checked instance) |
| T19 | Proposition 6.35, Remark 6.37 |
| T20 | Lemma 2.36 (argument order, C17) |
| T21 | Proposition 4.24 |
| T22 | Theorem 4.14 |
| T23 | Theorem 3.32 (Monte Carlo consistent) |
| T24 | Theorem 5.25, Proposition 5.27, Remark 5.28 |

### Errata v1.1

| Item | Status in this document |
|---|---|
| E-1 E[Z] = 2^{-2 q_n} | Theorem 3.32; Corollaries 3.33, 3.34 (E-1's "47 %" should read 46 %); Remarks 3.35, 6.29 |
| E-2 SPSA variance without 1/c^2 in the intrinsic term | Theorem 5.33, Remark 5.34, Proposition 5.35 |
| E-3 | engineering |
| E-4 | Remark 7.35 (the Feynman-path statement is standard) |
| E-5 | engineering (clarification) |
| E-6 pre-scale, narrowing | Definition 6.2, Theorems 6.13, 6.14, Lemma 6.22, Corollary 6.25 |
| E-7 | Proposition 6.32 |
| E-8 | Lemma 6.31 |
| E-9 | engineering (layout); the index maps are covered by Remark 3.14 |
| E-10 | Proposition 5.10; Theorems 5.40, 5.41, 5.43, 7.34; Corollary 7.32. The claim "diagonal Hessian 0 for every IQP sentence" is refuted by Theorem 7.34(d) |
| E-11 tight constant 1 | Lemmas 3.27, 3.28; Theorem 6.27; Corollary 6.28; Proposition 5.11 |
| E-12 | Theorems 1.7, 1.10, 1.11; Propositions 1.15, 1.21-1.25; Conjecture 1.26. E-12 repeats the spec's false justification, which Proposition 1.23 corrects. Its O(M^3) for the lattice chart holds for fixed k (Proposition 1.15 proves O(k^2 M_lat^3)) |
| E-13 | Lemma 4.10 (needs the hypothesis restT or restW empty) |
| Sec. B | Lemma 1.4, Theorem 5.14, Lemma 5.20, Lemma 3.26 |

### Review of companion v1.1

| Item | Status in this document |
|---|---|
| 2.1 | Theorem 3.32 |
| 2.2 | Theorem 5.33, Proposition 5.35 |
| 2.3, 5(d) | Remark 7.35 |
| 2.4 | Theorems 6.14, 6.20; Corollary 6.21; Lemma 6.22 |
| 2.5 | Lemma 6.31, Proposition 6.32 |
| 2.8 | Propositions 5.10, 5.21; Theorems 5.40-5.43 |
| 2.9 | Lemma 3.27; Theorems 6.23, 6.27 |
| 2.10 | Chapter 1 (Theorems 1.7-1.11, Propositions 1.15-1.25) |
| 2.11 | Lemma 3.26 |
| 3 item 15 | Theorem 3.22 |
| 4 | Proposition 7.26 |
| 5(a) | Propositions 2.43, 2.44, 7.28 |
| 5(c) | Remark 3.48(viii) |
| 5(e) | Remark 6.24 |
| 7 P0 | Proposition 1.23 and the remark after it |
| 8 | Theorem 1.13 |

### Hardware-limits (limits)

| Label | Status in this document |
|---|---|
| B1-B3 | Definition 1.17, Lemma 5.29, Remark 3.48(vii) |
| G.1-G.3 | Definition 3.36, Lemma 3.42 (G.2's matrix form corrected) |
| G.4 | definition (the InfoNCE loss; its derivatives are Corollary 5.29'(b)) |
| G.5, G.6 | Remark 3.48(i), Remark 4.12 |
| G.7, G.8 | Corollary 5.29'(a), (b) |
| G.9 | Lemma 5.29 |
| L.1-L.3 | Remark 3.35, Lemma 5.29, Proposition 4.11 |
| L.4 | Corollary 5.29'(c) |
| SF.1-SF.3 | Remark 6.37 |
| SF.4, SF.5 | Definition 3.40, Theorem 3.41, Proposition 3.45 |
| SF.6 | Theorems 3.39, 4.27 |
| SF.7 | Proposition 5.30 |
| T.1-T.5 | Definition 3.37, Lemmas 3.38, 3.46, Proposition 5.30 |
| T-SF2, T-SF3 | Lemmas 3.42, 3.43 |
| T-W4 | Theorem 3.39 |
| NU.1-NU.4 | Remark 3.47 |
| NU.5 | Lemma 3.18, Proposition 5.19 |
| NU.6, NU.7 | Proposition 5.30 |
| HO.1, HO.2 | Remark 3.14 |
| HO.2-HO.4 | Definition 2.41 |
| HO.5, HO.7, HO.8, HO.10 | Remark 3.48; Lemma 3.38 |
| Sec. 2.6 (1)-(3) | Lemma 5.2; Propositions 5.17, 5.37, 5.39 |
| Sec. 2.7 (6) | Remark 5.8 |
| Sec. 2.7 (7) Test A; Sec. 2.8 (7) | Remark 4.3, Lemma 4.33 |
| l.518, l.832, l.1218 | Corollary 3.33 (l.832 concerns cups) |
| l.715 | Remark 4.30 (it is not the (1.6) readout) |
| l.939 | Corollary 7.11 (Z-basis marginals only) |
| l.1094 | Remark 7.33 |
| l.1125-1127 | Proposition 7.14(c) |
| l.1463 | Proposition 1.18 |
| precision rules | Remark 6.34 |

# Appendix B. Conjectures, UNVERIFIED statements and open problems

**B.1 Grammar**

1. Conjecture 1.26 (characterisation of greedy failure without VP coordination).
2. Conjecture 1.30: uniqueness of the reductions of Rt_k and Ce_k for general k (k = 1, 2, 3 are certified by Theorem 1.10); +2 depth per subject-relative level. The attribution of the aux-inverted question assignment of Example 1.30' to Lambek (2008).
3. #P-hardness of counting crossing cup sets for the graphs G(X) (remark after Proposition 1.14).
4. A lower bound for every backtracking scheme (remark after Proposition 1.25).
5. m ~ 2.3 x words (remark after Theorem 1.11).
6. Citation details of Lambek 1999 and Buszkowski 2001 (Lemma 1.4, Theorem 1.13) and of Lambek 2008.

**B.2 Categories**

7. Citation details of Preller-Lambek and Joyal-Street (Definition 2.8), of the spider theorem, and of Sadrzadeh-Clark-Coecke (Definition 2.31).
8. Conjecture 2.34: realisation of spiders in the bent-wire circuit with factor 2^{-q/2} per e_F leg and per Bell cup.
9. Which cup pairing (component-wise or nested) the reference implementation uses (C8).
10. The DisCoCirc gate set of the literature; the Coecke-Meichanetzidis wording (Definition 2.41).
11. The qudit analogues of the circuit results (Remark 2.42).

**B.3 Hilbert-space semantics**

12. Ring directions of Sim14/Sim15, and the closing H layer of external IQP implementations (Definition 3.17).
13. Reck/Clements surjectivity details (Remark 3.23).
14. A structural explanation of the IQP mean Z = 0.041 at (2,1) and of near-zero Z (Remark 3.35(ii)).
15. That training raises Z above 2^{-2 q_n} (Remark 3.35(iii), Remark 7.20).
16. The O(1/n_MC) bias of ratio estimators (Remark 3.35(v)).
17. Literature attributions of the word-update operations (Remark 3.47).

**B.4 Tensor networks**

18. At most one back edge per wrapper; no nested cycles in the Sec. 11.2 lexicon; a spider wrapper costs nothing extra (Proposition 4.17(c)).
19. Treewidth of word graphs for general lexica [spec Sec. 10 item 8].
20. The size of the effect of inexact SVD tangents (Remark 4.26).
21. The per-site sampling constant (Remark 4.30).
22. The growth law W_train ~ (n_unitary/2) mean Q_w of addendum (11.23''), a heuristic: a tree 2-colouring can make the STATE class anywhere from 1 to n_unitary - 1 words (Appendix A.2).
23. Global best product approximation. The problem is non-convex, and sequential truncation is not optimal (Proposition 4.37(b)); no algorithmic guarantee is claimed.

**B.5 Differentiation and trainability**

24. Equality of the last two gradients of "Alice sleeps and dreams" (Remark 5.28).
25. Absence of barren plateaus for the normalised readout, Var[dp_c/dtheta] bounded below as W grows (Remark 5.44(iii)).
26. A saddle at the origin for every IQP profile beyond (1,1) and (2,1) (Theorem 7.34(d)).
27. The Lie-algebraic variance prediction (Remark 7.33).

**B.6 Numerics**

28. The sharper table bound 6.03e-5 < 2^-14 (Lemma 6.12). This is a finite exhaustive check whose enumeration is not reproduced here.
29. Behaviour under flush-to-zero (Remark 6.19).
30. General bounds on the observed Lk (Remark 6.24).
31. Multi-sentence amplification 1/sqrt(prod p_t) (Remark 6.29).
32. Accuracy claims for the algorithms verified only numerically in the engineering documents, including fp16-stored Jacobi and fp16 fused accumulation (Remark 6.37).

**B.7 Capacity**

33. Conjecture 7.13: generic ranks and surjectivity of Sim14/Sim15 at L_sat; generation of SO(d^2)/SU(d^2); reaching the eigenbasis of an arbitrary density matrix.
34. Numerical Jacobian ranks as theorems (Remark 7.12).
35. The heuristic q_n = 4..6 (Remark 7.20).
36. Attributions of data re-uploading (Theorem 7.18) and Eremenko-Yuditskii optimality (Remark 7.25).
37. The generalisation, retrieval and probe figures of addendum Sec. 15.3 (Remark 7.35).

# Appendix C. Standard results cited

Each entry gives the result, its source, and where the specialisation used here is proved.

1. **Lambek's switching lemma.** J. Lambek, "Type grammar revisited", LACL 1997, LNAI 1582 (1999). Citation details UNVERIFIED. The discrete-case proof is given in Lemma 1.4.
2. **Pregroup grammars are context-free.** W. Buszkowski, LACL 2001, LNAI 2099, 95-109. Citation UNVERIFIED. Not used in any proof (Theorem 1.13).
3. **Hopcroft-Karp matching.** SIAM J. Comput. 2 (1973) 225-231. Used for decidability in Proposition 1.14.
4. **Valiant: #P-completeness of the permanent.** Theor. Comput. Sci. 8 (1979) 189-201. Remark after Proposition 1.14.
5. **Free rigid (compact) categories.** Preller and Lambek, Math. Struct. Comp. Sci. 17 (2007); Joyal and Street. Universal property used in Proposition 2.17 (no later result needs it; see the remark after Proposition 2.17).
6. **Classification of commutative dagger-Frobenius algebras.** Coecke, Pavlovic and Vicary, Math. Struct. Comp. Sci. 2013. Only the proved direction (Proposition 2.28) is used.
7. **Spider theorem.** Lack 2004; Coecke, Paquette and Pavlovic. Not used in any proof; every tensor is checked directly.
8. **CPM construction.** P. Selinger, ENTCS 170 (2007). **Choi's theorem**: M.-D. Choi, Linear Algebra Appl. 10 (1975) 285-290. Used in Theorem 2.39 and Proposition 2.40.
9. **Gamma-Beta relation.** Johnson, Kotz and Balakrishnan, *Continuous Univariate Distributions* vol. 2, ch. 25. Specialisation, proved here: let X ~ Gamma(a,1) and Y ~ Gamma(b,1) be independent, and change variables to su = X + Y and v = X/(X+Y), with Jacobian su. The joint density becomes su^{a+b-1} e^{-su} v^{a-1}(1-v)^{b-1}/(Gamma(a)Gamma(b)), which factorises, so v ~ Beta(a,b). Also, for a standard complex Gaussian g, abs(g)^2 = X^2 + Y^2 with X, Y ~ N(0, 1/2), which is Exp(1). The density of a standard Gaussian vector is invariant under unitaries, so g/||g|| is Haar. Used in Theorem 3.32.
10. **Sard's theorem.** A. Sard, Bull. AMS 48 (1942); J. Milnor, *Topology from the Differentiable Viewpoint*, Sec. 2. Used in Theorem 7.6.
11. **Strong subadditivity; monotonicity of mutual information.** Lieb and Ruskai, J. Math. Phys. 14 (1973) 1938; Nielsen and Chuang, Thm 11.15. Used in Theorems 3.39(d) and 4.27.
12. **Fuchs-van de Graaf inequalities.** IEEE Trans. Inf. Theory 45 (1999) 1216. Used in Lemma 3.42(b); the pure-state and equal-spectrum qubit cases are proved there.
13. **Singular value decomposition and Schmidt decomposition.** Nielsen and Chuang, Thm 2.7. Used in Theorem 3.39(a).
14. **Eckart-Young-Mirsky.** Psychometrika 1 (1936) 211; Q. J. Math. 11 (1960) 50. **Weyl's singular-value inequality**: Horn and Johnson, *Topics in Matrix Analysis*, Thm 3.3.16. Used in Theorem 4.22.
15. **Floating-point dot-product and underflow models.** N. J. Higham, *Accuracy and Stability of Numerical Algorithms*, 2nd ed., SIAM 2002: Sec. 2.1 eq. (2.8), Sec. 3.1 Lemma 3.1 and eq. (3.5), Sec. 23.2.4 (3M). Used in Lemma 6.4 and Theorem 6.6.
16. **Projection onto a closed convex set is non-expansive.** Standard (for example Bauschke and Combettes, *Convex Analysis and Monotone Operator Theory*, Prop. 4.16). Used in Theorem 6.14.
17. **Daleckii-Krein formula.** Daleckii and Krein (1965); R. Bhatia, *Matrix Analysis*, Thm V.3.3. Used in Proposition 5.30(d).
18. **Coefficient formula for absolutely convergent exponential series (Bohr mean).** Standard. Used in Theorem 5.5.
19. **Markov brothers' inequality.** A. A. Markov (1889): max abs(Pl') <= 2 n^2 max abs(Pl)/(b - a) on [a, b]. Used in Theorem 7.23(d) and Remark 7.25.
20. **Schur-Weyl duality for the second tensor power.** The commutant of {U (x) U} is span{I, swap}. Used in Theorem 7.31.
21. **Barren plateaus.** McClean et al., Nat. Commun. 9, 4812 (2018): the specialisation is proved as Theorem 7.31. Cerezo et al., Nat. Commun. 12, 1791 (2021): cited only. Ragone et al. (2024): UNVERIFIED.
22. **Orbit-stabiliser theorem and fibre-dimension theorem for polynomial maps.** Standard (Shafarevich, *Basic Algebraic Geometry*, I.6.3). Used in Propositions 7.14(c) and 7.30.
23. **Reck et al.** Phys. Rev. Lett. 73 (1994) 58. **Clements et al.** Optica 3 (2016) 1460. Cited in Remark 3.23.
24. **Spall's SPSA.** IEEE Trans. Autom. Control 37 (1992) 332. Gain sequences in Definition 5.32; the variance is proved in Theorem 5.33.
25. **Quantum natural gradient.** Stokes, Izaac, Killoran and Carleo, Quantum 4, 269 (2020). Definition 5.36.
26. **Eremenko-Yuditskii.** J. Approx. Theory 146 (2007) 144. UNVERIFIED, Remark 7.25.
27. **Gibbs' inequality, Jensen's inequality, Bessel's inequality, the Catalan recurrence, Cauchy-Schwarz.** Standard; used throughout.
